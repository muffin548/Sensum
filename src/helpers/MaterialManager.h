#pragma once

#include "singleton.hpp"

class IMatRenderContext;
struct DrawModelState_t;
struct ModelRenderInfo_t;
class matrix3x4_t;
class IMaterial;
class Color;

class MaterialManager : public Singleton<MaterialManager>
{
public:
	MaterialManager();
	~MaterialManager();
	void OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, bool metallic, bool crystal_blue, bool metal_gibs, bool shards, bool dev_glow);
	void OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, bool metallic, bool crystal_blue, bool metal_gibs, bool shards, bool dev_glow, const Color& rgba);
	
	static inline IMaterial* material_regular = nullptr;
	static inline IMaterial* material_flat = nullptr;
	static inline IMaterial* material_reflective = nullptr;
	static inline IMaterial* material_dogtags_outline = nullptr;
	static inline IMaterial* material_gloss = nullptr;
	static inline IMaterial* material_fbi_glass = nullptr;
	static inline IMaterial* material_cologne_prediction_glass = nullptr;
	static inline IMaterial* material_crystal_clear = nullptr;
	static inline IMaterial* material_gold = nullptr;
	static inline IMaterial* material_glass_glass = nullptr;
	static inline IMaterial* material_wildwire_gold = nullptr;
	static inline IMaterial* material_crystal_blue = nullptr;
	static inline IMaterial* material_metal_gibs = nullptr;
	static inline IMaterial* material_shards = nullptr;
	static inline IMaterial* material_dev_glow = nullptr;
	static inline IMaterial* material_regular_xqz = nullptr;
	static inline IMaterial* material_flat_xqz = nullptr;
	static inline IMaterial* material_reflective_xqz = nullptr;
};