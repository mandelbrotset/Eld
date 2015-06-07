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
#include "Scene.h"
#include "SilkFire.h"

using namespace chag;
using namespace std;

namespace Scene {
GLuint shaderProgram;
GLuint shadowProgram;
const float3 up = {0.0f, 1.0f, 0.0f};

OBJModel *theIsland;
OBJModel *treeStam; 

float3 camera_lookat = make_vector(0.0f, 0.0f, 0.0f);
float3 camera_position = make_vector(0.0f, 0.0f, 0.0f);
float3 lightPosition = {0.6f, 0.6f, -2.4f};

float4x4 theIslandModelMatrix;
float4x4 treeStamModelMatrix;

GLuint shadowMapTexture;
GLuint shadowMapFBO;
const int shadowMapResolution = 1024*4;

float4x4 lightProjectionMatrix;
float4x4 lightViewMatrix;

float4x4 *cameraViewMatrix;
float4x4 *cameraProjectionMatrix;

bool nightVisionMode = false;

float r = 0.1f;
float g = 0.3f;
float b = 0.6f;

SilkFire* silkFire;

Scene::Scene(float4x4 *cvm, float4x4 *cpm) {
	cameraViewMatrix = cvm;
	cameraProjectionMatrix = cpm;
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
	*cameraProjectionMatrix = perspectiveMatrix(45.0f, float(w) / float(h), 0.1f, 1000.0f);
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
		setUniformSlow(shaderProgram, "viewMatrix", *cameraViewMatrix);
		setUniformSlow(shaderProgram, "projectionMatrix", *cameraProjectionMatrix);
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

	glClearColor(r,g,b,1.0);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	int w = glutGet((GLenum)GLUT_WINDOW_WIDTH);
	int h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, w, h);								
	glUseProgram( shaderProgram );	
	calculateSkitenKamera();
	calculateSkitenLjus();
	float4x4 lightMatrix = lightProjectionMatrix * lightViewMatrix * inverse(*cameraViewMatrix);
	setUniformSlow(shaderProgram, "lightMatrix", lightMatrix);
	
	setUniformSlow(shaderProgram, "diffuse_texture", 0);
	setUniformSlow(shaderProgram, "environmentMap", 1);
	setUniformSlow(shaderProgram, "inverseViewNormalMatrix", transpose(*cameraViewMatrix));

	float3 viewSpaceLightPos = transformPoint(*cameraViewMatrix, lightPosition); 
	setUniformSlow(shaderProgram, "viewSpaceLightPosition", viewSpaceLightPos);
	
	setUniformSlow(shaderProgram, "shadowMap", 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	
	setUniformSlow(shaderProgram, "nightVisionMode", (nightVisionMode) ? 1 : 0);
	setUniformSlow(shaderProgram, "lightDim", silkFire->getAlpha());
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

void Scene::toggleNightVisionMode() {
	nightVisionMode = !nightVisionMode;	
}

void Scene::changeLightPosition(float dx, float dy, float dz) {
	lightPosition[0] += dx;
	lightPosition[1] += dy;
	lightPosition[2] += dz;
}

}
