#ifdef WIN32
#include <windows.h>
#endif


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
#include "Silk.h"
#include <String.h>
#include <time.h>
#include "Perlin.h"
#include "SmokeEmitter.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;
using namespace chag;

float currentTime = 0.0f;
float lastTime = 0.0f;
GLuint shaderProgram;
GLuint shadowProgram;
const float3 up = {0.0f, 1.0f, 0.0f};

OBJModel *theIsland;
OBJModel *treeStam; 

float3 c_position = {0.0f, 0.0f, 0.0f};
float yaw = 50.0f;
float pitch = 45.0f;
float3 c_translation = {0.0f, 0.0f, 0.0f};
float3 c_target = {0.0f, 0.0f, 1.0f};

float3 camera_lookat = make_vector(0.0f, 0.0f, 0.0f);
float3 camera_position = make_vector(0.0f, 0.0f, 0.0f);
float3 lightPosition = {0.6f, 0.6f, -2.4f};

bool leftDown = false;
bool middleDown = false;
bool rightDown = false;
bool key_down[256];
int prev_x = 0;
int prev_y = 0;

float4x4 theIslandModelMatrix;
float4x4 treeStamModelMatrix;

GLuint shadowMapTexture;
GLuint shadowMapFBO;
const int shadowMapResolution = 1024*4;

float4x4 cameraViewMatrix;
float4x4 cameraProjectionMatrix;

float4x4 lightProjectionMatrix;
float4x4 lightViewMatrix;

bool nightVisionMode = false;
const int nrOfSilken = 10;

GLuint positionBuffer, colorBuffer, indexBuffer, vertexArrayObject, textureBuffer, texture;

/******************************* THE SILK **************************************/
Silk* silken[nrOfSilken];
int* indices;
float* texcoords;
float* positions[nrOfSilken];
GLuint		ppositionBuffer[nrOfSilken], pcolorBuffer, pindexBuffer, pvertexArrayObject[nrOfSilken];
GLuint		ptextureBuffer;
GLuint		ptexture;
GLuint		eldTextures[18];
GLuint		silkShaderProgram;
int silkwidth = 5;
int silkheight = 5;

/******************************* THE CLOUD **************************************/
const int cpsize = 7;
const int octaves = 3;
const float cradius = 150.0f;
const float cradstep = 0.01f;
const float cscale = float(cpsize - 1) / (cradius * 2);
const float maxFrequency = pow(2, octaves - 1);
const int cisize = (M_PI + cradstep) / cradstep;
const int csize = cisize * cisize;
bool cloudUp = true; //move noise up or down
int nrOfCloudCalcs = 10;
int currentCloud = 0;
const float offsetRadius = 30.0f;
int moveCloud = 0;
int cloudSpeed = 10;

GLuint cindexBuffer;
GLuint cloudPositionBuffer;
GLuint cloudTransaBuffer;
GLuint cloudVertexArrayObject;
GLuint cloudShaderProgram;
int *cloudIndices;
float* cloudPositions;
float** cloudTransas;
Perlin* perlin;

/**WATER**/

GLuint wpositionBuffer, wcolorBuffer, windexBuffer, wtextureBuffer, wtexture, wvertexArrayObject, waterShader;
const float waterPositions[] = {
	-cradius, 0.0f, -cradius,
	cradius, 0.0f, -cradius,
	-cradius, 0.0f, cradius,
	cradius, 0.0f, cradius
};

const int waterIndices[] = {
	0, 2, 1,
	2, 3, 1
};

const float waterTexCoords[] = {
	0, 0,
	100.0f, 0,
	0, 100.0f,
	100.0f, 100.0f
};


SmokeEmitter *smokeEmitter;

//clear color:
float r = 0.1f;
float g = 0.3f;
float b = 0.6f;

float cToPerlin(float f) {
	return (f + cradius * maxFrequency) * cscale;
}

