#version 430 core

float AMBIENT = 0.1;

uniform vec3 lightPos;
uniform sampler2D sunAlbedo;
uniform sampler2D sunNormal;

in vec3 vecNormal;
in vec3 worldPos;
in vec2 vecTex;

out vec4 outColor;

void main()
{
    vec3 lightDir = normalize(lightPos - worldPos);
    vec3 normal = normalize(vecNormal);

    vec3 normalFromMap = normalize(texture2D(sunNormal, vecTex).xyz * 2.0 - 1.0);

    float diffuse = max(0, dot(normalFromMap, lightDir));

    vec4 albedoColor = texture2D(sunAlbedo, vecTex);
    vec3 finalColor = albedoColor.rgb * (diffuse + AMBIENT);

    outColor = vec4(finalColor, 1.0);
}
