#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"

#include "Box.cpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>


GLuint programDefault;

Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;

glm::vec3 cameraPos = glm::vec3(20.f, 0, 0);
glm::vec3 cameraDir = glm::vec3(-1.f, 0.f, 0.f);

glm::vec3 spaceshipPos = glm::vec3(20.f, 0, 0);
glm::vec3 spaceshipDir = glm::vec3(-1.f, 0.f, 0.f);

GLuint VAO, VBO;

float aspectRatio = 1.f;

float exposition = 1.f;

glm::vec3 lightPos = glm::vec3(0, 0.f, 0);
glm::vec3 lightColor = glm::vec3(0.9, 0.7, 0.8) * 100;


float lastTime = -1.f;
float deltaTime = 0.f;

void updateDeltaTime(float time) {
	if (lastTime < 0) {
		lastTime = time;
		return;
	}

	deltaTime = time - lastTime;
	if (deltaTime > 0.1) deltaTime = 0.1;
	lastTime = time;
}
glm::mat4 createCameraMatrix()
{
	glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraSide, cameraDir));
	glm::mat4 cameraRotrationMatrix = glm::mat4({
		cameraSide.x,cameraSide.y,cameraSide.z,0,
		cameraUp.x,cameraUp.y,cameraUp.z ,0,
		-cameraDir.x,-cameraDir.y,-cameraDir.z,0,
		0.,0.,0.,1.,
		});
	cameraRotrationMatrix = glm::transpose(cameraRotrationMatrix);
	glm::mat4 cameraMatrix = cameraRotrationMatrix * glm::translate(-cameraPos);

	return cameraMatrix;
}

glm::mat4 createPerspectiveMatrix()
{

	glm::mat4 perspectiveMatrix;
	float n = 0.05;
	float f = 20.;
	float a1 = glm::min(aspectRatio, 1.f);
	float a2 = glm::min(1 / aspectRatio, 1.f);
	perspectiveMatrix = glm::mat4({
		1,0.,0.,0.,
		0.,aspectRatio,0.,0.,
		0.,0.,(f + n) / (n - f),2 * f * n / (n - f),
		0.,0.,-1.,0.,
		});


	perspectiveMatrix = glm::transpose(perspectiveMatrix);

	return perspectiveMatrix;
}

void drawObjectColor(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color) {

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programDefault, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform3f(glGetUniformLocation(programDefault, "color"), color.x, color.y, color.z);
	Core::DrawContext(context);
}

void drawMoon(Core::RenderContext& context, glm::vec3 color, float planetX, float planetZ, float time, float moonOrbitRadius) {
	float moonOrbitSpeed = 1.5f;
	float moonX = planetX + moonOrbitRadius * cos(moonOrbitSpeed * time);
	float moonZ = planetZ + moonOrbitRadius * sin(moonOrbitSpeed * time);
	glm::mat4 modelMatrix = glm::translate(glm::vec3(moonX, 0, moonZ)) * glm::scale(glm::vec3(0.3, 0.3, 0.3));
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniform3f(glGetUniformLocation(programDefault, "color"), color.x, color.y, color.z);
	glUniformMatrix4fv(glGetUniformLocation(programDefault, "transformation"), 1, GL_FALSE, (float*)&transformation);
	Core::DrawContext(context);
}

void drawPlanetWithMoon(Core::RenderContext& context, glm::vec3 color, float planetOrbitRadius, float planetOrbitSpeed, float time, glm::vec3 scalePlanet, float moonOrbitRadius) {
	float planetX = planetOrbitRadius * cos(planetOrbitSpeed * time);
	float planetZ = planetOrbitRadius * sin(planetOrbitSpeed * time);
	glm::mat4 modelMatrix = glm::translate(glm::vec3(planetX, 0, planetZ)) * glm::scale(scalePlanet);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniform3f(glGetUniformLocation(programDefault, "color"), color.x, color.y, color.z);
	glUniformMatrix4fv(glGetUniformLocation(programDefault, "transformation"), 1, GL_FALSE, (float*)&transformation);
	Core::DrawContext(context);
	// KSIÊ¯YC
	drawMoon(context, glm::vec3(0.8, 0.8, 0.8), planetX, planetZ, time, moonOrbitRadius);
}

