#include "features.h"
#include "../settings/globals.h"
#include "../settings/options.hpp"
#include "../helpers/console.h"

namespace color_modulation
{
	auto debug_crosshair = -1;
	auto view_model_fov = -1.f;
	auto debug_model_fov = -1;

	auto arms_state = false;
	auto post_processing = false;
	auto night_mode_state = false;
	auto night_mode_first = false;
	auto arms_wireframe_state = true;

	auto flViewmodel_offset_x = -1.f;
	auto flViewmodel_offset_y = -1.f;
	auto flViewmodel_offset_z = -1.f;

	ImVec4 last_sky;
	ImVec4 arms_color;

	ConVar* viewmodel_fov = nullptr;
	ConVar* debug_fov = nullptr;
	ConVar* r_3dsky = nullptr;
	ConVar* r_DrawSpecificStaticProp = nullptr;

	ConVar* viewmodel_offset_x = nullptr;
	ConVar* viewmodel_offset_y = nullptr;
	ConVar* viewmodel_offset_z = nullptr;

	const uint32_t model_textures = FNV("Model textures");
	const uint32_t skybox_textures = FNV("SkyBox textures");
	const uint32_t world_textures = FNV("World textures");
	const uint32_t static_prop_textures = FNV("StaticProp textures");
	const uint32_t particle_textures = FNV("Particle textures");

	uint8_t* sky_fn_offset = nullptr;

	void event()
	{
		if (!night_mode_first)
			if (settings::visuals::night_mode) night_mode_first = true;
			else return;

		if (!sky_fn_offset)
			sky_fn_offset = utils::pattern_scan(SET_SKY);

		assert(sky_fn_offset);

		if (!r_3dsky)
		{
			r_3dsky = g::cvar->find("r_3dsky");
			r_3dsky->m_fnChangeCallbacks.m_Size = 0;
		}

		if (!r_DrawSpecificStaticProp)
		{
			r_DrawSpecificStaticProp = g::cvar->find("r_drawspecificstaticprop");
			r_DrawSpecificStaticProp->m_fnChangeCallbacks.m_Size = 0;
		}

		reinterpret_cast<void(__fastcall*)(const char*)>(sky_fn_offset)(xorstr_("vertigo"));

		static auto sv_skyname = g::cvar->find(xorstr_("sv_skyname"));

		r_3dsky->SetValue(0);
		r_DrawSpecificStaticProp->SetValue(settings::visuals::night_mode ? 0 : 1);

		last_sky = ImVec4();
		debug_crosshair = -1;
		night_mode_state = !night_mode_state;
	}

	bool is_vars_changed()
	{
		if (last_sky != settings::visuals::sky)
			return true;

		if (night_mode_state != settings::visuals::night_mode)
			return true;

		if (arms_state != settings::chams::arms::enabled)
			return true;

		if (arms_wireframe_state != settings::chams::arms::wireframe)
			return true;

		if (arms_color != settings::chams::arms::color)
			return true;

		if (post_processing != globals::post_processing)
			return true;

		if (view_model_fov != settings::misc::viewmodel_fov)
			return true;

		if (debug_model_fov != settings::misc::debug_fov)
			return true;

		if (flViewmodel_offset_x != settings::misc::viewmodel_offset_x)
			return true;

		if (flViewmodel_offset_y != settings::misc::viewmodel_offset_y)
			return true;

		if (flViewmodel_offset_z != settings::misc::viewmodel_offset_z)
			return true;

		return false;
	}

