#version 130

precision highp float;

// inputs from vertex shader.
in vec4 color;
in vec2 texCoord;
in vec3 viewSpacePosition; 
in vec3 viewSpaceNormal; 
in vec3 viewSpaceLightPosition; 
in vec4 shadowMapCoord;

uniform sampler2D shadowMapTex;

// output to frame buffer.
out vec4 fragmentColor;

// global uniforms, that are the same for the whole scene
uniform sampler2DShadow shadowMap;

uniform vec3 scene_ambient_light = vec3(0.7, 0.7, 0.7);
uniform vec3 scene_light = vec3(3.9, 2.9, 2.9);

// object specific uniforms, change once per object but are the same for all materials in object.
uniform float object_alpha; 
uniform float object_reflectiveness = 0.0;

// matrial properties, changed when material changes.
uniform float material_shininess;
uniform vec3 material_diffuse_color; 
uniform vec3 material_specular_color; 
uniform vec3 material_emissive_color; 
uniform int has_diffuse_texture; 
uniform sampler2D diffuse_texture;
uniform samplerCube environmentMap;
uniform mat4 inverseViewNormalMatrix;
uniform int nightVisionMode;
uniform float lightDim;
vec3 calculateAmbient(vec3 ambientLight, vec3 materialAmbient)
{
	return ambientLight * materialAmbient;
}

vec3 calculateDiffuse(vec3 diffuseLight, vec3 materialDiffuse, vec3 normal, vec3 directionToLight)
{
	float diffuse = 0.5;
	return diffuseLight * diffuse * materialDiffuse * max(0, dot(normal, directionToLight));
}

vec3 calculateSpecular(vec3 specularLight, vec3 materialSpecular, float materialShininess, vec3 normal, vec3 directionToLight, vec3 directionFromEye)
{
	float shine = 0.5f;
	vec3 h = normalize(directionToLight - directionFromEye);
	float normalizationFactor = ((shine + 0.5) / 8.0);
	vec3 specularColor = specularLight * materialSpecular * pow(max(0, dot(h, normal)), materialShininess) * normalizationFactor;
	return specularColor;
}

vec3 calculateFresnel(vec3 materialSpecular, vec3 normal, vec3 directionFromEye)
{
	return materialSpecular + (vec3(1.0) - materialSpecular) * pow(clamp(1.0 + dot(directionFromEye, normal), 0.0, 1.0), 5.0) * 0.5f;
}

float getNightVisionColor() {
	if (nightVisionMode == 1) {
		if (viewSpaceLightPosition.y > 0) {
			return 200.0f;
		}
		return 20.5f;
	} else {
		return 1.0f;
	}
}

void main() 
{

	
	vec3 diffuse = material_diffuse_color;
	vec3 specular = material_specular_color;
	vec3 emissive = material_emissive_color;
	vec3 ambient = material_diffuse_color;
	
	if (has_diffuse_texture == 1)
	{
		diffuse *= texture(diffuse_texture, texCoord.xy).xyz; 
		ambient *= texture(diffuse_texture, texCoord.xy).xyz; 
		emissive *= texture(diffuse_texture, texCoord.xy).xyz; 
	}

	vec3 normal = normalize(viewSpaceNormal);
	vec3 directionToLight = normalize(viewSpaceLightPosition - viewSpacePosition);
	vec3 directionFromEye = normalize(viewSpacePosition);
	vec3 fresnelSpecular = calculateFresnel(specular, normal, directionFromEye);


	vec3 reflectionVector = (inverseViewNormalMatrix * vec4(reflect(directionFromEye, normal), 0.0)).xyz;
	vec3 envMapSample = texture(environmentMap, reflectionVector).rgb;

	vec3 diffuseColor = (has_diffuse_texture == 1) ? 
	texture(diffuse_texture, texCoord.xy).xyz : material_diffuse_color; 

	vec3 posToLight = normalize(viewSpaceLightPosition - viewSpacePosition);
	float diffuseReflectance = max(0.0, dot(posToLight, normalize(viewSpaceNormal)));

	float visibility = textureProj(shadowMap, shadowMapCoord);
	vec2 coord = shadowMapCoord.xy / shadowMapCoord.w;
	
	vec3 shading = calculateAmbient(scene_ambient_light, ambient)+ 0.1*calculateSpecular(scene_light, fresnelSpecular, material_shininess, normal, directionToLight, directionFromEye) + emissive + envMapSample * fresnelSpecular * object_reflectiveness;

	shading = vec3(shading.x, shading.y * getNightVisionColor(), shading.z);

	fragmentColor = vec4(calculateDiffuse(scene_light, diffuse, normal, directionToLight) * lightDim * visibility + shading, object_alpha);
}




