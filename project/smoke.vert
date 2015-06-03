
// Attributes
attribute vec3 aPosition;
attribute vec3 aVelocity;
attribute vec3 aParticleTimes;

// Uniforms
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform float uTime;

// Varying
varying float vAge;
varying vec3 vColor;

void main(void) {
	mat4 mvMatrix = viewMatrix * modelMatrix;
	mat4 mvpMatrix = projectionMatrix * mvMatrix;

    vec3 particleColor;
    vec3 newPos;
    float ageFactor;
    float delay = aParticleTimes.x;
    float lifetime = aParticleTimes.y;
    float age = mod(uTime, lifetime);
	vec3 particleAcceleration = vec3(0.0, 2.0, 0.0); // Use this to simulate wind for instance!

    if(age > delay) {
        // x = x0 + v*t+(a*t^2)/2
        // t = particle age
        newPos = aPosition + aVelocity * (age - delay) + 0.5 * particleAcceleration * (age - delay) * (age - delay);



        // The older the particle, the smaller and darker it will be
        ageFactor = 1.0 - ((age - delay) / lifetime);
        ageFactor = clamp(ageFactor, 0.0, 1.0);

        gl_PointSize = 15.0 * ageFactor; //Maybe divide by length(camera - particle)

    } else {
        newPos = aPosition;
        gl_PointSize = 0.0;
        ageFactor = 0.0;
    }

    particleColor = vec3(0.2 * ageFactor, 0.2 * ageFactor, 0.2 * ageFactor);
    vAge = ageFactor;
    vColor = particleColor;

    gl_Position = mvpMatrix * vec4(newPos, 1.0);
}
