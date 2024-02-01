#version 430 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 transformation;

out vec3 texCoord;

void main()
{
	texCoord = vertexPosition;
	gl_Position = transformation * vec4(vertexPosition, 1.0);
}