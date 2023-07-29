#pragma once
#include "Vector4.h"
#include "Matrix4x4.h"
#include <cstdint>

enum EnableLighting {

	None = 0,
	Lambert = 1,
	HalfLambert = 2,

};

struct MaterialData {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};