void initCloudIndices() {
	cloudIndices = new int[cisize * cisize * 3 * 2];
	int x, z, index;
	index = 0;
	for (z = 0; z < cisize - 1; z++) {
		for (x = 0; x < cisize - 1; x++) {	
			cloudIndices[index] = z * cisize + x + cisize; //1
			index++;
			cloudIndices[index] = z * cisize + x; //0
			index++;
			cloudIndices[index] = z * cisize + x + cisize + 1; //2
			index++;
			cloudIndices[index] = z * cisize + x; //0
			index++;
			cloudIndices[index] = z * cisize + x + 1; //3
			index++;
			cloudIndices[index] = z * cisize + x + cisize + 1; //2
			index++;
		}
	}
	glGenBuffers(1, &cindexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cindexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cloudIndices) * (cisize-1)*(cisize-1)*2*3, cloudIndices, GL_STATIC_DRAW );
}

void printPosition(int index) {
	printf("node%i: %f; %f; %f\n", index, cloudPositions[cloudIndices[index]], cloudPositions[cloudIndices[index]+1], cloudPositions[cloudIndices[index]+2]);
}

void printPositions() {
	for (int i = 0; i < csize * 3; i++) {
		printf("pos %i: %f\n", i, cloudPositions[i]);
	}
}

void printIndices() {
	for (int i = 0; i < (cisize - 1) * (cisize - 1) * 3 * 2; i++) {
		printf("ind: %i: %i\n", i, cloudIndices[i]);
	}
}

void initCloudPositions() {
	cloudPositions = new float[csize*3];
	int pindex = 0;
	int xindex = 0;
	int zindex = 0;
	float x, z, xx, zz, yy;
	x = 0;
	z = 0;
	for (zindex = 0; zindex < cisize; zindex++) {
		x = 0;
		for (xindex = 0; xindex < cisize; xindex++) {	
			xx = cradius * sin(x) * cos(z);
			yy = cradius * sin(x) * sin(z);
			zz = cradius * cos(x);
			cloudPositions[pindex] = xx;
			pindex++;
			cloudPositions[pindex] = yy;
			pindex++;
			cloudPositions[pindex] = zz;
			pindex++;
			x += cradstep;
		}
		z += cradstep;
	}
	glGenBuffers(1, &cloudPositionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cloudPositionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * csize * 3, cloudPositions, GL_STATIC_DRAW );
}

float getPerlinValue(float x, float y, float z) {
	float frequency = 1.0f;
	float amplitude = 1.0f;
	float transa = 0.0f;
	for (int i = 0; i < octaves; i++) {
		transa += perlin->getValue(cToPerlin(x * frequency), cToPerlin(y * frequency), cToPerlin(z * frequency)) * amplitude;
		if (i + 1 < octaves) {
			frequency *= 2.0f;
			amplitude /= 2.0f;
		}
	}
	return transa;
}

void normalizeTransas(float min, float max) {
	int tindex;
	int xindex;
	int zindex;
	float transa;
	float size = max - min;
	for (int i = 0; i < nrOfCloudCalcs; i++) {
		tindex = 0;
		xindex = 0;
		zindex = 0;
		for (zindex = 0; zindex < cisize; zindex++) {
			for (xindex = 0; xindex < cisize; xindex++) {	
				transa = cloudTransas[i][tindex];
				cloudTransas[i][tindex] = (transa - min) / size;
				tindex++;
			}
		}
	}
}

