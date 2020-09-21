#include "Chams.h"
#include "../settings/settings.h"
#include "../features/features.h"
#include "../hooks/hooks.h"
#include <fstream>
#include "esp.hpp"

bool IsXQZ()
{
	if (settings::chams::enemymodenew == 0)
		return false;

	if (settings::chams::enemymodenew == 1)
		return false;

	if (settings::chams::enemymodenew == 2)
		return false;

	if (settings::chams::enemymodenew == 3)
		return false;

	if (settings::chams::enemymodenew == 4)
		return false;

	if (settings::chams::enemymodenew == 5)
		return false;

	if (settings::chams::enemymodenew == 6)
		return false;

	if (settings::chams::enemymodenew == 7)
		return false;

	if (settings::chams::enemymodenew == 8)
		return false;

	if (settings::chams::enemymodenew == 9)
		return true;

	if (settings::chams::enemymodenew == 10)
		return true;

	if (settings::chams::enemymodenew == 11)
		return true;

	return false;
}

void Chams::OnSceneEnd()
{
	if (!g::engine_client->IsInGame() || !g::engine_client->IsConnected() || !g::local_player)
		return;

	ChamsModes LocalChamsMode = (ChamsModes)settings::chams::localmodenew;
	ChamsModes TeamChamsMode = (ChamsModes)settings::chams::teammodenew;
	ChamsModes EnemyChamsMode = (ChamsModes)settings::chams::enemymodenew;

	bool LocalChams = settings::chams::localnew;
	bool TeamChams = settings::chams::teamnew;
	bool EnemyChams = settings::chams::enemynew;

	Color LocalColor = settings::chams::LocalColor_vis;
	Color TeamColor = settings::chams::TeamColor_vis;
	Color EnemyColor = settings::chams::EnemyColor_vis;

	Color LocalColorXqz = settings::chams::LocalColor_XQZ;
	Color TeamColorXqz = settings::chams::TeamColor_XQZ;
	Color EnemyColorXqz = settings::chams::EnemyColor_XQZ;

	for (int i = 1; i < g::engine_client->GetMaxClients(); i++)
	{
		auto entity = static_cast<c_base_player*>(g::entity_list->GetClientEntity(i));
		if (!entity || !entity->IsPlayer() || entity->IsDormant() || !entity->IsAlive())
			continue;

		if (utils::is_line_goes_through_smoke(g::local_player->GetEyePos(), entity->get_hitbox_position(entity, HITBOX_HEAD))) //GetRenderOrigin()
			continue;

		bool IsLocal = entity == g::local_player;
		bool IsTeam = !entity->IsEnemy();

		bool normal = false;
		bool flat = false;
		bool wireframe = false;
		bool glass = false;
		bool metallic = false;
		bool xqz = false;
		bool metallic_xqz = false;
		bool flat_xqz = false;
		bool glow = false;
		bool glow_xqz = false;
		bool crystal_blue = false;
		bool metal_gibs = false;
		bool shards = false;
		bool dev_glow = false;

		ChamsModes mode = IsLocal ? LocalChamsMode : (IsTeam ? TeamChamsMode : EnemyChamsMode);

		if (IsLocal && !LocalChams)
		{
			continue;
		}
		if ((IsTeam && !IsLocal) && !TeamChams)
		{
			continue;
		}
		if (!IsTeam && !EnemyChams)
		{
			continue;
		}


		Color clr = IsLocal ? LocalColor : (IsTeam ? TeamColor : (!IsXQZ() ? EnemyColorXqz : EnemyColor));
		Color clr2 = IsLocal ? LocalColorXqz : (IsTeam ? TeamColorXqz : (IsXQZ() ? EnemyColorXqz : EnemyColor));

		switch (mode)
		{
		case ChamsModes::regular:
			normal = true;
			break;
		case ChamsModes::flat:
			flat = true;
			break;
		case ChamsModes::wireframe:
			wireframe = true;
			break;
		case ChamsModes::glass:
			glass = true;
			break;
		case ChamsModes::reflective:
			metallic = true;
			break;
		case ChamsModes::crystal_blue:
			crystal_blue = true;
			break;
		case ChamsModes::metal_gibs:
			metal_gibs = true;
			break;
		case ChamsModes::shards:
			shards = true;
			break;
		case ChamsModes::dev_glow:
			dev_glow = true;
			break;
		case ChamsModes::regular_xqz:
			normal = true;
			xqz = true;
			break;
		case ChamsModes::flat_xqz:
			flat = true;
			flat_xqz = true;
			break;
		case ChamsModes::reflective_xqz:
			metallic = true;
			metallic_xqz = true;
			break;
		}

		int health = entity->m_iHealth();

		if (settings::chams::health_chams)
		{
			if (health == 100 || health >= 81)
				clr2 = Color(5, 255, 0, 255);
			else if (health == 80 || health >= 61)
				clr2 = Color::Yellow;
			else if (health == 60 || health >= 21)
				clr2 = Color(255, 127, 0, 255); //Orange
			else if (health <= 20)
				clr2 = Color::Red;
		}

		MaterialManager::Get().OverrideMaterial(xqz || metallic_xqz || flat_xqz, flat, wireframe, glass, metallic, crystal_blue, metal_gibs, shards, dev_glow, clr2);
		entity->GetClientRenderable()->DrawModel(0x1, 255);
		if (xqz || metallic_xqz || flat_xqz || glow_xqz)
		{
			MaterialManager::Get().OverrideMaterial(false, flat, wireframe, glass, metallic, crystal_blue, metal_gibs, shards, dev_glow, clr);
			entity->GetClientRenderable()->DrawModel(0x1, 255);
		}
		g::mdl_render->ForcedMaterialOverride(nullptr);
	}
	g::mdl_render->ForcedMaterialOverride(nullptr);
}