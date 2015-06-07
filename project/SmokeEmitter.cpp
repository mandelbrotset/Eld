
#include "SmokeEmitter.h"
#include "Global.h"

SmokeEmitter::SmokeEmitter(int NUM_OF_PARTICLES, float radius, float hight, float3 position) : NUM_OF_PARTICLES(NUM_OF_PARTICLES), SMOKE_RADIUS(radius), SMOKE_HIGHT(hight), EMITTER_START_POSITION(position), BYTES_PER_FLOAT(4) {
		smokeParticles = new SmokeParticle*[NUM_OF_PARTICLES];
        	
        	// Init smoke particles
		for(int i = 0; i < NUM_OF_PARTICLES; i++) {
				smokeParticles[i] = new SmokeParticle();
        		initSmokeParticle(smokeParticles[i]);
       	}
        	
        	initBuffers();
			time = 0;
	}
	
	SmokeEmitter::~SmokeEmitter() {
		for(int i = 0; i < NUM_OF_PARTICLES; i++)
			delete smokeParticles[i];
	}
	
	void SmokeEmitter::draw(){
		
		int w = glutGet((GLenum)GLUT_WINDOW_WIDTH);
		int h = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
		glViewport(0, 0, w, h);
		
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glUseProgram(smokeShaderProgram);

		setUniformSlow(smokeShaderProgram, "viewMatrix", cameraViewMatrix);
		setUniformSlow(smokeShaderProgram, "projectionMatrix", cameraProjectionMatrix);
		setUniformSlow(smokeShaderProgram, "modelMatrix", make_identity<float4x4>());
		
		setUniformSlow(smokeShaderProgram, "uTime", time);

		glBindVertexArray(smokeArrayObject);
		glVertexAttribPointer(3, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/ );
		
		glDrawArrays(GL_POINTS, 0, NUM_OF_PARTICLES);
		glDepthMask(GL_TRUE);
		
		time = (time > 1000.0f) ? 0.0f : time + 0.005f;
	}
	
	void SmokeEmitter::initBuffers() {
		float* positions = getAllPositions();
		float* velocities = getAllVelocities();
		float* times = getAllTimes();
		
		glGenBuffers(1, &positionBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(positions) * NUM_OF_PARTICLES *3, positions, GL_STATIC_DRAW);
		
		glGenBuffers(1, &velocityBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, velocityBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(velocities) * NUM_OF_PARTICLES *3, velocities, GL_STATIC_DRAW);
		
		glGenBuffers(1, &timesBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, timesBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(times) * NUM_OF_PARTICLES *3, times, GL_STATIC_DRAW);
		
		// VBO
		glGenVertexArrays(1, &smokeArrayObject);
		glBindVertexArray(smokeArrayObject);
		
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/ );
		
		glBindBuffer(GL_ARRAY_BUFFER, velocityBuffer);
		glVertexAttribPointer(1, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/ );	
		
		glBindBuffer(GL_ARRAY_BUFFER, timesBuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/ );
		
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		
		// Shader program
		smokeShaderProgram = loadShaderProgram("smoke.vert", "smoke.frag"); 
		glBindAttribLocation(smokeShaderProgram, 0, "aPosition"); 	
		glBindAttribLocation(smokeShaderProgram, 1, "aVelocity");
		glBindAttribLocation(smokeShaderProgram, 2, "aParticleTimes");
		linkShaderProgram(smokeShaderProgram); 		
	}

    void SmokeEmitter::initSmokeParticle(SmokeParticle *particle) {        
        // Start position
        // Randomly distributed points in a circle:
        // x = sqrt(r)*cos(theta)
        // y = sqrt(r)*sin(theta)
        // r is a random number between origo and the radius. Theta is a random number between 0 and 2*pi
        float theta = nextFloat() * 2 * M_PI;
        float radius = nextFloat() * SMOKE_RADIUS;
        float startPositionX = sqrt(radius) * cos(theta) + EMITTER_START_POSITION.x;
        float startPositionY = EMITTER_START_POSITION.y;
        float startPositionZ = sqrt(radius) * sin(theta) + EMITTER_START_POSITION.z;

        // Initial velocity
        float velocityX = (1 * nextFloat() - 0.5f)/2;
        float velocityY = 0.1f;
        float velocityZ = (1 * nextFloat() - 0.5f)/2;

        // Times
        float lifetime = nextFloat() * SMOKE_HIGHT;
        float delay = nextFloat();

        // Set it!
        particle->setStartPosition(startPositionX, startPositionY, startPositionZ);
        particle->setInitVelocity(velocityX, velocityY, velocityZ);
        particle->setDelay(delay);
        particle->setLifetime(lifetime);
    }

    float* SmokeEmitter::getAllPositions() {
        float* positions = new float[NUM_OF_PARTICLES * 3];

	int index = 0;
        for(int i = 0; i < NUM_OF_PARTICLES; i++) {
        	float x = smokeParticles[i]->getStartPosition()[0];
        	float y = smokeParticles[i]->getStartPosition()[1];
        	float z = smokeParticles[i]->getStartPosition()[2];
        	
        	positions[index] = x;
        	index++;
        	positions[index] = y;
        	index++;
        	positions[index] = z;
        	index++;      	
        }

        return positions;
    }

    float* SmokeEmitter::getAllVelocities() {
        float* velocities = new float[NUM_OF_PARTICLES * 3];

	int index = 0;
        for(int i = 0; i < NUM_OF_PARTICLES; i++) {
            	float x = smokeParticles[i]->getInitVelocity()[0];
        	float y = smokeParticles[i]->getInitVelocity()[1];
        	float z = smokeParticles[i]->getInitVelocity()[2];
        	
        	velocities[index] = x;
        	index++;
        	velocities[index] = y;
        	index++;
        	velocities[index] = z;
        	index++;
        	
        }

        return velocities;
    }

    float* SmokeEmitter::getAllTimes() {
        float* times = new float[NUM_OF_PARTICLES * 3];

	int index = 0;
        for(int i = 0; i < NUM_OF_PARTICLES; i++) {
            float delay = smokeParticles[i]->getDelay();
            float lifetime = smokeParticles[i]->getLifetime();
            float age = smokeParticles[i]->getAge();

            times[index] = delay;
            index++;
            times[index] = lifetime;
            index++;
            times[index] = age;
            index++;
        }

        return times;
    }
    
    float SmokeEmitter::nextFloat() {
    	return static_cast <float> (rand()) /( static_cast <float> (RAND_MAX));
    }
