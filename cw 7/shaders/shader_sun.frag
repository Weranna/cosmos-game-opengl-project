#version 430 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

float AMBIENT = 0.3;
float DIFFUSE_INTENSITY = 0.2;

uniform vec3 lightDir;
uniform sampler2D sunAlbedo;
uniform sampler2D sunNormal;

in vec3 vecNormal;
in vec3 fragPos;
in vec2 vecTex;

void main()
{
    vec3 normal = normalize(vecNormal);

    vec3 normalFromMap = normalize(texture2D(sunNormal, vecTex).xyz * 2.0 - 1.0);

    float diffuse = max(0, dot(normalFromMap, lightDir));
    diffuse = pow(diffuse, DIFFUSE_INTENSITY); // Apply intensity control

    vec4 albedoColor = texture2D(sunAlbedo, vecTex);
    vec3 finalColor = albedoColor.rgb * min(1,diffuse + AMBIENT);

    FragColor = vec4(finalColor, 1.0);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.1f)
        BrightColor = vec4(FragColor.rgb, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}