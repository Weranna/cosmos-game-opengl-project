#pragma once
#include "Render_Utils.h"
#include "Texture.h"


namespace Core {
    class RenderSprite
    {
    public:
        RenderSprite();
        ~RenderSprite();
        void DrawSprite(GLuint program, float width, float height);
        void UpdateSprite(GLuint newTextureID);

    private:

        unsigned int VAO;
        unsigned int VBO;
        GLuint currentTextureID;
        void initSprite();
    };
}