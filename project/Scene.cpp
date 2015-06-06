#include "Perlin.h"
#include <float4x4.h>
#include <float3x3.h>
#include <random>
#include <time.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "SilkFire.h"
#include <glutil.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <IL/il.h>
#include <IL/ilut.h>
#include <fstream>
#include "Scene.h"
#include "Global.h"

using namespace chag;
using namespace std;

GLuint shaderProgram;
GLuint shadowProgram;
OBJModel *theIsland;
OBJModel *treeStam; 
float4x4 theIslandModelMatrix;
float4x4 treeStamModelMatrix;

GLuint shadowMapTexture;
GLuint shadowMapFBO;
const int shadowMapResolution = 1024*4;
bool nightVisionMode = false;
GLuint positionBuffer, colorBuffer, indexBuffer, vertexArrayObject, textureBuffer, texture;

SilkFire* silk;

float4x4 lightProjectionMatrix;
float4x4 lightViewMatrix;
float3 lightPosition = {0.6f, 0.6f, -2.4f};
const float3 up = {0.0f, 1.0f, 0.0f};

Scene::Scene(SilkFire* silkfire) {
	silk = silkfire;
}

Scene::~Scene() {

}

void Scene::drawModel(OBJModel *model, const float4x4 &modelMatrix, bool shadow)
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
		setUniformSlow(shaderProgram, "viewMatrix", Global::cameraViewMatrix);
		setUniformSlow(shaderProgram, "projectionMatrix", Global::cameraProjectionMatrix);
		setUniformSlow(shaderProgram, "modelMatrix", modelMatrix);
	}
	model->render();
	glUseProgram( currentProgram );
}

void Scene::drawShadowCasters()
{
	calculateSkitenKamera();
	drawModel(theIsland, make_identity<float4x4>(), false);
	setUniformSlow(shaderProgram, "object_reflectiveness", 0.1f); 
	drawModel(treeStam, make_translation(make_vector(0.0f, 0.0f, 0.0f)), false); 
	setUniformSlow(shaderProgram, "object_reflectiveness", 0.0f); 
}

void Scene::draw()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);	

	glClearColor(Global::CLEAR_COLOR_R, Global::CLEAR_COLOR_G, Global::CLEAR_COLOR_B, 1.0);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	int w = glutGet((GLenum)GLUT_WINDOW_WIDTH);
	int h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, w, h);								
	glUseProgram( shaderProgram );	
	calculateSkitenKamera();
	calculateSkitenLjus();
	float4x4 lightMatrix = lightProjectionMatrix * lightViewMatrix * inverse(Global::cameraViewMatrix);
	setUniformSlow(shaderProgram, "lightMatrix", lightMatrix);
	
	setUniformSlow(shaderProgram, "diffuse_texture", 0);
	setUniformSlow(shaderProgram, "environmentMap", 1);
	setUniformSlow(shaderProgram, "inverseViewNormalMatrix", transpose(Global::cameraViewMatrix));

	float3 viewSpaceLightPos = transformPoint(Global::cameraViewMatrix, lightPosition); 
	setUniformSlow(shaderProgram, "viewSpaceLightPosition", viewSpaceLightPos);
	
	setUniformSlow(shaderProgram, "shadowMap", 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	
	setUniformSlow(shaderProgram, "nightVisionMode", (nightVisionMode) ? 1 : 0);
	setUniformSlow(shaderProgram, "lightDim", silk->getAlpha());
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

void Scene::initScene() {
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
}

void Scene::calculateSkitenLjus() {
	int w = glutGet((GLenum)GLUT_WINDOW_WIDTH);
	int h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
	lightViewMatrix = lookAt(lightPosition, make_vector(0.0f, 0.0f, 0.0f), up);
	lightProjectionMatrix = perspectiveMatrix(90.0f, 1.0, 1.0f, 1000.0f);
}

void Scene::calculateSkitenKamera() {
	int w = glutGet((GLenum)GLUT_WINDOW_WIDTH);
	int h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
	Global::cameraProjectionMatrix = perspectiveMatrix(45.0f, float(w) / float(h), 0.1f, 1000.0f);
}

void Scene::changeLightPosition(float dx, float dy, float dz) {
	lightPosition[0] += dx;
	lightPosition[1] += dy;
	lightPosition[2] += dz;
}

void Scene::toggleNightVisionMode() {
	nightVisionMode = !nightVisionMode;
}