void computeTransas() {
	int tindex;
	int xindex;
	int zindex;
	float x, z, transa, xx, zz, yy;
	float min = 10000.0f;
	float max = -10000.0f;
	float radOffset = 0.0f;
	float xOffset = 0.0f;
	float zOffset = 0.0f;
	x = 0;
	z = 0;
	for (int i = 0; i < nrOfCloudCalcs; i++) {
		printf("Computing transa %i", i);
		tindex = 0;
		zindex = 0;
		xindex = 0;
		z = 0;
		zOffset = offsetRadius * sin(radOffset);
		xOffset = offsetRadius * cos(radOffset);

		cloudTransas[i] = new float[csize];
		for (zindex = 0; zindex < cisize; zindex++) {
			x = 0;
			for (xindex = 0; xindex < cisize; xindex++) {	
				xx = offsetRadius + cradius * sin(x) * cos(z) + xOffset;
				yy = cradius * sin(x) * sin(z);
				zz = offsetRadius + cradius * cos(x) + zOffset;
				transa = getPerlinValue(xx, yy, zz);
				cloudTransas[i][tindex] = transa;
				tindex++;
				x += cradstep;
				if (transa < min) min = transa;
				if (transa > max) max = transa;
			}
			z += cradstep;
		}
		radOffset += 2 * M_PI / float(nrOfCloudCalcs);
		printf(" done\n");
	}
	normalizeTransas(min, max);
	for (int i = 0; i < nrOfCloudCalcs; i++) {
		printf("transa %i: %f\n", i, cloudTransas[i][1000]);
	}
}

void updateTransas() {
	moveCloud++;
	if (moveCloud > cloudSpeed) {
		moveCloud = 0;

		currentCloud += 1;
		if (currentCloud >= nrOfCloudCalcs) {
			currentCloud = 0;
		}
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, cloudTransaBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * csize, cloudTransas[currentCloud], GL_STATIC_DRAW );
}

void initTransas() {
	glGenBuffers(1, &cloudTransaBuffer);
	cloudTransas = new float*[nrOfCloudCalcs];
	int start = glutGet(GLUT_ELAPSED_TIME);
	computeTransas();
	updateTransas();
	int end = glutGet(GLUT_ELAPSED_TIME);
	printf("time: %i\n", end - start);
}

void initPerlin() {
	printf("csize: %i\n", csize);
	perlin = new Perlin();
	perlin->createGrid3D(cpsize * maxFrequency + 2 * (int)offsetRadius + 2, cpsize * maxFrequency * 2, cpsize * maxFrequency + 2 * (int)offsetRadius + 2);
}

void fixCloudVertexAttribThings() {
	glGenVertexArrays(1, &cloudVertexArrayObject);
	glBindVertexArray(cloudVertexArrayObject);

	glBindBuffer( GL_ARRAY_BUFFER, cloudPositionBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/ );	
	
	glBindBuffer( GL_ARRAY_BUFFER, cloudTransaBuffer);
	glVertexAttribPointer(1, 1, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/ );
	
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cindexBuffer );

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

void setupCloudShader() {
	cloudShaderProgram = loadShaderProgram("CloudPoint.vert", "CloudPoint.frag"); 
	glBindAttribLocation(cloudShaderProgram, 0, "position");
	glBindAttribLocation(cloudShaderProgram, 1, "trans");
	glBindFragDataLocation(cloudShaderProgram, 0, "fragmentColor");
	
	linkShaderProgram(cloudShaderProgram); 

	glUseProgram(cloudShaderProgram);
}

void initCloud() {
	initPerlin();
	initCloudIndices();
	initCloudPositions();
	initTransas();
	fixCloudVertexAttribThings();
	setupCloudShader();
}

void drawCloud() {
	updateTransas();
	int w = glutGet((GLenum)GLUT_WINDOW_WIDTH);
	int h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, w, h);
	glDisable(GL_CULL_FACE);
	glUseProgram(cloudShaderProgram);
	float4x4 projectionMatrix = perspectiveMatrix(45.0f, float(w)/float(h), 0.01f, 300.0f); 
	setUniformSlow(cloudShaderProgram, "viewMatrix", cameraViewMatrix);
	setUniformSlow(cloudShaderProgram, "projectionMatrix", cameraProjectionMatrix);
	setUniformSlow(cloudShaderProgram, "modelMatrix", make_identity<float4x4>());
	glBindVertexArray(cloudVertexArrayObject);
	glVertexAttribPointer(2, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/ );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, cindexBuffer );
	glDrawElements(GL_TRIANGLES, (cisize - 1) * (cisize - 1) * 3 * 2, GL_UNSIGNED_INT, 0);
	glUseProgram( 0 );
}

