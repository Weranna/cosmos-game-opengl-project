#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

uniform mat4 transformation;
uniform mat4 modelMatrix;

out vec3 vecNormal;
out vec3 worldPos;

uniform vec3 lightPos;
uniform vec3 spotlightPos;
uniform vec3 cameraPos;

out vec3 viewDirTS;
out vec3 lightDirTS;
out vec3 spotlightDirTS;
out vec2 vecTex;

void main()
{
	worldPos = (modelMatrix* vec4(vertexPosition,1)).xyz;
	vecNormal = normalize((modelMatrix* vec4(vertexNormal,0)).xyz);
	gl_Position = transformation * vec4(vertexPosition, 1.0);
	vec3 w_tangent = normalize(mat3(modelMatrix)*vertexTangent);
	vec3 w_bitangent = normalize(mat3(modelMatrix)*vertexBitangent);
	mat3 TBN = transpose(mat3(w_tangent, w_bitangent, vecNormal));
	
	vec3 V = normalize(cameraPos-worldPos);
	viewDirTS = TBN*V;
	vec3 L = normalize(lightPos-worldPos);
	lightDirTS = TBN*L;
	vec3 SL = normalize(spotlightPos-worldPos);
	spotlightDirTS = TBN*SL;

	vecTex = vertexTexCoord;
    vecTex.y = 1.0 - vecTex.y;
}
