#include <float4x4.h>
#include <float3x3.h>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace chag;
using namespace std;
class Cloud
{
public:
	Cloud(float hemisphereRadius);
	~Cloud(void);
	void initCloud();
	void decreaseCloudSpeed();
	void increaseCloudSpeed();
	int getCloudSpeed();
	void draw(float4x4 cameraViewMatrix, float4x4 cameraProjectionMatrix);
private:
	float Cloud::cToPerlin(float f);
	void Cloud::initCloudIndices();
	void Cloud::printPosition(int index);
	void Cloud::printPositions();
	void Cloud::printIndices();
	void Cloud::initCloudPositions();
	float Cloud::getPerlinValue(float x, float y, float z);
	void Cloud::normalizeTransas(float min, float max);
	void Cloud::computeTransas();
	void Cloud::updateTransas();
	void Cloud::initTransas();
	void Cloud::initPerlin();
	void Cloud::fixCloudVertexAttribThings();
	void Cloud::setupCloudShader();
	void writeToFile();
	void readFromFile();
	bool fileExist(const char *fileName);
};
