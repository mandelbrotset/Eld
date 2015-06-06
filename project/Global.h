#include <float4x4.h>
#include <float3x3.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Silk.h"

namespace {
	const float CLEAR_COLOR_R;
	const float CLEAR_COLOR_G;
	const float CLEAR_COLOR_B;

	float4x4 cameraViewMatrix;
	float4x4 cameraProjectionMatrix;
}

