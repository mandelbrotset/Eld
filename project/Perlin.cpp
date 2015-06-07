#include "Perlin.h"
#include <float4x4.h>
#include <float3x3.h>
#include <random>
#include <time.h>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace chag;
using namespace std;
float3 *grid;

void Perlin::createGrid3D(int width, int height, int depth) {
	float gx, gy, gz;
	grid = new float3**[width];
	for (int x = 0; x < width; x++) {
		grid[x] = new float3*[height];
		for (int y = 0; y < height; y++) {
			grid[x][y] = new float3[depth];
			for (int z = 0; z < depth; z++) {
				gx = -1.0f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2.0f)));// TODO: use pseudo-random with a lookup-table instead!!!
				gy = -1.0f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2.0f)));
				gz = -1.0f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2.0f)));
				grid[x][y][z] = make_vector(gx, gy, gz);
				grid[x][y][z] = grid[x][y][z] * (1 / length(grid[x][y][z])); //make unit vector
			}
		}
	}
}

float Perlin::getValue(float x, float y, float z) {
	//find nearest grid nodes
	int x0 = (int)x;
	int x1 = x0+1;
	int y0 = (int)y;
	int y1 = y0+1;
	int z0 = (int)z;
	int z1 = z0+1;
	
	//calc distance vectors
	float dvx0 = x - float(x0);
	float dvx1 = x - float(x1);
	float dvy0 = y - float(y0);
	float dvy1 = y - float(y1);
	float dvz0 = z - float(z0);
	float dvz1 = z - float(z1);
	float3 dvx0y0z0 = make_vector(dvx0, dvy0, dvz0);
	float3 dvx1y0z0 = make_vector(dvx1, dvy0, dvz0);
	float3 dvx0y1z0 = make_vector(dvx0, dvy1, dvz0);
	float3 dvx1y1z0 = make_vector(dvx1, dvy1, dvz0);
	float3 dvx0y0z1 = make_vector(dvx0, dvy0, dvz1);
	float3 dvx1y0z1 = make_vector(dvx1, dvy0, dvz1);
	float3 dvx0y1z1 = make_vector(dvx0, dvy1, dvz1);
	float3 dvx1y1z1 = make_vector(dvx1, dvy1, dvz1);

	//calc interpolation weights
	float wx = x - (float)x0;
	float wy = y - (float)y0;
	float wz = z - (float)z0;

	//get gradient vectors
	float3 gx0y0z0 = grid[x0][y0][z0];
	float3 gx1y0z0 = grid[x1][y0][z0];
	float3 gx0y1z0 = grid[x0][y1][z0];
	float3 gx1y1z0 = grid[x1][y1][z0];
	float3 gx0y0z1 = grid[x0][y0][z1];
	float3 gx1y0z1 = grid[x1][y0][z1];
	float3 gx0y1z1 = grid[x0][y1][z1];
	float3 gx1y1z1 = grid[x1][y1][z1];

	float dotx0y0z0 = dotProduct(gx0y0z0, dvx0y0z0);
	float dotx1y0z0 = dotProduct(gx1y0z0, dvx1y0z0);
	float dotx0y1z0 = dotProduct(gx0y1z0, dvx0y1z0);
	float dotx1y1z0 = dotProduct(gx1y1z0, dvx1y1z0);
	float dotx0y0z1 = dotProduct(gx0y0z1, dvx0y0z1);
	float dotx1y0z1 = dotProduct(gx1y0z1, dvx1y0z1);
	float dotx0y1z1 = dotProduct(gx0y1z1, dvx0y1z1);
	float dotx1y1z1 = dotProduct(gx1y1z1, dvx1y1z1);

	float iy0z0 = interpolation(dotx0y0z0, dotx1y0z0, wx);
	float iy1z0 = interpolation(dotx0y1z0, dotx1y1z0, wx);
	float iy0z1 = interpolation(dotx0y0z1, dotx1y0z1, wx);
	float iy1z1 = interpolation(dotx0y1z1, dotx1y1z1, wx);
	float iz0 = interpolation(iy0z0, iy1z0, wy);
	float iz1 = interpolation(iy0z1, iy1z1, wy);
	float result = interpolation (iz0, iz1, wz);
	//return result;
	return molnerize(result);
}

float Perlin::interpolate(float a0, float a1, float w) {
	return a0*(1 - w) + a1*w;
}

float Perlin::linear(float a0, float a1, float w) {
	return a0 + w*(a1-a0);
}

float Perlin::poly(float w) {
	return 3*w*w - 2*w*w*w;
}

float Perlin::interpolation(float a0, float a1, float w) {
	return linear(a0, a1, poly(w));
}

float Perlin::interpolate3(float a0, float a1, float w) {
	return (1.0f-w)*a0 + w*a1;
}

float Perlin::dotProduct(float3 v1, float3 v2) {
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

float Perlin::molnerize(float value) {
	value = (value + 1.0f) / 2.0f;
	return value;
	//float factor = 0.5f * (1.0f+sin(M_PI*value + M_PI * 3.0f/2.0f));
	//return value * factor;
}