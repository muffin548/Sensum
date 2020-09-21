#include "../render.h"
#include "../../settings/globals.h"
#include "../../settings/settings.h"
#include "../../helpers/imdraw.h"
#include "../../helpers/console.h"
#include "../..//features/features.h"

extern void bind_button(const char* label, int& key);
extern bool hotkey(const char* label, int* k, const ImVec2& size_arg = ImVec2(0.f, 0.f));

namespace render
{
	namespace menu
	{
		void visuals_tab()
		{
			child("ESP", []()
				{
					columns(2);
					{
						checkbox("Enabled", &settings::esp::enabled);

						ImGui::NextColumn();

						ImGui::PushItemWidth(-1);
						hotkey("##binds.esp", &globals::binds::esp);
						ImGui::PopItemWidth();
					}
					columns(1);

					checkbox("Visible Only", &settings::esp::visible_only);

					checkbox("Name", &settings::esp::names);

					columns(2);
					{
						checkbox("Weapon", &settings::esp::weapons);

						ImGui::NextColumn();

						const char* weapon_modes[] = {
						"Text",
						"Icons"
						};

						ImGui::PushItemWidth(-1);
						{
							ImGui::Combo("Mode", &settings::esp::weapon_mode, weapon_modes, IM_ARRAYSIZE(weapon_modes));
						}
						ImGui::PopItemWidth();
					}
					ImGui::Columns(1);

					columns(2);
					{
						checkbox("Player Info Box", &settings::visuals::player_info_box);

						ImGui::NextColumn();
						ImGui::PushItemWidth(-1);
						{
							ImGui::SliderFloatLeftAligned("Alpha##infobox", &settings::visuals::player_info_box_alpha, 0.0f, 1.0f, "%0.1f");
						}
						ImGui::PopItemWidth();
					}
					ImGui::Columns(1);

					columns(2);
					{
						checkbox("Grief Box", &settings::visuals::grief_box);

						ImGui::NextColumn();
						ImGui::PushItemWidth(-1);
						{
							ImGui::SliderFloatLeftAligned("Alpha##griefbox", &settings::visuals::grief_box_alpha, 0.0f, 1.0f, "%0.1f");
						}
						ImGui::PopItemWidth();
					}
					ImGui::Columns(1);

					columns(2);
					{
						checkbox("Boxes", &settings::esp::boxes);

						ImGui::NextColumn();

						const char* box_types[] = {
							"Normal",
							"Corner"
						};

						ImGui::PushItemWidth(-1);
						{
							ImGui::Combo("##esp.box_type", &settings::esp::box_type, box_types, IM_ARRAYSIZE(box_types));
						}
						ImGui::PopItemWidth();
					}
					ImGui::Columns(1);

					const char* positions[] =
					{
						"Left",
						"Right",
						"Bottom"
					};

					const char* HealthPositions[] =
					{
						"Left",
						"Right",
						"Bottom",
						"Number"
					};

					columns(2);
					{
						checkbox("Health", &settings::esp::health);

						ImGui::NextColumn();

						ImGui::PushItemWidth(-1);
						ImGui::Combo("##health.position", &settings::esp::health_position, HealthPositions, IM_ARRAYSIZE(HealthPositions));
						ImGui::PopItemWidth();
					}
					columns(1);

					columns(2);
					{
						checkbox("Armor", &settings::esp::armour);

						ImGui::NextColumn();

						ImGui::PushItemWidth(-1);
						ImGui::Combo("##armor.position", &settings::esp::armour_position, positions, IM_ARRAYSIZE(positions));
						ImGui::PopItemWidth();
					}
					columns(1);

					checkbox("Bone ESP", &settings::esp::bone_esp);
					checkbox("Is Scoped", &settings::esp::is_scoped);
					checkbox("Is Flashed", &settings::esp::is_flashed);
					checkbox("Is Defusing", &settings::esp::is_defusing);
					checkbox("Is Desyncing", &settings::esp::is_desyncing);
					checkbox("Has Kit", &settings::esp::haskit);
					checkbox("Ammo ESP", &settings::esp::ammo);
					checkbox("Money ESP", &settings::esp::money);
					checkbox("Sound ESP", &settings::esp::soundesp);

					checkbox("Beams", &settings::esp::beams);
					checkbox("Sound Direction (?)", &settings::esp::sound);
				
					checkbox("Bomb Damage ESP", &settings::esp::bomb_esp);
					checkbox("Offscreen ESP", &settings::esp::offscreen);
				});

			ImGui::NextColumn();

			child("Chams", []()
				{
					static const char* ChamsTypes[] = {
					"Visible - Normal",
					"Visible - Flat",
					"Visible - Wireframe",
					"Visible - Glass",
					"Visible - Metallic",
					"Visible - Crystal Blue",
					"Visible - Metal Gibs",
					"Visible - Shards",
					"Visible - Glow",
					"XQZ - Normal",
					"XQZ - Flat",
					"XQZ - Metallic"
					};

					static const char* bttype[] = {
					"Off",
					"Last Tick",
					"All Ticks"
					};

					static const char* chamsMaterials[] = {
					"Normal",
					"Dogtags",
					"Flat",
					"Metallic",
					"Platinum",
					"Glass",
					"Crystal",
					"Gold",
					"Dark Chrome",
					"Plastic/Gloss",
					"Glow"
					};

					columns(2);
					{
						checkbox("Enemy", &settings::chams::enemynew);

						ImGui::NextColumn();

						ImGui::PushItemWidth(-1);
						ImGui::Combo("Enemy - Mode", &settings::chams::enemymodenew, ChamsTypes, IM_ARRAYSIZE(ChamsTypes));
						ImGui::PopItemWidth();
					}
					columns(1);

					columns(2);
					{
						checkbox("Team", &settings::chams::teamnew);

						ImGui::NextColumn();

						ImGui::PushItemWidth(-1);
						ImGui::Combo("Team - Mode", &settings::chams::teammodenew, ChamsTypes, IM_ARRAYSIZE(ChamsTypes));
						ImGui::PopItemWidth();
					}
					columns(1);

					columns(2);
					{
						checkbox("Local", &settings::chams::localnew);

						ImGui::NextColumn();

						ImGui::PushItemWidth(-1);
						ImGui::Combo("Local - Mode", &settings::chams::localmodenew, ChamsTypes, IM_ARRAYSIZE(ChamsTypes));
						ImGui::PopItemWidth();
					}
					columns(1);

					columns(2);
					{
						checkbox("Real Angle   ", &settings::chams::desync);

						ImGui::NextColumn();

						ImGui::PushItemWidth(-1);
						ImGui::Combo("Material", &settings::chams::desyncChamsMode, chamsMaterials, IM_ARRAYSIZE(chamsMaterials));
						ImGui::PopItemWidth();
					}
					columns(1);

					ImGui::SameLine();
					checkbox("Planted C4", &settings::chams::plantedc4_chams);
					checkbox("Weapons (?)       ", &settings::chams::wep_droppedchams);
					tooltip("Dropped Weapons Chams");
					ImGui::SameLine();
					checkbox("Nades", &settings::chams::nade_chams);
					checkbox("Health Chams", &settings::chams::health_chams);

					static const char* glow_modes[] = {
						"Exterior",
						"Interior",
						"Outline"
					};

					child("Glow", []()
						{

							columns(2);
							{
								checkbox("Enemy", &settings::glow::glowEnemyEnabled);

								ImGui::NextColumn();

								ImGui::PushItemWidth(-1);
								ImGui::Combo("Enemy - Mode", &settings::glow::style_enemy, glow_modes, IM_ARRAYSIZE(glow_modes));
								ImGui::PopItemWidth();
							}
							columns(1);

							columns(2);
							{
								checkbox("Team  ", &settings::glow::glowTeamEnabled);

								ImGui::NextColumn();

								ImGui::PushItemWidth(-1);
								ImGui::Combo("Team - Mode", &settings::glow::style_teammate, glow_modes, IM_ARRAYSIZE(glow_modes));
								ImGui::PopItemWidth();
							}
							columns(1);

							checkbox("Planted C4         ", &settings::glow::glowC4PlantedEnabled);
							ImGui::SameLine();
							checkbox("Nades", &settings::glow::glowNadesEnabled);
							checkbox("Weapons (?)", &settings::glow::glowDroppedWeaponsEnabled);
							tooltip("Dropped Weapons Glow");
						});
				});

			ImGui::NextColumn();

			child("Extra", []()
				{
					static const char* cross_types[] = {
						"Crosshair",
						"Circle"
					};

					static const char* hitmarkersounds[] = {
						"Cod",
						"Skeet",
						"Punch",
						"Metal",
						"Boom"
					};

					checkbox("Buy Log", &settings::esp::buylog);
					checkbox("Planted C4", &settings::visuals::planted_c4);
					checkbox("World Weapons", &settings::visuals::dropped_weapons);
					checkbox("World Grenades", &settings::visuals::world_grenades);
					checkbox("Sniper Crosshair", &settings::visuals::sniper_crosshair);
					checkbox("Snap Lines", &settings::esp::snaplines);
					checkbox("Armor Status (?)", &settings::esp::kevlarinfo);
					tooltip("Will display HK if enemy has kevlar + helmer or K if enemy has kevlar only.");
					checkbox("Grenade Prediction", &settings::visuals::grenade_prediction);
					checkbox("Damage Indicator", &settings::misc::damage_indicator);
					checkbox("Aimbot Fov", &settings::esp::drawFov);
					checkbox("Spread Crosshair", &settings::visuals::spread_cross);
					checkbox("Bullet Tracer", &settings::visuals::bullet_tracer);


					columns(2);
					{
						checkbox("Hitmarker", &settings::visuals::hitmarker);

						ImGui::NextColumn();

						ImGui::PushItemWidth(-1);
						ImGui::Combo("Hitmarker Sound", &settings::visuals::hitsound, hitmarkersounds, IM_ARRAYSIZE(hitmarkersounds));
						ImGui::PopItemWidth();
					}
					columns(1);

					columns(2);
					{
						checkbox("RCS Crosshair", &settings::visuals::rcs_cross);

						ImGui::NextColumn();

						ImGui::PushItemWidth(-1);
						ImGui::Combo("RCS Crosshair Type", &settings::visuals::rcs_cross_mode, cross_types, IM_ARRAYSIZE(cross_types));
						ImGui::PopItemWidth();
					}
					columns(1);

					if (settings::visuals::rcs_cross_mode == 1)
						ImGui::SliderFloatLeftAligned("Radius", &settings::visuals::radius, 8.f, 18.f, "%.1f");

					const auto old_night_state = settings::visuals::night_mode;
					const auto old_style_state = settings::visuals::newstyle;
					checkbox("Night Mode", &settings::visuals::night_mode);

					if (settings::visuals::night_mode)
					{
						ImGui::SliderFloatLeftAligned("Night Mode Intensity:", &settings::esp::mfts, 0.0f, 1.0f, "%.1f %");

						if (ImGui::Button("Apply", ImVec2(ImGui::GetContentRegionAvailWidth(), 0.f)))
						{
							color_modulation::set_material_tone();
						}
					}

					checkbox("Dark Menu", &settings::visuals::newstyle);
					if (old_style_state != settings::visuals::newstyle)
						imdraw::apply_style(settings::visuals::newstyle);
				});
		}
	}
}