#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;

uniform mat4 transformation;
uniform mat4 modelMatrix;

out vec2 vecTex;
out vec3 vecNormal;
out vec3 fragPos; // Position in world space

void main()
{
    fragPos = (modelMatrix * vec4(vertexPosition, 1.0)).xyz;
    vecNormal = normalize((modelMatrix * vec4(vertexNormal, 0.0)).xyz);
    vecTex = vec2(vertexTexCoord.x, vertexTexCoord.y);
    gl_Position = transformation * vec4(vertexPosition, 1.0);
}
