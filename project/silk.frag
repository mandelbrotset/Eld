#version 130

precision highp float;

in vec2 texCoord;

uniform sampler2D colorTexture;
uniform float alphaFactor;

out vec4 fragmentColor;

void main() 
{
	vec4 color = texture2D(colorTexture, texCoord.xy);
	color.a = color.a * alphaFactor;
	if(color.a < 0.000000000001)
		discard;
	fragmentColor = color;
}
