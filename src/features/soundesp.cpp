#include "soundesp.h"
#include "../helpers/math.h"
#include "esp.hpp"
#include "../settings/globals.h"

c_sound_esp sound_esp;

std::vector<c_sound_info> sound_logs;

void c_sound_esp::draw_circle(Color colors, Vector position) noexcept {
	BeamInfo_t beam_info;
	beam_info.m_nType = TE_BEAMRINGPOINT;
	beam_info.m_pszModelName = "sprites/physbeam.vmt";
	beam_info.m_nModelIndex = g::mdl_info->GetModelIndex("sprites/physbeam.vmt");
	beam_info.m_nHaloIndex = -1;
	beam_info.m_flHaloScale = 5;
	beam_info.m_flLife = 1.f;
	beam_info.m_flWidth = 15.f;
	beam_info.m_flFadeLength = 1.0f;
	beam_info.m_flAmplitude = 0.f;
	beam_info.m_flRed = colors.r();
	beam_info.m_flGreen = colors.g();
	beam_info.m_flBlue = colors.b();
	beam_info.m_flBrightness = colors.a();
	beam_info.m_flSpeed = 0.f;
	beam_info.m_nStartFrame = 0.f;
	beam_info.m_flFrameRate = 60.f;
	beam_info.m_nSegments = -1;
	beam_info.m_nFlags = FBEAM_SHADEIN; //FBEAM_FADEOUT
	beam_info.m_vecCenter = position + Vector(0, 0, 5);
	beam_info.m_flStartRadius = 20.f;
	beam_info.m_flEndRadius = 640.f;

	Beam_t* beam = g::view_render_beams->CreateBeamRingPoint(beam_info);
	if (beam)
		g::view_render_beams->DrawBeam(beam);
}

void c_sound_esp::event_player_footstep(IGameEvent* event) noexcept {
	if (!settings::esp::soundesp)
		return;

	if (!g::engine_client->IsConnected() && !g::engine_client->IsInGame())
		return;

	if (!event)
		return;

	auto local_player = reinterpret_cast<c_base_player*>(g::entity_list->GetClientEntity(g::engine_client->GetLocalPlayer()));

	if (!local_player)
		return;

	auto walker = reinterpret_cast<c_base_player*>(g::entity_list->GetClientEntity(g::engine_client->GetPlayerForUserID(event->GetInt("userid"))));

	if (!walker)
		return;

	if (walker->IsDormant())
		return;

	static int timer;
	timer += 1;

	if (timer > 1)
		timer = 0;

	if (walker->m_iTeamNum() != g::local_player->m_iTeamNum()) {
		if (walker && timer < 1) {
			sound_logs.push_back(c_sound_info(walker->GetAbsOrigin(), g::global_vars->curtime, event->GetInt("userid")));
		}
	}
}

void c_sound_esp::event_player_hurt(IGameEvent* event) noexcept {
	if (!settings::esp::soundesp)
		return;

	if (!g::engine_client->IsConnected() && !g::engine_client->IsInGame())
		return;

	if (!event)
		return;

	auto local_player = reinterpret_cast<c_base_player*>(g::entity_list->GetClientEntity(g::engine_client->GetLocalPlayer()));

	if (!local_player)
		return;

	auto attacker = g::entity_list->GetClientEntity(g::engine_client->GetPlayerForUserID(event->GetInt("attacker")));

	if (!attacker)
		return;

	auto victim = reinterpret_cast<c_base_player*>(g::entity_list->GetClientEntity(g::engine_client->GetPlayerForUserID(event->GetInt("userid"))));

	if (!victim)
		return;

	static int timer;

	timer += 1;

	if (timer > 2)
		timer = 0;

	if (attacker == local_player) {
		if (timer < 1) {
			sound_logs.push_back(c_sound_info(victim->GetAbsOrigin(), g::global_vars->curtime, event->GetInt("userid"))); //victim->GetAbsOrigin()
		}
	}
}

void c_sound_esp::draw() noexcept {
	if (!settings::esp::soundesp)
		return;

	if (!g::engine_client->IsConnected() && !g::engine_client->IsInGame())
		return;

	for (unsigned int i = 0; i < sound_logs.size(); i++) {
		draw_circle(Color(settings::esp::colorSoundEsp.r(), settings::esp::colorSoundEsp.g(), settings::esp::colorSoundEsp.b(), settings::esp::colorSoundEsp.a()), sound_logs[i].position);
		sound_logs.erase(sound_logs.begin() + i);
	}
}