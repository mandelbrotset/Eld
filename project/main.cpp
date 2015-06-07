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
#include "Cloud.h"
#include "SilkFire.h"
#include "Scene.h"

using namespace std;
using namespace chag;

float currentTime = 0.0f;
float lastTime = 0.0f;

float4x4 cameraViewMatrix;
float4x4 cameraProjectionMatrix;

GLuint positionBuffer, colorBuffer, indexBuffer, vertexArrayObject, textureBuffer, texture;

Scene::Scene *scene;
/*******************************  SILKE    **************************************/
SilkFire* silkFire;
/********************************************************************************/

/******************************* THE CLOUD **************************************/
float cradius = 150.0f;
Cloud* cloud;
/********************************************************************************/

/********************************* WATER ****************************************/
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
/********************************************************************************/

/********************************* SMOKE ****************************************/
SmokeEmitter *smokeEmitter;
/********************************************************************************/

float3 c_position = {0.0f, 0.0f, 0.0f};
float yaw = 50.0f;
float pitch = 45.0f;
float3 c_translation = {0.0f, 0.0f, 0.0f};
float3 c_target = {0.0f, 0.0f, 1.0f};
bool leftDown = false;
bool middleDown = false;
bool rightDown = false;
bool key_down[256];
int prev_x = 0;
int prev_y = 0;

void drawWater() {
	int w = glutGet((GLenum)GLUT_WINDOW_WIDTH);
	int h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, w, h);
	glDisable(GL_CULL_FACE);
	glUseProgram(silkFire->getShaderProgram());	
	float4x4 projectionMatrix = perspectiveMatrix(45.0f, float(w)/float(h), 0.01f, 300.0f); 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	setUniformSlow(silkFire->getShaderProgram(), "viewMatrix", cameraViewMatrix);
	setUniformSlow(silkFire->getShaderProgram(), "projectionMatrix", cameraProjectionMatrix);
	setUniformSlow(silkFire->getShaderProgram(), "modelMatrix", make_identity<float4x4>());
	setUniformSlow(silkFire->getShaderProgram(), "alphaFactor", 1.0f);
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

	glUseProgram(silkFire->getShaderProgram());
	int texLoc = glGetUniformLocation(silkFire->getShaderProgram(), "colorTexture");
	glUniform1i(texLoc, 0);
}

float3 sphericalTotreeStamtesian(float theta, float phi, float r) {
	return make_vector( r * sinf(theta)*sinf(phi),
					 	r * cosf(phi), 
						r * cosf(theta)*sinf(phi) );
}

void initObjects() {
	cloud = new Cloud(cradius);
	cloud->initCloud();

	silkFire = new SilkFire();
	silkFire->initSilk();

	initWater();

	float3 smokePosition = {0.9f, 0.3f, -2.51f};
	smokeEmitter = new SmokeEmitter(100000, 0.001f, 2.0f, smokePosition);

	scene = new Scene::Scene(&cameraViewMatrix, &cameraProjectionMatrix);
	scene->initScene();
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
	
	initObjects();
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void display(void)
{
	silkFire->updateSilke();
	scene->draw();
	drawWater();
	silkFire->draw(cameraViewMatrix, cameraProjectionMatrix, currentTime);
	cloud->draw(cameraViewMatrix, cameraProjectionMatrix);
	smokeEmitter->draw(cameraViewMatrix, cameraProjectionMatrix, make_identity<float4x4>());
	glutSwapBuffers();
	CHECK_GL_ERROR();
}

void handleKeyUp(unsigned char key, int x, int y) {
	key_down[key] = false;

	switch (key) {
	case 101: //e
		cloud->increaseCloudSpeed();
		break;
	case 113: //q
		cloud->decreaseCloudSpeed();
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
		scene->toggleNightVisionMode();
		break;
	case 120: //x
		scene->changeLightPosition(1, 0, 0);
		break;
	case 121: //y
		scene->changeLightPosition(0, 1, 0);
		break;
	case 122: //z
		scene->changeLightPosition(0, 0, 2);
		break;
	case 117: //u
		scene->changeLightPosition(0, 1, 0);
		break;
	case 105: //i
		scene->changeLightPosition(0, -1, 0);
		break;
	case 111: //o
		silkFire->updateSilke();
		break;
	case 108: //l
		silkFire->makeRandomForce(10.0f);
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

void init(int argc, char *argv[]) {
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
}

int main(int argc, char *argv[])
{
	init(argc, argv);
	glutMainLoop();
	return 0;          
}
