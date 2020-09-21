#include "MaterialManager.h"
#include <fstream>

#include "../valve_sdk/csgostructs.hpp"
#include "../settings/settings.h"
#include "../hooks/hooks.h"
#include "../helpers/input.h"

MaterialManager::MaterialManager()
{
	std::ofstream("csgo\\materials\\sensum_regular.vmt") << R"#("VertexLitGeneric"
{
  "$basetexture" "vgui/white"
  "$ignorez"      "0"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
	std::ofstream("csgo\\materials\\sensum_reflective.vmt") << R"#("VertexLitGeneric"
{
  "$basetexture" "vgui/white"
  "$envmap"       "env_cubemap"
  "$model" "1"
  "$normalmapalphaenvmapmask"  "1"
  "$envmapcontrast" "1"
  "$flat" "1"
  "$nocull" "0"
  "$selfillum" "1"
  "$halflambert" "1"
  "$nofog" "0"
  "$ignorez" "0"
  "$znearer" "0"
  "$wireframe" "0"
}
)#";
	std::ofstream("csgo\\materials\\sensum_reflectiveignorez.vmt") << R"#("VertexLitGeneric"
{
  "$basetexture" "vgui/white"
  "$envmap"       "env_cubemap"
  "$model" "1"
  "$normalmapalphaenvmapmask"  "1"
  "$envmapcontrast" "1"
  "$flat" "1"
  "$nocull" "0"
  "$selfillum" "1"
  "$halflambert" "1"
  "$nofog" "0"
  "$ignorez" "1"
  "$znearer" "0"
  "$wireframe" "0"
}
)#";
	std::ofstream("csgo\\materials\\sensum_ignorez.vmt") << R"#("VertexLitGeneric"
{
  "$basetexture" "vgui/white"
  "$ignorez"      "1"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
	std::ofstream("csgo\\materials\\sensum_flat.vmt") << R"#("UnlitGeneric"
{
  "$basetexture" "vgui/white"
  "$ignorez"      "0"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
	std::ofstream("csgo\\materials\\sensum_flat_ignorez.vmt") << R"#("UnlitGeneric"
{
  "$basetexture" "vgui/white"
  "$ignorez"      "1"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";

	material_regular = g::mat_system->FindMaterial("sensum_regular", TEXTURE_GROUP_MODEL);
	material_flat = g::mat_system->FindMaterial("sensum_flat", TEXTURE_GROUP_MODEL);
	material_reflective = g::mat_system->FindMaterial("sensum_reflective", TEXTURE_GROUP_MODEL);
	material_dogtags_outline = g::mat_system->FindMaterial("models/inventory_items/dogtags/dogtags_outline", TEXTURE_GROUP_OTHER); //blinking chams - no?
	material_gloss = g::mat_system->FindMaterial("models/inventory_items/trophy_majors/gloss", TEXTURE_GROUP_OTHER); //interesting - hand/desync chams?
	material_fbi_glass = g::mat_system->FindMaterial("models/player/ct_fbi/ct_fbi_glass", TEXTURE_GROUP_OTHER); //no - maybe for hand/desync chams.
	material_cologne_prediction_glass = g::mat_system->FindMaterial("models/inventory_items/cologne_prediction/cologne_prediction_glass", TEXTURE_GROUP_OTHER); //interesting - maybe hand/desync chams.
	material_crystal_clear = g::mat_system->FindMaterial("models/inventory_items/trophy_majors/crystal_clear", TEXTURE_GROUP_OTHER); //looks quite good for chams, cant color them :/
	material_gold = g::mat_system->FindMaterial("models/inventory_items/trophy_majors/gold", TEXTURE_GROUP_OTHER); //looks good as well.
	material_glass_glass = g::mat_system->FindMaterial("models/gibs/glass/glass", TEXTURE_GROUP_OTHER); //no, maybe for hand/weapons chams.
	material_wildwire_gold = g::mat_system->FindMaterial("models/inventory_items/wildfire_gold/wildfire_gold_detail", TEXTURE_GROUP_OTHER); //interesting - for sure yes for enemy chams.
	material_crystal_blue = g::mat_system->FindMaterial("models/inventory_items/trophy_majors/crystal_blue", TEXTURE_GROUP_OTHER); //YES I WANT THIS!!!
	material_metal_gibs = g::mat_system->FindMaterial("models/gibs/metalgibs/metal_gibs", TEXTURE_GROUP_OTHER); //kinda good looking material - silverish color
	material_shards = g::mat_system->FindMaterial("models/inventory_items/op09/op09_shards", TEXTURE_GROUP_OTHER); //looks kinda good - purple color
	material_dev_glow = g::mat_system->FindMaterial("dev/glow_color", TEXTURE_GROUP_OTHER);
	material_regular_xqz = g::mat_system->FindMaterial("sensum_ignorez", TEXTURE_GROUP_MODEL);
	material_flat_xqz = g::mat_system->FindMaterial("sensum_flat_ignorez", TEXTURE_GROUP_MODEL);
	material_reflective_xqz = g::mat_system->FindMaterial("sensum_reflectiveignorez", TEXTURE_GROUP_MODEL);

	material_regular->IncrementReferenceCount();
	material_flat->IncrementReferenceCount();
	material_reflective->IncrementReferenceCount();
	material_dogtags_outline->IncrementReferenceCount();
	material_gloss->IncrementReferenceCount();
	material_fbi_glass->IncrementReferenceCount();
	material_cologne_prediction_glass->IncrementReferenceCount();
	material_crystal_clear->IncrementReferenceCount();
	material_gold->IncrementReferenceCount();
	material_glass_glass->IncrementReferenceCount();
	material_wildwire_gold->IncrementReferenceCount();
	material_crystal_blue->IncrementReferenceCount();
	material_metal_gibs->IncrementReferenceCount();
	material_shards->IncrementReferenceCount();
	material_dev_glow->IncrementReferenceCount();
	material_regular_xqz->IncrementReferenceCount();
	material_flat_xqz->IncrementReferenceCount();
	material_reflective_xqz->IncrementReferenceCount();
}

MaterialManager::~MaterialManager()
{
	std::remove("csgo\\materials\\sensum_regular.vmt");
	std::remove("csgo\\materials\\sensum_ignorez.vmt");
	std::remove("csgo\\materials\\sensum_flat_ignorez.vmt");
	std::remove("csgo\\materials\\sensum_flat.vmt");
	std::remove("csgo\\materials\\sensum_reflective.vmt");
	std::remove("csgo\\materials\\sensum_reflective.vmt");
	std::remove("csgo\\materials\\sensum_reflectiveignorez.vmt");
}

void MaterialManager::OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, bool metallic, bool crystal_blue, bool metal_gibs, bool shards, bool dev_glow)
{
	static IMaterial* material = nullptr;

	if (flat)
	{
		if (ignoreZ)
		{
			material = material_flat_xqz;
		}
		else
		{
			material = material_flat;
		}
	}
	else
	{
		if (ignoreZ && !metallic && !flat)
		{
			material = material_regular_xqz;
		}
		else if (ignoreZ && metallic && !flat)
		{
			material = material_reflective_xqz;
		}
		else if (metallic)
		{
			material = material_reflective;
		}
		else
		{
			material = material_regular;
		}
	}

	if (crystal_blue)
	{
		material = material_crystal_blue;
	}

	if (metal_gibs)
	{
		material = material_metal_gibs;
	}

	if (shards)
	{
		material = material_shards;
	}

	if (dev_glow)
	{
		material = material_dev_glow;
	}

	if (glass)
	{
		material = material_flat;
		material->AlphaModulate(0.45f);
		material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignoreZ);
	}

	material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, wireframe);

	g::mdl_render->ForcedMaterialOverride(material);
	material->IncrementReferenceCount();
}

