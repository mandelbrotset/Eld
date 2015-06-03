

class SmokeParticle {
	private:
	float age;
	float lifetime;
	float delay;
	float* startPosition;
	float* initVelocity;

	public:
	SmokeParticle();
	~SmokeParticle();

	float getAge();
	float getLifetime();
	float getDelay();
	float* getStartPosition();
	float* getInitVelocity();

	void setAge(float age);
	void setLifetime(float lifetime);
	void setDelay(float delay);
	void setStartPosition(float* startPosition);
	void setStartPosition(float x, float y, float z);
	void setInitVelocity(float* initVelocity);
	void setInitVelocity(float x, float y, float z);

};
