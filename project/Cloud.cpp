#include "Perlin.h"
#include <float4x4.h>
#include <float3x3.h>
#include <random>
#include <time.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Cloud.h"
#include <glutil.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <IL/il.h>
#include <IL/ilut.h>
#include <fstream>

using namespace chag;
using namespace std;

const int cpsize = 7;
const int octaves = 3;
const float cradstep = 0.01f;
float hemisphereRad;
float cscale;
const float maxFrequency = pow(2, octaves - 1);
const int cisize = (M_PI + cradstep) / cradstep;
const int csize = cisize * cisize;
const int nrOfCloudCalcs = 100;
const float offsetRadius = 5.0f;
int currentCloud = 0;
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

bool computeTrans = false;

Cloud::Cloud(float hemisphereRadius) {
	hemisphereRad = hemisphereRadius;
	cscale = float(cpsize - 1) / (hemisphereRad * 2);
}

Cloud::~Cloud() {

}

float Cloud::cToPerlin(float f) {
	return (f + hemisphereRad * maxFrequency) * cscale;
}

void Cloud::initCloudIndices() {
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

void Cloud::printPosition(int index) {
	printf("node%i: %f; %f; %f\n", index, cloudPositions[cloudIndices[index]], cloudPositions[cloudIndices[index]+1], cloudPositions[cloudIndices[index]+2]);
}

void Cloud::printPositions() {
	for (int i = 0; i < csize * 3; i++) {
		printf("pos %i: %f\n", i, cloudPositions[i]);
	}
}

void Cloud::printIndices() {
	for (int i = 0; i < (cisize - 1) * (cisize - 1) * 3 * 2; i++) {
		printf("ind: %i: %i\n", i, cloudIndices[i]);
	}
}

void Cloud::initCloudPositions() {
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
			xx = hemisphereRad * sin(x) * cos(z);
			yy = hemisphereRad * sin(x) * sin(z);
			zz = hemisphereRad * cos(x);
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

float Cloud::getPerlinValue(float x, float y, float z) {
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

void Cloud::normalizeTransas(float min, float max) {
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

void Cloud::computeTransas() {
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
				xx = offsetRadius + hemisphereRad * sin(x) * cos(z) + xOffset;
				yy = hemisphereRad * sin(x) * sin(z);
				zz = offsetRadius + hemisphereRad * cos(x) + zOffset;
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

void Cloud::updateTransas() {
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

bool Cloud::fileExist(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

void Cloud::initTransas() {
	glGenBuffers(1, &cloudTransaBuffer);
	cloudTransas = new float*[nrOfCloudCalcs];
	int start = glutGet(GLUT_ELAPSED_TIME);
	if (fileExist("data.bin")) {
		printf("Data file found!\n");
		readFromFile();
	} else {
		printf("No data file found. Computing..\n");
		computeTransas();
		writeToFile();
	}
	int end = glutGet(GLUT_ELAPSED_TIME);
	printf("time: %i\n", end - start);
}

void Cloud::initPerlin() {
	printf("csize: %i\n", csize);
	perlin = new Perlin();
	perlin->createGrid3D(cpsize * maxFrequency + 2 * (int)offsetRadius + 2, cpsize * maxFrequency * 2, cpsize * maxFrequency + 2 * (int)offsetRadius + 2);
}

void Cloud::fixCloudVertexAttribThings() {
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

void Cloud::setupCloudShader() {
	cloudShaderProgram = loadShaderProgram("CloudPoint.vert", "CloudPoint.frag"); 
	glBindAttribLocation(cloudShaderProgram, 0, "position");
	glBindAttribLocation(cloudShaderProgram, 1, "trans");
	glBindFragDataLocation(cloudShaderProgram, 0, "fragmentColor");
	
	linkShaderProgram(cloudShaderProgram); 

	glUseProgram(cloudShaderProgram);
}

void Cloud::initCloud() {
	initPerlin();
	initCloudIndices();
	initCloudPositions();
	initTransas();
	fixCloudVertexAttribThings();
	setupCloudShader();
}

void Cloud::draw(float4x4 cameraViewMatrix, float4x4 cameraProjectionMatrix) {
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

int Cloud::getCloudSpeed() {
	return cloudSpeed;
}

void Cloud::decreaseCloudSpeed() {
	if (cloudSpeed > 1) cloudSpeed--;
}

void Cloud::increaseCloudSpeed() {
	cloudSpeed++;
}

//FILE IO

void Cloud::writeToFile() {
	printf("writing ");
	ofstream file ("data.bin", ios::binary);
	for (int i = 0; i < nrOfCloudCalcs; i++) {
		file.write((char*)cloudTransas[i], sizeof (float) * csize);
		printf(".");
	}
	file.close();
	printf(" done\n");
}

void Cloud::readFromFile() {
	printf("reading ");	
	ifstream file ("data.bin", ios::binary);
	for (int i = 0; i < nrOfCloudCalcs; i++) {
		cloudTransas[i] = new float[csize];
		file.read((char*)cloudTransas[i], sizeof(float) * csize);
		printf(".");
	}
	file.close();
	printf(" done\n");
}