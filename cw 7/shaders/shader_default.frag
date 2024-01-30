 #version 430 core

float AMBIENT = 0.08;
float PI = 3.14;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D aoTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D metallicTexture;

uniform vec3 cameraPos;

uniform vec3 lightPos;
uniform vec3 lightColor;

uniform float exposition;

in vec3 vecNormal;
in vec3 worldPos;
in vec2 vecTex;

out vec4 outColor;

in vec3 viewDirTS;
in vec3 lightDirTS;
in vec3 spotlightDirTS;
in vec3 sunDirTS;

in vec3 test;

float DistributionGGX(vec3 normal, vec3 H, float roughness){
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(normal, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness){
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 normal, vec3 V, vec3 lightDir, float roughness){
    float NdotV = max(dot(normal, V), 0.0);
    float NdotL = max(dot(normal, lightDir), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0){
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 PBRLight(vec3 lightDir, vec3 radiance, vec3 normal, vec3 V){
    float diffuse = max(0, dot(normal, lightDir));

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, texture(albedoTexture, vecTex).rgb, texture(metallicTexture, vecTex).r);

    vec3 H = normalize(V + lightDir);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, H, texture(roughnessTexture, vecTex).r);
    float G   = GeometrySmith(normal, V, lightDir, texture(roughnessTexture, vecTex).r);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - texture(metallicTexture, vecTex).r;

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    // Add to outgoing radiance Lo
    float NdotL = max(dot(normal, lightDir), 0.0);
    return (kD * texture(albedoTexture, vecTex).rgb / PI + specular) * radiance * NdotL;
}

void main(){
    vec3 normal = normalize(texture(normalTexture, vecTex).xyz * 2.0 - 1.0);

    vec3 lightDir = normalize(lightDirTS);
    vec3 viewDir = normalize(viewDirTS);

    vec3 ambient = AMBIENT * texture(albedoTexture, vecTex).rgb * texture(aoTexture, vecTex).r;
    //vec3 ambient = AMBIENT * texture(albedoTexture, vecTex).rgb;
    vec3 attenuatedlightColor = lightColor / pow(length(lightPos - worldPos), 2);
    vec3 ilumination;
    ilumination = ambient + PBRLight(lightDir, attenuatedlightColor, normal, viewDir);

    // Sun
    ilumination = ilumination + PBRLight(lightDir, 10*lightColor, normal, viewDir);

    outColor = vec4(vec3(1.0) - exp(-ilumination * exposition), 1);
}