	void sky_changer()
	{
		static auto sv_skyname = g::cvar->find(xorstr_("sv_skyname"));

		auto sv_skyname_backup = g::cvar->find("sv_skyname")->GetString();

		if (!settings::visuals::skychanger)
			sv_skyname->SetValue(sv_skyname_backup);

		switch (settings::visuals::skychanger_mode)
		{
		case 0:
			return;
			break;
		case 1: //Baggage
			sv_skyname->SetValue("cs_baggage_skybox");
			break;
		case 2: //Tibet
			sv_skyname->SetValue("cs_tibet");
			break;
		case 3: //Embassy
			sv_skyname->SetValue("embassy");
			break;
		case 4: //Italy
			sv_skyname->SetValue("italy");
			break;
		case 5: //Daylight 1
			sv_skyname->SetValue("sky_cs15_daylight01_hdr");
			break;
		case 6: //Daylight 2
			sv_skyname->SetValue("sky_cs15_daylight02_hdr");
			break;
		case 7: //Daylight 3
			sv_skyname->SetValue("sky_cs15_daylight03_hdr");
			break;
		case 8: //Daylight 4
			sv_skyname->SetValue("sky_cs15_daylight04_hdr");
			break;
		case 9: //Cloudy
			sv_skyname->SetValue("sky_csgo_cloudy01");
			break;
		case 10: //Night 1
			sv_skyname->SetValue("sky_csgo_night02");
			break;
		case 11: //Night 2
			sv_skyname->SetValue("sky_csgo_night02b");
			break;
		case 12: //Night Flat
			sv_skyname->SetValue("sky_csgo_night_flat");
			break;
		case 13: //Day HD
			sv_skyname->SetValue("sky_day02_05_hdr");
			break;
		case 14: //Day
			sv_skyname->SetValue("sky_day02_05");
			break;
		case 15: //Rural
			sv_skyname->SetValue("sky_l4d_rural02_ldr");
			break;
		case 16: //Vertigo HD
			sv_skyname->SetValue("vertigo_hdr");
			break;
		case 17: //Vertigo Blue HD
			sv_skyname->SetValue("vertigoblue_hdr");
			break;
		case 18: //Vertigo
			sv_skyname->SetValue("vertigo");
			break;
		case 19: //Vietnam
			sv_skyname->SetValue("vietnam");
			break;
		case 20: //Dusty Sky
			sv_skyname->SetValue("sky_dust");
			break;
		case 21: //Jungle
			sv_skyname->SetValue("jungle");
			break;
		case 22: //Nuke
			sv_skyname->SetValue("nukeblank");
			break;
		case 23: //Office
			sv_skyname->SetValue("office");
			break;
		default:
			break;
		}
	}

	void set_convars()
	{
		static auto r_modelAmbientMin = g::cvar->find(xorstr_("r_modelAmbientMin"));
		static auto mat_postprocess_enable = g::cvar->find(xorstr_("mat_postprocess_enable"));
		static auto mat_force_tonemap_scale = g::cvar->find(xorstr_("mat_force_tonemap_scale"));

		if (!viewmodel_fov)
		{
			viewmodel_fov = g::cvar->find(xorstr_("viewmodel_fov"));
			viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
		}

		if (!debug_fov)
		{
			debug_fov = g::cvar->find(xorstr_("fov_cs_debug"));
			debug_fov->m_fnChangeCallbacks.m_Size = 0;
		}

		if (!viewmodel_offset_x)
		{
			viewmodel_offset_x = g::cvar->find(xorstr_("viewmodel_offset_x"));
			viewmodel_offset_x->m_fnChangeCallbacks.m_Size = 0;
		}

		if (!viewmodel_offset_y)
		{
			viewmodel_offset_y = g::cvar->find(xorstr_("viewmodel_offset_y"));
			viewmodel_offset_y->m_fnChangeCallbacks.m_Size = 0;
		}

		if (!viewmodel_offset_z)
		{
			viewmodel_offset_z = g::cvar->find(xorstr_("viewmodel_offset_z"));
			viewmodel_offset_z->m_fnChangeCallbacks.m_Size = 0;
		}

		r_modelAmbientMin->SetValue(settings::visuals::night_mode ? 1.f : 0.f);
		mat_force_tonemap_scale->SetValue(settings::visuals::night_mode ? 0.2f : 1.f);

		viewmodel_fov->SetValue(settings::misc::viewmodel_fov);
		mat_postprocess_enable->SetValue(post_processing ? 1 : 0);

		auto engine_focus = g::cvar->find("engine_no_focus_sleep"); //This command makes csgo not go into low fps mode while alt tabbed.
		engine_focus->SetValue(0); //This is an anti untrusted measurement while being in windowed & alt tabbed.
		//(Because sometimes clamping fails because of low fps while alt tabbed.)

		static auto backup_viewmodel_x = g::cvar->find("viewmodel_offset_x")->GetFloat();
		static auto backup_viewmodel_y = g::cvar->find("viewmodel_offset_y")->GetFloat();
		static auto backup_viewmodel_z = g::cvar->find("viewmodel_offset_z")->GetFloat();

		if (settings::misc::override_viewmodel)
		{
			viewmodel_offset_x->SetValue(settings::misc::viewmodel_offset_x);
			viewmodel_offset_y->SetValue(settings::misc::viewmodel_offset_y);
			viewmodel_offset_z->SetValue(settings::misc::viewmodel_offset_z);
		}
		else
		{
			viewmodel_offset_x->SetValue(backup_viewmodel_x);
			viewmodel_offset_y->SetValue(backup_viewmodel_y);
			viewmodel_offset_z->SetValue(backup_viewmodel_z);
		}
	}

