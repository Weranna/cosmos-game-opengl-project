#version 430 core

out vec4 fragColor;
uniform vec3 color;
uniform float n, f;

//obiekty zanikaj¹ jak siê odleci
float count_z(float n, float f, float z_p) {
    z_p = z_p * 2.0 - 1.0;
    float z;
    z = -2.0 * n * f / (z_p * (f-n) - n - f);
    return (z-n)/(f-n);
}

void main()
{
    vec3 backgroundColor = vec3(0.0f, 0.3f, 0.3f);
    float z;
    z = count_z(n,f, gl_FragCoord.z);

    vec3 finalColor = mix(color, backgroundColor, z);

    fragColor = vec4(finalColor, 1.0);
   
}

