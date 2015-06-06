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
#include "Silk.h"
#include "Global.h"

using namespace chag;
using namespace std;

const int nrOfSilken = 10;
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
float3 forceField = make_vector(0.0f, 0.03f, 0.0f);

SilkFire::SilkFire() {

}

SilkFire::~SilkFire() {

}

void SilkFire::initSilk() {
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
		updateTexture(silken[s], 0);
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

void SilkFire::updateTexture(Silk* silke, int currentTime) {
	int index = silke->getFlameIndex();
	silke->updateFlame(currentTime);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, eldTextures[index]);
}

void SilkFire::updateSilke() {
	for (int s = 0; s < nrOfSilken; s++) {
		silken[s]->applyForceField(make_vector(0.0f, 0.01f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1.5f-0.01f))), 0.0f));
		silken[s]->updateInternalForces();
		silken[s]->updatePhysics(0.01667);
	}
}

void SilkFire::draw(int currentTime) {
	int w = glutGet((GLenum)GLUT_WINDOW_WIDTH);
	int h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, w, h);
	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);
	glUseProgram( silkShaderProgram );

	float4x4 projectionMatrix = perspectiveMatrix(45.0f, float(w)/float(h), 0.01f, 300.0f); 

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	setUniformSlow(silkShaderProgram, "viewMatrix", Global::cameraViewMatrix);
	setUniformSlow(silkShaderProgram, "projectionMatrix", Global::cameraProjectionMatrix);
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
		updateTexture(silken[s], currentTime);
		setUniformSlow(silkShaderProgram, "alphaFactor", silken[s]->getAlphaFactor());
		
		glAlphaFunc(GL_GREATER, 0.0);
		glEnable(GL_ALPHA_TEST);
		glDrawElements(GL_TRIANGLES, (silkwidth-1)*(silkheight-1)*2*3, GL_UNSIGNED_INT, 0);
	}
	glDepthMask(GL_TRUE);
	glUseProgram( 0 );
}

float SilkFire::getAlpha() {
	//float alphaSum = 0.0f;
	//for (int s = 0; s < nrOfSilken / 3; s++) {
//		alphaSum += silken[s]->getAlphaFactor();
//	} den blinkar för lite om man gör så
	return silken[0]->getAlphaFactor();
}

void SilkFire::makeRandomForce(float magnitude) {
	for (int s = 0; s < nrOfSilken; s++) {
		forceField[0] = float(rand() % 200 - 100) * 0.1f;
		forceField[1] = float(rand() % 4 - 2) * 0.01f;
		forceField[2] = float(rand() % 200 - 100) * 0.2f;
		silken[s]->applyForceField(forceField);
	}
}

GLuint SilkFire::getShaderProgram() {
	return silkShaderProgram;
}