void drawWater() {
	int w = glutGet((GLenum)GLUT_WINDOW_WIDTH);
	int h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, w, h);
	glDisable(GL_CULL_FACE);
	glUseProgram( silkShaderProgram );	
	float4x4 projectionMatrix = perspectiveMatrix(45.0f, float(w)/float(h), 0.01f, 300.0f); 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	setUniformSlow(silkShaderProgram, "viewMatrix", cameraViewMatrix);
	setUniformSlow(silkShaderProgram, "projectionMatrix", cameraProjectionMatrix);
	setUniformSlow(silkShaderProgram, "modelMatrix", make_identity<float4x4>());
	setUniformSlow(silkShaderProgram, "alphaFactor", 1.0f);
	glEnable(GL_NORMALIZE);
	glBindBuffer(GL_ARRAY_BUFFER, wpositionBuffer);
	glBindVertexArray(wvertexArrayObject);
	glVertexAttribPointer(4, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/ );
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, windexBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wtexture);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glUseProgram( 0 );
}

void updateTexture(Silk* silke) {
	int index = silke->getFlameIndex();
	silke->updateFlame(currentTime);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, eldTextures[index]);
}

void initWater() {
	glGenBuffers(1, &wpositionBuffer);
	glBindBuffer( GL_ARRAY_BUFFER, wpositionBuffer);
	glBufferData( GL_ARRAY_BUFFER, sizeof(waterPositions) * 12, waterPositions, GL_STATIC_DRAW );

	glGenBuffers( 1, &windexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, windexBuffer );										
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(waterIndices) * 6, waterIndices, GL_STATIC_DRAW );			

	glGenBuffers(1, &wtextureBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, wtextureBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(waterTexCoords) * 8, waterTexCoords, GL_STATIC_DRAW);

	glGenVertexArrays(1, &wvertexArrayObject);
	glBindVertexArray(wvertexArrayObject);
	glBindBuffer( GL_ARRAY_BUFFER, wpositionBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);	
	
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, windexBuffer );
	glBindBuffer(GL_ARRAY_BUFFER, wtextureBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	wtexture = ilutGLLoadImage("../scenes/water.png");

	glUseProgram(silkShaderProgram);
	int texLoc = glGetUniformLocation(silkShaderProgram, "colorTexture");
	glUniform1i(texLoc, 0);
}