void renderScene(GLFWwindow* window)
{
	//GRANATOWE T£O
	glClearColor(0.0f, 0.0f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 transformation;
	float time = glfwGetTime();
	updateDeltaTime(time);
	glUseProgram(programDefault);

	// S£OÑCE
	drawObjectColor(sphereContext, glm::scale(glm::vec3(10.f)) * glm::translate(glm::vec3(0, 0, 0)), glm::vec3(0.9, 0.8, 0.4));

	// UK£AD S£ONECZNY - PLANETY Z KSIÊ¯YCEM
	//MERKURY
	drawPlanetWithMoon(sphereContext, glm::vec3(0.5f, 0.5f, 0.5f), 15.0f, 0.4f, time, glm::vec3(0.5),1);
	//WENUS
	drawPlanetWithMoon(sphereContext, glm::vec3(0.9f, 0.7f, 0.2f), 20.0f, 0.35f, time, glm::vec3(1.f),1.5);
	//ZIEMIA
	drawPlanetWithMoon(sphereContext, glm::vec3(0.0f, 1.0f, 0.f), 25.0f, 0.3f, time, glm::vec3(1.3f),2);
	//MARS
	drawPlanetWithMoon(sphereContext, glm::vec3(1.0f, 0.0f, 0.f), 30.0f, 0.25f, time, glm::vec3(1.3f), 2);
	//JOWISZ
	drawPlanetWithMoon(sphereContext, glm::vec3(0.9f, 0.7f, 0.5f), 40.0f, 0.2f, time, glm::vec3(2.5f), 3);
	//SATURN - bez pierœcieni (mo¿e dorobiæ póŸniej spróbowaæ?)
	drawPlanetWithMoon(sphereContext, glm::vec3(0.7f, 0.7f, 0.5f), 50.0f, 0.15f, time, glm::vec3(2.2f), 3);
	//URAN - tu te¿ teoretycznie pierscienie by mozna dodac
	drawPlanetWithMoon(sphereContext, glm::vec3(0.0f, 0.0f, 0.6f), 55.0f, 0.1f, time, glm::vec3(1.6f), 2.5);
	//NEPTUN
	drawPlanetWithMoon(sphereContext, glm::vec3(0.0f, 0.0f, 0.4f), 60.0f, 0.05f, time, glm::vec3(1.8f), 2.5);

	//STATEK
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});
	drawObjectColor(shipContext, glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.0004)), glm::vec3(0.8, 0.8, 0.8));

	glUseProgram(0);
	glfwSwapBuffers(window);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
}
void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}

void init(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);

	programDefault = shaderLoader.CreateProgram("shaders/shader_default.vert", "shaders/shader_default.frag");

	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/spaceship.fbx", shipContext);
}

void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(programDefault);
}

//obsluga wejscia
void processInput(GLFWwindow* window)
{
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::vec3(0.f, 1.f, 0.f);
	float angleSpeed = 0.05f * deltaTime * 60;
	float moveSpeed = 0.3f * deltaTime * 60;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		spaceshipPos += spaceshipDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		spaceshipPos -= spaceshipDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		spaceshipPos += spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		spaceshipPos -= spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		spaceshipPos += spaceshipUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		spaceshipPos -= spaceshipUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		spaceshipDir = glm::vec3(glm::eulerAngleY(angleSpeed) * glm::vec4(spaceshipDir, 0));
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		spaceshipDir = glm::vec3(glm::eulerAngleY(-angleSpeed) * glm::vec4(spaceshipDir, 0));

	cameraPos = spaceshipPos - 1.5 * spaceshipDir + glm::vec3(0, 1, 0) * 0.5f;
	cameraDir = spaceshipDir;

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		exposition -= 0.05;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		exposition += 0.05;

	//cameraDir = glm::normalize(-cameraPos);

}

// funkcja jest glowna petla
void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		renderScene(window);
		glfwPollEvents();
	}
}
//}