#include <float4x4.h>
#include <float3x3.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Silk.h"

using namespace chag;
using namespace std;
class SilkFire
{
public:
	SilkFire();
	~SilkFire(void);
	void draw(int currentTime);
	void updateSilke();
	float getAlpha();
	void makeRandomForce(float magnitude);
	GLuint getShaderProgram();
	void initSilk();
private:
	void updateTexture(Silk* silke, int currentTime);
};