void initSilk() {
	float size;
	float min = 0.4f;
	float max = 0.8f;
	for (int s = 0; s < nrOfSilken; s++) {
		size = min + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(max-min)));
		silken[s] = new Silk(silkwidth, silkheight, size, size, s);
		positions[s] = silken[s]->getPositions();
	}

	indices = silken[0]->getIndices();
	texcoords = silken[0]->getTexCoords();

	for (int s = 0; s < nrOfSilken; s++) {
		glGenBuffers(1, &ppositionBuffer[s]);
		glBindBuffer( GL_ARRAY_BUFFER, ppositionBuffer[s]);
		glBufferData( GL_ARRAY_BUFFER, sizeof(positions[s]) * silkwidth * silkheight * 3, positions[s], GL_STATIC_DRAW );
	}

	glGenBuffers( 1, &pindexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, pindexBuffer );										
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) * (silkwidth-1)*(silkheight-1)*2*3, indices, GL_STATIC_DRAW );			

	glGenBuffers(1, &ptextureBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, ptextureBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords) * silkwidth * silkheight * 2, texcoords, GL_STATIC_DRAW);
	
	for (int s = 0; s < nrOfSilken; s++) {
		glGenVertexArrays(1, &pvertexArrayObject[s]);
		glBindVertexArray(pvertexArrayObject[s]);
		glBindBuffer( GL_ARRAY_BUFFER, ppositionBuffer[s]);
		glVertexAttribPointer(0, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/ );	
		
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, pindexBuffer );
		glBindBuffer(GL_ARRAY_BUFFER, ptextureBuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}

	silkShaderProgram = loadShaderProgram("silk.vert", "silk.frag"); 
	glBindAttribLocation(silkShaderProgram, 0, "position"); 	
	glBindAttribLocation(silkShaderProgram, 1, "texCoordIn");
	glBindFragDataLocation(silkShaderProgram, 0, "fragmentColor");
	linkShaderProgram(silkShaderProgram); 
	
	eldTextures[0] = ilutGLLoadImage("../scenes/fire/fire2/fire1.png");
	eldTextures[1] = ilutGLLoadImage("../scenes/fire/fire2/fire2.png");
	eldTextures[2] = ilutGLLoadImage("../scenes/fire/fire2/fire3.png");
	eldTextures[3] = ilutGLLoadImage("../scenes/fire/fire2/fire4.png");
	eldTextures[4] = ilutGLLoadImage("../scenes/fire/fire2/fire5.png");
	eldTextures[5] = ilutGLLoadImage("../scenes/fire/fire2/fire6.png");
	eldTextures[6] = ilutGLLoadImage("../scenes/fire/fire2/fire7.png");
	eldTextures[7] = ilutGLLoadImage("../scenes/fire/fire2/fire8.png");
	eldTextures[8] = ilutGLLoadImage("../scenes/fire/fire2/fire9.png");
	eldTextures[9] = ilutGLLoadImage("../scenes/fire/fire2/fire10.png");
	eldTextures[10] = ilutGLLoadImage("../scenes/fire/fire2/fire11.png");
	eldTextures[11] = ilutGLLoadImage("../scenes/fire/fire2/fire12.png");
	eldTextures[12] = ilutGLLoadImage("../scenes/fire/fire2/fire13.png");
	eldTextures[13] = ilutGLLoadImage("../scenes/fire/fire2/fire14.png");
	eldTextures[14] = ilutGLLoadImage("../scenes/fire/fire2/fire15.png");
	eldTextures[15] = ilutGLLoadImage("../scenes/fire/fire2/fire16.png");
	
	for (int s = 0; s < nrOfSilken; s++) {
		updateTexture(silken[s]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0F);
	}
	
	glUseProgram(silkShaderProgram);
	int texLoc = glGetUniformLocation(silkShaderProgram, "colorTexture");
	glUniform1i(texLoc, 0);
}

float3 sphericalTotreeStamtesian(float theta, float phi, float r) {
	return make_vector( r * sinf(theta)*sinf(phi),
					 	r * cosf(phi), 
						r * cosf(theta)*sinf(phi) );
}

