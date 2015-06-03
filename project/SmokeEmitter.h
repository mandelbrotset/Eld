
#include <float3x3.h>
#include <float4x4.h>
#include <random>
#include <time.h>
#include <math.h>
#include "SmokeParticle.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <IL/il.h>
#include <IL/ilut.h>

#include <stdlib.h>
#include <algorithm>

#include <OBJModel.h>
#include <glutil.h>


using namespace chag;

class SmokeEmitter {
	private:
	const int NUM_OF_PARTICLES;
    const int BYTES_PER_FLOAT;
    const float SMOKE_RADIUS;
    const float SMOKE_HIGHT;
    const float3 EMITTER_START_POSITION;

	SmokeParticle** smokeParticles;
	float time;
	
	void initBuffers();
	void initSmokeParticle(SmokeParticle* smokeParticle);
	float* getAllPositions();
	float* getAllVelocities();
	float* getAllTimes();
	
	float nextFloat();
	
	// OpenGL
	GLuint positionBuffer;
	GLuint velocityBuffer;
	GLuint timesBuffer;
	GLuint smokeArrayObject;
	GLuint smokeShaderProgram;
	
	public:
	SmokeEmitter(int NUM_OF_PARTICLES, float radius, float hight, float3 position);
	~SmokeEmitter();
	
	void draw(float4x4 cameraViewMatrix, float4x4 cameraProjectionMatrix, float4x4 modelMatrix);
	
};
