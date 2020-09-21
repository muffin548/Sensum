#pragma once

#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/MaterialManager.h"
#include "../helpers/singleton.hpp"
#include "../valve_sdk/interfaces/IStudioRender.h"
#include <deque>

enum class ChamsModes : int
{
	regular,
	flat,
	wireframe,
	glass,
	reflective,
	crystal_blue,
	metal_gibs,
	shards,
	dev_glow,
	regular_xqz,
	flat_xqz,
	reflective_xqz
};

class Chams : public Singleton<Chams>
{
public:
	void OnSceneEnd();
};
