
#include "SmokeParticle.h"

    SmokeParticle::SmokeParticle() {
        startPosition = new float[3];
        initVelocity = new float[3];
        age = lifetime = delay = 0;
    }

	SmokeParticle::~SmokeParticle() {
		delete []  startPosition;
		delete [] initVelocity;
	}

    float SmokeParticle::getAge() {
        return age;
    }

    void SmokeParticle::setAge(float age) {
        this->age = age;
    }

    float SmokeParticle::getLifetime() {
        return lifetime;
    }

    void SmokeParticle::setLifetime(float lifetime) {
        this->lifetime = lifetime;
    }

    float SmokeParticle::getDelay() {
        return delay;
    }

    void SmokeParticle::setDelay(float delay) {
        this->delay = delay;
    }

    float* SmokeParticle::getStartPosition() {
        return startPosition;
    }

    void SmokeParticle::setStartPosition(float* startPosition) {
        this->startPosition = startPosition;
    }

    void SmokeParticle::setStartPosition(float x, float y, float z) {
        startPosition[0] = x;
        startPosition[1] = y;
        startPosition[2] = z;
    }

    void SmokeParticle::setInitVelocity(float x, float y, float z) {
        initVelocity[0] = x;
        initVelocity[1] = y;
        initVelocity[2] = z;
    }

    float* SmokeParticle::getInitVelocity() {
        return initVelocity;
    }

    void SmokeParticle::setInitVelocity(float* initVelocity) {
        this->initVelocity = initVelocity;
    }