void initGL()
{
	glewInit();  

	startupGLDiagnostics();
	setupGLDebugMessages();

	ilInit();
	ilutRenderer(ILUT_OPENGL);

	if( !glBindFragDataLocation )
	{
		glBindFragDataLocation = glBindFragDataLocationEXT;
	}
	
	initCloud();
	initSilk();
	initWater();
	float3 smokePosition = {0.9f, 0.3f, -2.51f};
	smokeEmitter = new SmokeEmitter(100000, 0.001f, 2.0f, smokePosition);

	shaderProgram = loadShaderProgram("simple.vert", "simple.frag");
	glBindAttribLocation(shaderProgram, 0, "position"); 	
	glBindAttribLocation(shaderProgram, 2, "texCoordIn");
	glBindAttribLocation(shaderProgram, 1, "normalIn");
	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");
	linkShaderProgram(shaderProgram);

	shadowProgram = loadShaderProgram("shadow.vert", "shadow.frag");
	glBindAttribLocation(shadowProgram, 0, "position");
	glBindFragDataLocation(shadowProgram, 0, "fragmentColor");	
	linkShaderProgram(shadowProgram);

	theIsland = new OBJModel(); 
	theIsland->load("../scenes/forest.obj");

	treeStam = new OBJModel(); 
	treeStam->load("../scenes/treestam.obj");

	theIslandModelMatrix = make_scale<float4x4>(make_vector(1.0f, 1.0f, 1.0f));
	treeStamModelMatrix = make_translation(make_vector(0.0f, 1.0f, 0.0f));

	glGenTextures(1, &shadowMapTexture);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, shadowMapResolution, shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	float4 zeros = {1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &zeros.x);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

	glBindTexture(GL_TEXTURE_2D, 0);
	glGenFramebuffers(1, &shadowMapFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void calculateSkitenLjus() {
	int w = glutGet((GLenum)GLUT_WINDOW_WIDTH);
	int h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
	lightViewMatrix = lookAt(lightPosition, make_vector(0.0f, 0.0f, 0.0f), up);
	lightProjectionMatrix = perspectiveMatrix(90.0f, 1.0, 1.0f, 1000.0f);
}

void calculateSkitenKamera() {
	int w = glutGet((GLenum)GLUT_WINDOW_WIDTH);
	int h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
	cameraProjectionMatrix = perspectiveMatrix(45.0f, float(w) / float(h), 0.1f, 1000.0f);
}

void drawModel(OBJModel *model, const float4x4 &modelMatrix, bool shadow)
{
	GLint currentProgram; 
	glGetIntegerv( GL_CURRENT_PROGRAM, &currentProgram );

	if (shadow) {
		calculateSkitenLjus();
		glUseProgram( shadowProgram );
		setUniformSlow(shadowProgram, "viewMatrix", lightViewMatrix);
		setUniformSlow(shadowProgram, "projectionMatrix", lightProjectionMatrix);
		setUniformSlow(shadowProgram, "modelMatrix", modelMatrix); 
	} else {
		calculateSkitenKamera();
		glUseProgram( shaderProgram );
		setUniformSlow(shaderProgram, "viewMatrix", cameraViewMatrix);
		setUniformSlow(shaderProgram, "projectionMatrix", cameraProjectionMatrix);
		setUniformSlow(shaderProgram, "modelMatrix", modelMatrix);
	}
	model->render();
	glUseProgram( currentProgram );
}

void drawShadowCasters()
{
	calculateSkitenKamera();
	drawModel(theIsland, make_identity<float4x4>(), false);
	setUniformSlow(shaderProgram, "object_reflectiveness", 0.1f); 
	drawModel(treeStam, make_translation(make_vector(0.0f, 0.0f, 0.0f)), false); 
	setUniformSlow(shaderProgram, "object_reflectiveness", 0.0f); 
}

void drawScene(void)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);	

	glClearColor(r,g,b,1.0);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	int w = glutGet((GLenum)GLUT_WINDOW_WIDTH);
	int h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, w, h);								
	glUseProgram( shaderProgram );	
	calculateSkitenKamera();
	calculateSkitenLjus();
	float4x4 lightMatrix = lightProjectionMatrix * lightViewMatrix * inverse(cameraViewMatrix);
	setUniformSlow(shaderProgram, "lightMatrix", lightMatrix);
	
	setUniformSlow(shaderProgram, "diffuse_texture", 0);
	setUniformSlow(shaderProgram, "environmentMap", 1);
	setUniformSlow(shaderProgram, "inverseViewNormalMatrix", transpose(cameraViewMatrix));

	float3 viewSpaceLightPos = transformPoint(cameraViewMatrix, lightPosition); 
	setUniformSlow(shaderProgram, "viewSpaceLightPosition", viewSpaceLightPos);
	
	setUniformSlow(shaderProgram, "shadowMap", 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	
	setUniformSlow(shaderProgram, "nightVisionMode", (nightVisionMode) ? 1 : 0);
	float alphaSum = 0.0f;
	for (int s = 0; s < nrOfSilken / 3; s++) {
		alphaSum += silken[s]->getAlphaFactor();
	}

	setUniformSlow(shaderProgram, "lightDim", silken[0]->getAlphaFactor());
	setUniformSlow(shaderProgram, "lightpos", lightPosition); 

	drawShadowCasters();

	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	setUniformSlow(shaderProgram, "object_alpha", 1.0f); 
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE); 

	glUseProgram( 0 );	
}

