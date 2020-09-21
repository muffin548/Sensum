#pragma once
#include <vector>
#include "../valve_sdk/math/Vectors.hpp"

class glow_object_definition_t {
public:
	void set(float r, float g, float b, float a) {
		color = Vector(r, g, b);
		m_flAlpha = a;
		m_bRenderWhenOccluded = true;
		m_bRenderWhenUnoccluded = false;
		m_flBloomAmount = 1.0f;
	}
	void set(float r, float g, float b, float a, int style) {
		color = Vector(r, g, b);
		m_flAlpha = a;
		m_bRenderWhenOccluded = true;
		m_bRenderWhenUnoccluded = false;
		m_flBloomAmount = 1.0f;
		m_nGlowStyle = style; //0 - Exterior glow, 1 - Interior glow, 2 - Outline glow
	}
	bool unused() {
		return m_nNextFreeSlot != -2;
	}

	void* entity;
	Vector color;
	float   m_flAlpha;                 //0x0010
	uint8_t pad_0014[4];               //0x0014
	float   m_flSomeFloat;             //0x0018
	uint8_t pad_001C[4];               //0x001C
	float   m_flBloomAmount;          //0x0020
	bool    m_bRenderWhenOccluded;     //0x0024
	bool    m_bRenderWhenUnoccluded;   //0x0025
	bool    m_bFullBloomRender;        //0x0026
	uint8_t pad_0027[5];               //0x0027
	int32_t m_nGlowStyle;              //0x002C
	int32_t m_nSplitScreenSlot;        //0x0030
	int32_t m_nNextFreeSlot;           //0x0034
};

class glow_manager_t {
public:
	glow_object_definition_t* objects;
	char pad[8];
	int size;
};
