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

using namespace chag;
using namespace std;

namespace Scene {

class Scene
{
public:
	Scene(float4x4 *cvm, float4x4 *cpm);
	void draw();
	void toggleNightVisionMode();
	void initScene();
	void changeLightPosition(float dx, float dy, float dz);
private:
	void drawShadowCasters();
	void drawModel(OBJModel *model, const float4x4 &modelMatrix, bool shadow);
	void calculateSkitenKamera();
	void calculateSkitenLjus();
};

}