void updateCamera() {

	float4x4 yawRotation = 
  { 
    { cos(yaw), 0.0f, sin(yaw), 0.0f },
    { 0.0f, 1.0f, 0.0f, 0.0f },
    { -sin(yaw), 0.0f, cos(yaw), 0.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f }
  };

	float4x4 pitchRotation = 
  { 
    { 1.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, cos(pitch), -sin(pitch), 0.0f },
    { 0.0f, sin(pitch), cos(pitch), 0.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f }
  };

	float4x4 rotation = yawRotation * pitchRotation;

	c_translation = transformPoint(yawRotation, c_translation); //?

	c_position += c_translation;
	c_translation[0] = 0.0f;
	c_translation[1] = 0.0f;
	c_translation[2] = 0.0f;
	float3 temp = {0.0f, 1.0f, 0.0f};
	float3 forward = transformPoint(rotation, temp);
	c_target = c_position + forward;
	float3 up3 = {0.0f, 0.0f, 1.0f};
	float3 up = transformPoint(rotation, up3);
	cameraViewMatrix = lookAt(c_position, c_target, up);
}

void drawSilk() {
	int w = glutGet((GLenum)GLUT_WINDOW_WIDTH);
	int h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, w, h);
	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);
	glUseProgram( silkShaderProgram );

	float4x4 projectionMatrix = perspectiveMatrix(45.0f, float(w)/float(h), 0.01f, 300.0f); 

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	setUniformSlow(silkShaderProgram, "viewMatrix", cameraViewMatrix);
	setUniformSlow(silkShaderProgram, "projectionMatrix", cameraProjectionMatrix);
	setUniformSlow(silkShaderProgram, "modelMatrix", make_identity<float4x4>());

	for (int s = 0; s < nrOfSilken; s++) {
		positions[s] = silken[s]->getPositions();
		glBindBuffer( GL_ARRAY_BUFFER, ppositionBuffer[s]);
		glBufferData( GL_ARRAY_BUFFER, sizeof(positions[s]) * silkwidth * silkheight * 3, positions[s], GL_STATIC_DRAW );
		glBindVertexArray(pvertexArrayObject[s]);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/ );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, pindexBuffer );
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		updateTexture(silken[s]);
		setUniformSlow(silkShaderProgram, "alphaFactor", silken[s]->getAlphaFactor());
		
		glAlphaFunc(GL_GREATER, 0.0);
		glEnable(GL_ALPHA_TEST);
		glDrawElements(GL_TRIANGLES, (silkwidth-1)*(silkheight-1)*2*3, GL_UNSIGNED_INT, 0);
	}
	glDepthMask(GL_TRUE);
	glUseProgram( 0 );
}

int i;
float3 forceField = make_vector(0.0f, 0.03f, 0.0f);
void makeRandomForce(float magnitude) {
	for (int s = 0; s < nrOfSilken; s++) {
		forceField[0] = float(rand() % 200 - 100) * 0.1f;
		forceField[1] = float(rand() % 4 - 2) * 0.01f;
		forceField[2] = float(rand() % 200 - 100) * 0.2f;
		silken[s]->applyForceField(forceField);
	}
}

void updateSilke() {
	float t = currentTime - lastTime;
	//printf("FPS: %f\n", 1.0f / t);
	for (int s = 0; s < nrOfSilken; s++) {
		silken[s]->applyForceField(make_vector(0.0f, 0.01f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1.5f-0.01f))), 0.0f));
		silken[s]->updateInternalForces();
		silken[s]->updatePhysics(0.01667);
	}
	lastTime = currentTime;
}

