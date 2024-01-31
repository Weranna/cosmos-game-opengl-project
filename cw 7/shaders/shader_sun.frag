// shader_sun.frag
#version 430 core

float AMBIENT = 0.3; // Adjust the ambient lighting
float DIFFUSE_INTENSITY = 2.0; // Adjust the diffuse intensity

uniform vec3 lightDir; // Directional light direction
uniform sampler2D sunAlbedo;
uniform sampler2D sunNormal;

in vec3 vecNormal;
in vec3 fragPos; // Position in world space
in vec2 vecTex;

out vec4 outColor;

void main()
{
    vec3 normal = normalize(vecNormal);

    vec3 normalFromMap = normalize(texture2D(sunNormal, vecTex).xyz * 2.0 - 1.0);

    float diffuse = max(0, dot(normalFromMap, lightDir)); // Notice the negative sign
    diffuse = pow(diffuse, DIFFUSE_INTENSITY); // Apply intensity control

    vec4 albedoColor = texture2D(sunAlbedo, vecTex);
    vec3 finalColor = albedoColor.rgb * min(1,diffuse + AMBIENT);

    outColor = vec4(finalColor, 1.0);
}
