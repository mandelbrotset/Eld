#version 130

precision highp float;

in float transa;
out vec4 fragmentColor;

void main() 
{
	fragmentColor = vec4(1.0,1.0,1.0,transa);
}
