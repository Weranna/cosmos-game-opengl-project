#pragma once
 
#include "glew.h"
#include "freeglut.h"
#include <ext.hpp>
#include <iostream>

namespace Core
{
	GLuint LoadTexture(const char * filepath);
	void SetActiveTexture(GLuint textureID, const char * shaderVariableName, GLuint programID, int textureUnit);
	GLuint LoadSkybox(const std::string filepaths[6]);
}