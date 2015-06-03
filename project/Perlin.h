
#include <float4x4.h>
#include <float3x3.h>

using namespace chag;
using namespace std;
class Perlin
{
public:
	float getValue(float x, float y, float z);
	void createGrid3D(int width, int height, int depth);
private:
	float3 ***grid;
	float dotProduct(float3 v1, float3 v2);
	float interpolate(float a0, float a1, float w);
	float linear(float a0, float a1, float w);
	float interpolate3(float a0, float a1, float w);
	float poly(float w);
	float interpolation(float a0, float a1, float w);
	float max;
	float min;
	float molnerize(float value);
};