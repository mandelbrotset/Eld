#include <float4x4.h>
#include <float3x3.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Silk.h"

using namespace chag;
using namespace std;
class Scene
{
public:
	Scene(SilkFire* silk);
	~Scene();
	void initScene();
	void draw();
	void toggleNightVisionMode();
	void changeLightPosition(float dx, float dy, float dz);
	
private:
	void drawModel(OBJModel *model, const float4x4 &modelMatrix, bool shadow);
	void drawShadowCasters();
	void calculateSkitenLjus();
	void calculateSkitenKamera();
};