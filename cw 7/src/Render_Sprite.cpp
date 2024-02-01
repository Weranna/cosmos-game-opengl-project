#include <iostream>
#include "Render_Sprite.h"
#include <ext.hpp>

Core::RenderSprite::RenderSprite() {
    this->initSprite();
}

Core::RenderSprite::~RenderSprite() {
    glDeleteVertexArrays(1, &this->VAO);
    glDeleteBuffers(1, &this->VBO);
}

void Core::RenderSprite::DrawSprite(GLuint program, float width, float height) {
    glm::mat4 projection = glm::ortho(0.0f, 1000.0f, 1000.0f, 0.0f, -1.0f, 1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(500.0f - width / 2.0f, 500.0f - height / 2.0f, 0.0f));
    model = glm::scale(model, glm::vec3(glm::vec2(width, height), 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, (float*)&projection);
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, (float*)&model);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->currentTextureID);
    GLuint textureLocation = glGetUniformLocation(program, "spriteTexture");
    glUniform1i(textureLocation, 0);

    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Core::RenderSprite::initSprite() {
    float vertices[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);

    glBindVertexArray(this->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(this->VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Core::RenderSprite::UpdateSprite(GLuint newTextureID) {
    this->currentTextureID = newTextureID;
}