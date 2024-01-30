#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D spriteTexture;

void main()
{    
    vec4 texColor = texture(spriteTexture, TexCoords);
    
    if (texColor.a < 0.1)
        discard;
    
    FragColor = texColor;
}
