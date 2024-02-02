#pragma once
 
#include "glew.h"
#include "glm.hpp"
#include <iostream>
#include <map>
#include <vector>

struct Contexts {
    Core::RenderContext shipContext;
    Core::RenderContext sphereContext;
    Core::RenderContext trash1Context;
    Core::RenderContext trash2Context;
    Core::RenderContext asteroidContext;
    Core::RenderContext laserContext;
    Core::RenderContext skyboxContext;
    Core::RenderContext barierContext;
};

struct TextureSet {
    GLuint albedo;
    GLuint normal;
    GLuint ao;
    GLuint roughness;
    GLuint metallic;
};

struct PlanetTextures {
    TextureSet mercury;
    TextureSet venus;
    TextureSet earth;
    TextureSet mars;
    TextureSet jupiter;
    TextureSet saturn;
    TextureSet uran;
    TextureSet neptune;
};

struct Textures {
    TextureSet sun;
    TextureSet spaceship;
    PlanetTextures planets;
    TextureSet trash1;
    TextureSet trash2;
    TextureSet asteroid;
    TextureSet laser;
    TextureSet barier;
};

struct ObjectInfo {
    glm::vec3 coordinates;
    float orbit;
    bool destroyed = false;
};

struct Planets {
    std::map<std::string, ObjectInfo> planetsProperties;
    std::map<std::string, std::vector<ObjectInfo>> trashProperties;
};

struct Laser {
    glm::vec3 position;
    glm::vec3 direction;
    float startTime;
    const float duration = 0.5f;
    bool isActive;

    Laser() : position(0.0f), direction(0.0f), startTime(0.0f), isActive(false) {}
};

struct TextureSprite {
    GLuint sprite_1;
    GLuint sprite_2;
    GLuint sprite_3;
    GLuint sprite_4;
    GLuint sprite_5;
    GLuint sprite_6;
    GLuint sprite_7;
    GLuint sprite_8;
    GLuint sprite_end;
    GLuint sprite_start;
};