void display(void)
{
	updateSilke();
	drawScene();
	drawSilk();
	drawWater();
	drawCloud();
	smokeEmitter->draw(cameraViewMatrix, cameraProjectionMatrix, make_identity<float4x4>());
	glutSwapBuffers();
	CHECK_GL_ERROR();
}

void handleKeyUp(unsigned char key, int x, int y) {
	key_down[key] = false;

	switch (key) {
	case 101: //e
		cloudSpeed++;
		break;
	case 113: //q
		if (cloudSpeed > 1) cloudSpeed--;
		break;
	}
}

void handleKeys(unsigned char key, int /*x*/, int /*y*/)
{
	key_down[key] = true;
	c_translation[0] = 0.0f;
	c_translation[1] = 0.0f;
	c_translation[2] = 0.0f;
	switch(key)
	{
	case 27: //ESC
		exit(0);
		break;
	case 110: //n
		nightVisionMode = !nightVisionMode;
		break;
	case 120: //x
		lightPosition[0]+=1;
		break;
	case 121: //y
		lightPosition[1]+=1;
		break;
	case 122: //z
		lightPosition[2]+=2;
		break;
	case 117: //u
		camera_position[1]+=1;
		break;
	case 105: //i
		camera_position[1]-=1;
		break;
	case 111: //o
		updateSilke();
		break;
	case 108: //l
		makeRandomForce(10.0f);
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	prev_x = x;
	prev_y = y;
	int delta_x = x - prev_x;
	int delta_y = y - prev_y;
}

void handleMovements() {
	if (key_down[119]) { //w
		c_translation[2] = -0.1;
	}
	if (key_down[97]) { //a
		c_translation[0] = -0.1;
	}
	if (key_down[115]) { //s
		c_translation[2] = 0.1;
	}
	if (key_down[100]) { //d
		c_translation[0] = 0.1;
	}
	if (key_down[32]) { //space
		c_translation[1] = 0.1;
	}
	if (key_down['c']) { //c
		c_translation[1] = -0.1;
	}
}

void handleCamera(int i) {
	handleMovements();
	updateCamera();
	glutTimerFunc(1, handleCamera, 0);
}

bool just_warped = false;
void motion(int x, int y)
{
	if (just_warped) {
		just_warped = false;
		return;
	}
	int delta_x = (float)(x - glutGet(GLUT_WINDOW_WIDTH)/2);
	int delta_y = (float)(y - glutGet(GLUT_WINDOW_HEIGHT)/2);

	yaw += delta_x*0.001f;
	pitch += delta_y*0.001f;

	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
	just_warped = true;
}

void idle( void )
{
	static float startTime = float(glutGet(GLUT_ELAPSED_TIME)) / 1000.0f;
	currentTime = float(glutGet(GLUT_ELAPSED_TIME)) / 1000.0f - startTime;
	glutPostRedisplay();  
}

void setCallbackFunctions() {
	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(handleKeys);
	glutKeyboardUpFunc(handleKeyUp);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutTimerFunc(1, handleCamera, 0);
}

void initgluten() {
	glutInitWindowSize(800,600);
	glutInitContextVersion(3,0);
	glutInitContextFlags(GLUT_DEBUG);
	glutCreateWindow("Silk fire, particle smoke and perlin clouds");
}

int main(int argc, char *argv[])
{
#	if defined(__linux__)
	linux_initialize_cwd();
#	endif // ! __linux__

	glutInit(&argc, argv);
#	if defined(GLUT_SRGB)
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
#	else
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	
	printf( "--\n" );
	printf( "-- WARNING: your GLUT doesn't support sRGB / GLUT_SRGB\n" );
#	endif
	srand (time(0));
	initgluten();
	setCallbackFunctions();
	initGL();
	glutMainLoop();
	return 0;          
}