	void set_material_tone()
	{
		if (!settings::visuals::night_mode)
			return;

		static auto mat_force_tonemap_scale = g::cvar->find(xorstr_("mat_force_tonemap_scale"));

		mat_force_tonemap_scale->SetValue(settings::esp::mfts);
	}

	void sniper_crosshair()
	{
		if (!g::local_player)
			return;

		bool is_scoped = g::local_player->m_bIsScoped();
		if (!g::local_player->IsAlive() && g::local_player->m_hObserverTarget())
		{
			auto observer = (c_base_player*)c_base_player::GetEntityFromHandle(g::local_player->m_hObserverTarget());
			if (observer && observer->IsPlayer())
				is_scoped = observer->m_bIsScoped();
		}

		static auto weapon_debug_spread_show = g::cvar->find(xorstr_("weapon_debug_spread_show"));

		if (settings::visuals::sniper_crosshair)
		{
			if (debug_crosshair != 0 && is_scoped)
			{
				debug_crosshair = 0;
				weapon_debug_spread_show->SetValue(0);
			}

			if (debug_crosshair != 3 && !is_scoped)
			{
				debug_crosshair = 3;
				weapon_debug_spread_show->SetValue(3);
			}
		}
		else if (debug_crosshair != 0)
		{
			debug_crosshair = 0;
			weapon_debug_spread_show->SetValue(0);
		}
	}

	void handle()
	{
		sniper_crosshair();

		if (!is_vars_changed())
			return;

		//last_sky = settings::visuals::sky;

		post_processing = globals::post_processing;
		view_model_fov = settings::misc::viewmodel_fov;
		debug_model_fov = settings::misc::debug_fov;
		night_mode_state = settings::visuals::night_mode;

		arms_color = settings::chams::arms::color;
		arms_state = settings::chams::arms::enabled;
		arms_wireframe_state = settings::chams::arms::wireframe;

		set_convars();
		set_material_tone();
		sky_changer();

		static auto sv_skyname_backup = g::cvar->find("sv_skyname")->GetString();
		static auto sv_skyname = g::cvar->find(xorstr_("sv_skyname"));

		for (auto i = g::mat_system->FirstMaterial(); i != g::mat_system->InvalidMaterial(); i = g::mat_system->NextMaterial(i))
		{
			auto* material = g::mat_system->GetMaterial(i);
			if (!material)
				continue;

			const auto name = material->GetName();
			const auto group = material->GetTextureGroupName();

			const auto _name = fnv::hash_runtime(name);
			const auto _group = fnv::hash_runtime(group);

			if (_group == model_textures)
			{
				if (std::string(name).substr(0, 28) != std::string(xorstr_("models/weapons/v_models/arms")))
					continue;

				if (std::string(name).substr(0, 34) == std::string(xorstr_("models/weapons/v_models/arms/glove")))
					continue;

				const auto color = settings::chams::arms::color;
				if (settings::chams::arms::enabled)
					material->ColorModulate(color.x, color.y, color.z);
				else
					material->ColorModulate(1.f, 1.f, 1.f);

				//material->AlphaModulate(0.4f);

				material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, settings::chams::arms::enabled && settings::chams::arms::wireframe);
			}

			if (!night_mode_first)
				if (settings::visuals::night_mode)
					night_mode_first = true; //Thanks, Credits to Klavaro - MartiNJ409
				else continue;

			sv_skyname->SetValue(settings::visuals::night_mode ? "sky_csgo_night02" : sv_skyname_backup);
		}
	}
}