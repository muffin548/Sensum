#include "../features/features.h"
#include "../settings/globals.h"
#include "../render/render.h"
#include "../helpers/entities.h"
#include "../features/soundesp.h"

#include <mutex>

entities::sound_t sounds[MAX_PLAYERS];

c_sound_esp _sound_esp;

class sound_info {
public:
	sound_info(Vector positions, float times) {
		this->position = positions;
		this->time = times;
	}

	Vector position;
	float time;
};

std::vector<sound_info> slogs;

namespace offscreen_entities
{

	int alpha = 0;
	bool plus_or_minus = false;

	QAngle angles;
	ImVec2 display_size;
	CUtlVector<SndInfo_t> sndList;

	ImU32 sound_color;
	ImU32 origin_color;

	void dot(const Vector& origin, const Vector& pos, const ImU32& color)
	{
		QAngle aim;
		math::vector2angles(pos - origin, aim);

		Vector popvec;
		math::angle2vectors(QAngle(0, 270, 0) - aim + QAngle(0, angles.yaw, 0), popvec);

		const auto screen_pos = Vector(display_size.x / 2, display_size.y / 2, 0) + (popvec * 190.f);

		globals::draw_list->AddCircleFilled(ImVec2(screen_pos.x, screen_pos.y), 12.f, color);
	}

	auto rotate_points(Vector* points, float rotation)->void
	{
		const auto points_center = (points[0] + points[1] + points[2]) / 3;
		for (int k = 0; k < 3; k++)
		{
			auto& point = points[k];
			point -= points_center;

			const auto temp_x = point.x;
			const auto temp_y = point.y;

			const auto theta = DEG2RAD(rotation);
			const auto c = cos(theta);
			const auto s = sin(theta);

			point.x = temp_x * c - temp_y * s;
			point.y = temp_x * s + temp_y * c;

			point += points_center;
		}
	};

	auto render_dot(ImDrawList* draw_list, const Vector& origin, const QAngle& angles, const ImU32& color) -> void
	{
		const auto display_size = ImGui::GetIO().DisplaySize;
		const auto screen_center = Vector(display_size.x * .5f, display_size.y * .5f);

		QAngle aim_angles;
		math::vector2angles(origin - g::local_player->m_vecOrigin(), aim_angles);

		const auto angle_yaw_rad = DEG2RAD(angles.yaw - aim_angles.yaw - 90);

		const auto size = 10;
		const auto radius = 45;

		const auto new_point_x = screen_center.x + ((((display_size.x - (size * 3)) * .5f) * (radius / 100.0f)) * cos(angle_yaw_rad)) + (int)(6.0f * (((float)size - 4.f) / 16.0f));
		const auto new_point_y = screen_center.y + ((((display_size.y - (size * 3)) * .5f) * (radius / 100.0f)) * sin(angle_yaw_rad));

		Vector points[3] =
		{
			Vector(new_point_x - size, new_point_y - size),
			Vector(new_point_x + size, new_point_y),
			Vector(new_point_x - size, new_point_y + size)
		};

		rotate_points(points, angles.yaw - aim_angles.yaw - 90.f);

		draw_list->AddTriangleFilled({ points[0].x + 1.f, points[0].y + 1.f }, { points[1].x + 1.f, points[1].y + 1.f }, { points[2].x + 1.f, points[2].y + 1.f }, IM_COL32_BLACK);
		draw_list->AddTriangleFilled({ points[0].x, points[0].y }, { points[1].x, points[1].y }, { points[2].x, points[2].y }, color);
	}

	auto render_sounds(ImDrawList* draw_list, const QAngle& angles)->void
	{
		if (!settings::esp::sound)
			return;

		for (const auto& sound : sounds)
		{
			if (sound.index != 0)
			{
				render_dot(draw_list, sound.origin, angles, IM_COL32_WHITE);

				if (settings::esp::soundesp)
				{
					static int timer;

					timer += 1;

					if (timer > 2)
						timer = 0;

					if (timer < 1)
						slogs.push_back(sound_info(sound.origin, g::global_vars->curtime));

					float current_time = g::local_player->m_nTickBase() * g::global_vars->interval_per_tick;

					for (unsigned int i = 0; i < slogs.size(); i++) {

						if (slogs[i].time < current_time)
						{
							slogs.erase(slogs.begin() + i);
							continue;
						}

						_sound_esp.draw_circle(settings::esp::colorSoundEsp, slogs[i].position);
					}
				}
			}
		}

		//char buf[256];
		//sprintf_s(buf, "globals::sound is %i", (int)globals::sound);
		//g::hud_chat->ChatPrintf(0, 0, buf);
	}

	void sound()
	{
		sndList.RemoveAll();
		CallVFunction<void(__thiscall*)(void*, CUtlVector<SndInfo_t>&)>(g::engine_sound, 19)(g::engine_sound, sndList); //index was 19

		auto eye_pos = g::local_player->GetEyePos();
		for (int i = 0; i < sndList.Count(); i++)
		{
			if (!sndList[i].m_pOrigin || !sndList[i].m_nSoundSource || !sndList[i].m_bUpdatePositions || sndList[i].m_nChannel != 4)
				continue;

			if (eye_pos.DistTo(*sndList[i].m_pOrigin) > 800)
				continue;

			auto* player = c_base_player::GetPlayerByIndex(sndList[i].m_nSoundSource);
			if (!player || !player->IsPlayer() || player->is_dormant() || !player->IsAlive() || player == g::local_player)
				continue;

			if (!settings::misc::deathmatch && player->m_iTeamNum() == g::local_player->m_iTeamNum())
				continue;

			dot(g::local_player->m_vecOrigin(), *sndList[i].m_pOrigin, sound_color);
		}
	}

	void render(ImDrawList* _draw_list)
	{
		if (!settings::esp::enabled || !g::local_player)
			return;

		if (!settings::esp::offscreen && !settings::esp::sound)
			return;

		g::engine_client->GetViewAngles(angles);

		if (settings::esp::sound && entities::m_mutex_sounds.try_lock())
		{
			memcpy(sounds, entities::m_sounds, sizeof(sounds));

			entities::m_mutex_sounds.unlock();
		}

		ImGui::PushFont(render::fonts::visuals);
		{
			render_sounds(_draw_list, angles);
		}
		ImGui::PopFont();
	}
}