void MaterialManager::OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, bool metallic, bool crystal_blue, bool metal_gibs, bool shards, bool dev_glow, const Color& rgba)
{
	static IMaterial* material = nullptr;

	if (flat)
	{
		if (ignoreZ)
		{
			material = material_flat_xqz;
		}
		else
		{
			material = material_flat;
		}
	}
	else
	{
		if (ignoreZ && !metallic && !flat)
		{
			material = material_regular_xqz;
		}
		else if (ignoreZ && metallic && !flat)
		{
			material = material_reflective_xqz;
		}
		else if (metallic)
		{
			material = material_reflective;
		}
		else
		{
			material = material_regular;
		}
	}

	if (crystal_blue)
	{
		material = material_crystal_blue;
	}

	if (metal_gibs)
	{
		material = material_metal_gibs;
	}

	if (shards)
	{
		material = material_shards;
	}

	if (dev_glow)
	{
		material = material_dev_glow;
	}

	if (glass)
	{
		material = material_flat;
		material->AlphaModulate(0.45f);
		material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignoreZ);
	}

	material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, wireframe);

	g::mdl_render->ForcedMaterialOverride(material);
	g::render_view->SetColorModulation(rgba.r() / 255.0f, rgba.g() / 255.0f, rgba.b() / 255.0f);
	material->IncrementReferenceCount();
}
