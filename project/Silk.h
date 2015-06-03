#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <IL/il.h>
#include <IL/ilut.h>

#include <stdlib.h>
#include <algorithm>

#include <OBJModel.h>
#include <glutil.h>
#include <float4x4.h>
#include <float3x3.h>
#include "Node.h"

using namespace chag;
using namespace std;

class Silk
{
public:

	Silk(int width, int height, float sizeX, float sizeY, int i);
	~Silk(void);
	void initialize();
	void updateInternalForces();
	void applyExternalForce(int index, float3 force);
	void updatePhysics(float time);
	void updateFlame(float time);
	void applyForceField(float3 towards);
	int getFlameIndex();
	int* getIndices();
	float* getTexCoords();
	float* getPositions();
	float getAlphaFactor();
private:
	// vi antar ideala fjädrar samt ignorerar kvantmekanik
	float alphaFactor;
	int width;
	int height;
	float sizeX;
	float sizeY;
	int index;
	int flameIndex;
	vector<Node*> nodes;
	void createSilkPlane();
	void addAdjacasentNodes();
};

