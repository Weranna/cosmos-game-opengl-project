#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform sampler2D spriteTexture;

void main()
{    
    vec4 texColor = texture(spriteTexture, TexCoords);
    
    if (texColor.a < 0.1)
        discard;
    
    FragColor = texColor;

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 10.0f)
        BrightColor = vec4(FragColor.rgb, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
