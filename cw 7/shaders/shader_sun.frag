// shader_sun.frag
#version 430 core
layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 BloomColor;


float AMBIENT = 0.3; // Adjust the ambient lighting
float DIFFUSE_INTENSITY = 2.0; // Adjust the diffuse intensity

uniform vec3 lightDir; // Directional light direction
uniform sampler2D sunAlbedo;
uniform sampler2D sunNormal;

in vec3 vecNormal;
in vec3 fragPos; // Position in world space
in vec2 vecTex;


void main()
{
    vec3 normal = normalize(vecNormal);

    vec3 normalFromMap = normalize(texture2D(sunNormal, vecTex).xyz * 2.0 - 1.0);

    float diffuse = max(0, dot(normalFromMap, lightDir)); // Notice the negative sign
    diffuse = pow(diffuse, DIFFUSE_INTENSITY); // Apply intensity control

    vec4 albedoColor = texture2D(sunAlbedo, vecTex);
    vec3 finalColor = albedoColor.rgb * min(1,diffuse + AMBIENT);
    outColor = vec4(finalColor, 1.0);

    if (outColor.r > 0.05f)
		outColor.r *= 5.0f;

	// Calculate brightness by adding up all the channels with different weights each
	float brightness = dot(outColor.rgb, vec3(0.2126f, 0.7152f, 0.0722f));
    if(brightness > 0.15f)
        BloomColor = vec4(outColor.rgb, 1.0f);
    else
        BloomColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    


}
