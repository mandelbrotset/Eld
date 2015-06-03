#include "Silk.h"

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
#include <time.h>
#include <math.h>

float *positions;

Silk::Silk(int width, int height, float sizeX, float sizeY, int i)
{
	this->flameIndex = rand() %10;
	this->width = width;
	this->height = height;
	this->sizeX = sizeX;
	this->sizeY = sizeY;
	this->alphaFactor = 0.0f;
	this->index = i;
	createSilkPlane();
	positions = new float[width * height * 3]; // 75 w*h*3 antal noder * dimension
}

Silk::~Silk(void)
{
}

void Silk::initialize() {
	
}

void Silk::createSilkPlane() {

	float3 fireplacePosition = make_vector(0.6f, 0.2f, -2.51f);
	float xr, zr;
	xr = -0.05f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(0.05f-(-0.05f))));
	zr = -0.05f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(0.05f-(-0.05f))));
	float3 randomPosition = make_vector(xr, 0.0f, zr);
	fireplacePosition += randomPosition;
	
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			float xx = (float(x)/float(width)) *(sizeX) + fireplacePosition[0];
			float yy =  (float(y)/float(height)) * (sizeY) + fireplacePosition[1];
			float zz = fireplacePosition[2];
			float3 position = make_vector(xx, yy, fireplacePosition[2]);
			
			float eq = sizeX / float(width);
			Node* n = new Node(y == 0 && x == width / 2, 5.0f, 1000.0f, 1000.0f, 1000.0f, position, eq, sqrt(2.0f*eq*eq));
			nodes.push_back(n);
		}
	}
	
	addAdjacasentNodes();
}

void Silk::addAdjacasentNodes() {
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			Node* node = nodes[y * width + x];
			if (x > 0) { //left
				node->addAdjacensentNode(nodes[y * width + x - 1]);
			}
			if (x < width - 1) { //right
				node->addAdjacensentNode(nodes[y * width + x + 1]);
			}
			if (y > 0) { //down
				node->addAdjacensentNode(nodes[(y - 1) * width + x]);
			}
			if (y < height - 1) { //up
				node->addAdjacensentNode(nodes[(y + 1) * width + x]);
			}
			if (x > 0 && y > 0) { //diagonal down left
				node->addDiagonalNode(nodes[(y - 1) * width + (x - 1)]);
			}
			if (x < width - 1&& y < height - 1) { //diagonal up right
				node->addDiagonalNode(nodes[(y + 1) * width + (x + 1)]);
			}
			if (x > 0 && y < height - 1) { //diagonal up left
				node->addDiagonalNode(nodes[(y+1) * width + (x - 1)]);
			}
			if (x < width - 1 && y > 0) { //diagonal down right
				node->addDiagonalNode(nodes[(y-1) * width + (x + 1)]);
			}
			if (x < width - 2) { // bend springs right
				node->addSecondaryAdjacensentNode(nodes[y * width + x + 2]);
			}
			if (x > 1) { // bend springs left
				node->addSecondaryAdjacensentNode(nodes[y * width + (x - 2)]);
			}
			if (y < height - 2) { // bend springs up
				node->addSecondaryAdjacensentNode(nodes[(y + 2) * width + x]);
			}
			if (y > 1) { // bend springs down
				node->addSecondaryAdjacensentNode(nodes[(y - 2) * width + x]);
			}
		}
	}
}

void Silk::updateInternalForces() {
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			nodes[y * width + x]->updateInternalForces();
		}
	}
}

void Silk::updatePhysics(float time) {
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			nodes[y * width + x]->updatePhysics(time);
		}
	}
}

void Silk::applyExternalForce(int index, float3 force) {
	nodes[index]->applyExternalForce(force);
}

void Silk:: applyForceField(float3 towards) {
	for (int i = 0; i < width * height; i++) {
		nodes[i]->applyExternalForce(towards);
	}
}

float Silk:: getAlphaFactor() {
	return alphaFactor;
}

float minr = 10.0f;
float maxr = 20.0f;
void Silk:: updateFlame(float time) {
	float period = minr + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxr-minr)));
	alphaFactor = ((sin(time*period)+1.0f)/8.0f);
	flameIndex++;
	if (flameIndex >= 16) {
		flameIndex = 0;
	}
}

int Silk:: getFlameIndex() {
	return flameIndex;
}


float* Silk::getPositions() {
	for (int i = 0; i < width * height; i++) {
		positions[i * 3] = nodes[i]->getPosition()[0];
		positions[i * 3 + 1] = nodes[i]->getPosition()[1];
		positions[i * 3 + 2] = nodes[i]->getPosition()[2];
	}
	return positions;
}

int* Silk::getIndices() {
	int size = (width-1)*(height-1)*2*3;
	int *indices = new int[size]; //96 (w-1)*(h-1)*2*3 antal trianglar * 3
	int index = 0;
	for (int y = 0; y < height - 1; y++) {
		for (int x = 0; x < width - 1; x++) { // trianglar: 0, 1, 2 och 0, 2, 3
			indices[index] = y * width + x; // 0
			index++;
			indices[index] = y * width + x + width; // 1
			index++;
			indices[index] = y * width + x + width + 1; // 2
			index++;
			indices[index] = y * width + x; // 0
			index++;
			indices[index] = y * width + x + width + 1; // 2
			index++;			
			indices[index] = y * width + x + 1; // 3
			index++;
		}
	}
	return indices;
}

float* Silk::getTexCoords() {
	int size = width * height * 2;
	float *texCoords = new float[size]; //50 antal indeces (w)*(h)*2
	int index = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			float fx = (float) x;
			float fy = (float) y;
			float fwidth = (float) width;
			float fheight = (float) height;

			texCoords[index] = fx / (fwidth - 1.0f);
			index++;
			texCoords[index] = fy / (fheight - 1.0f);
			index++;
		}
	}
	return texCoords;
}