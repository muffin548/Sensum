#include "../../render/render.h"
#include "../../settings/globals.h"
#include "../../settings/settings.h"
#include "../../helpers/imdraw.h"
#include "../../helpers/console.h"

extern void bind_button(const char* label, int& key);
extern bool hotkey(const char* label, int* k, const ImVec2& size_arg = ImVec2(0.f, 0.f));

namespace render
{
	namespace menu
	{
		void colors_tab()
		{
			separator("Chams");
			ColorEdit4("Enemy Visible", &settings::chams::EnemyColor_vis, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			ImGui::SameLine();
			ColorEdit4("Enemy XQZ", &settings::chams::EnemyColor_XQZ, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);

			ColorEdit4("Team Visible  ", &settings::chams::TeamColor_vis, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			ImGui::SameLine();
			ColorEdit4("Team XQZ", &settings::chams::TeamColor_XQZ, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);

			ColorEdit4("Local Visible   ", &settings::chams::LocalColor_vis, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			ImGui::SameLine();
			ColorEdit4("Local XQZ", &settings::chams::LocalColor_XQZ, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			ColorEdit4("Planted C4     ##chams", &settings::chams::colorPlantedC4Chams, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			ImGui::SameLine();
			ColorEdit4("Weapons (?)##chams", &settings::chams::ColorWeaponDroppedChams, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			tooltip("Dropped Weapons");
			ColorEdit4("Grenades      ##chams", &settings::chams::colorNadeChams, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			ImGui::SameLine();
			ColorEdit4("Real Angle##chams", &settings::chams::desync_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);

			separator("ESP");
			ColorEdit4("ESP Visible ", &settings::esp::visibleColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			ImGui::SameLine();
			ColorEdit4("ESP Invisible", &settings::esp::occludedColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);

			ColorEdit4("Bone ESP", &settings::esp::bone_esp_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);

			ColorEdit4("RCS Cross  ", &settings::visuals::recoilcolor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			ImGui::SameLine();
			ColorEdit4("Spread Cross", &settings::visuals::spread_cross_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			ColorEdit4("Aimbot Fov", &settings::visuals::drawfov_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			ImGui::SameLine();
			ColorEdit4("Bullet Tracer (?)", &settings::visuals::colorBulletTracer, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			tooltip("Disabled saving of Bullet Tracer - Causes bug that will make beams not render :(. Default color is purple.");
			ColorEdit4("AA Indicator", &settings::esp::aa_indicator_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			ColorEdit4("Sound ESP", &settings::esp::colorSoundEsp, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);

			separator("Glow");

			ColorEdit4("Enemy", &settings::glow::glowEnemyColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			ImGui::SameLine();
			ColorEdit4("Planted C4##glow", &settings::glow::glowC4PlantedColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			ImGui::SameLine();
			ColorEdit4("Weapons##glow", &settings::glow::glowDroppedWeaponsColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			tooltip("Dropped Weapons");
			ColorEdit4("Team  ", &settings::glow::glowTeamColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
			ImGui::SameLine();
			ColorEdit4("Grenades##glow", &settings::glow::glowNadesColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar);
		}
	}
}