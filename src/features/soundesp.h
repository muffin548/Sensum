#pragma once

#include "../valve_sdk/math/Vectors.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include "../settings/settings.h"

class c_sound_info {
public:
	c_sound_info(Vector positions, float times, int userids) {
		this->position = positions;
		this->time = times;
		this->userid = userids;
	}

	Vector position;
	float time;
	int userid;
};

class c_sound_esp {
public:
	void draw() noexcept;
	void draw_circle(Color colors, Vector position) noexcept;
	void event_player_footstep(IGameEvent* event) noexcept;
	void event_player_hurt(IGameEvent* event) noexcept;
};

extern c_sound_esp sound_esp;