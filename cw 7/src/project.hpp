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

// teraz o wiele wiêkszy porz¹dek ze zmiennymi
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
};

Textures textures;

GLuint programDefault;
GLuint programSun;

Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;
Core::RenderContext trash1Context;
Core::RenderContext trash2Context;
Core::RenderContext asteroidContext;

glm::vec3 cameraPos = glm::vec3(20.f, 0, 0);
glm::vec3 cameraDir = glm::vec3(-1.f, 0.f, 0.f);

glm::vec3 spaceshipPos = glm::vec3(20.f, 0, 0);
glm::vec3 spaceshipDir = glm::vec3(-1.f, 0.f, 0.f);

GLuint VAO, VBO;

float aspectRatio = 1.f;

float exposition = 1.f;

glm::vec3 lightPos = glm::vec3(0, 0.f, 0);
glm::vec3 lightColor = glm::vec3(0.9, 0.7, 0.8) * 100;

double lastMouseX = 0.0;
double lastMouseY = 0.0;

float cameraSide = 0.0f;
float cameraUp = 0.0f;

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


void drawShip(GLuint program, Core::RenderContext& context, glm::mat4 modelMatrix, TextureSet textures) {

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform1f(glGetUniformLocation(program, "exposition"), exposition);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(program, "lightPos"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(program, "lightColor"), 1.0f, 1.0f, 1.0f);
	Core::SetActiveTexture(textures.albedo, "albedoTexture", program, 0);
	Core::SetActiveTexture(textures.normal, "normalTexture", program, 1);
	Core::SetActiveTexture(textures.ao, "aoTexture", program, 2);
	Core::SetActiveTexture(textures.roughness, "roughnessTexture", program, 3);
	Core::SetActiveTexture(textures.metallic, "metallicTexture", program, 4);

	Core::DrawContext(context);

}

void drawObjectTexture(GLuint program,Core::RenderContext& context, TextureSet textures, glm::mat4 modelMatrix) {

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(programDefault, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programDefault, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform1f(glGetUniformLocation(programDefault, "exposition"), exposition);
	glUniform3f(glGetUniformLocation(programDefault, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(programDefault, "lightPos"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(programDefault, "lightColor"), 1.0f, 1.0f, 1.0f);
	Core::SetActiveTexture(textures.albedo, "albedoTexture", program, 0);
	Core::SetActiveTexture(textures.normal, "normalTexture", program, 1);
	Core::SetActiveTexture(textures.ao, "aoTexture", program, 2);
	Core::SetActiveTexture(textures.roughness, "roughnessTexture", program, 3);
	Core::SetActiveTexture(textures.metallic, "metallicTexture", program, 4);
	Core::DrawContext(context);

}

void drawTrash(float planetX, float planetZ, float time, float orbitRadius, glm::vec3 scalePlanet) {
	float orbitSpeed = 1.5f;

	int numberOfTrash = ceil(scalePlanet.r);
	for (int i = 1; i <= numberOfTrash; ++i) {
		
		float trashX1 = planetX + orbitRadius * cos(orbitSpeed * time + i * 100);
		float trashZ1 = planetZ + orbitRadius * sin(orbitSpeed * time + i * 50);

		float trashX2 = planetX + orbitRadius * cos(orbitSpeed * time - i * 50);
		float trashZ2 = planetZ + orbitRadius * sin(orbitSpeed * time - i * 100);

		glm::mat4 modelMatrix1 = glm::translate(glm::vec3(trashX1, 0.5f, trashZ1)) *
			glm::rotate(2.f * time, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(0.5f * time, glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(scalePlanet / 10.0f);

		glm::mat4 modelMatrix2 = glm::translate(glm::vec3(trashX2, -0.5f, trashZ2)) *
			glm::rotate(2.f * time, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(0.5f * time, glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(scalePlanet/ 2.f);

		drawObjectTexture(programDefault,trash1Context, textures.trash1, modelMatrix1);
		drawObjectTexture(programDefault,trash2Context, textures.trash2, modelMatrix2);
	}
}



void drawPlanet(Core::RenderContext& context, TextureSet textures, float planetOrbitRadius, float planetOrbitSpeed, float time, glm::vec3 scalePlanet, float trashOrbitRadius) {
	float planetX = planetOrbitRadius * cos(planetOrbitSpeed * time);
	float planetZ = planetOrbitRadius * sin(planetOrbitSpeed * time);
	glm::mat4 modelMatrix = glm::translate(glm::vec3(planetX, 0, planetZ)) * glm::scale(scalePlanet);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(programDefault, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programDefault, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform1f(glGetUniformLocation(programDefault, "exposition"), exposition);
	glUniform3f(glGetUniformLocation(programDefault, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(programDefault, "lightPos"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(programDefault, "lightColor"), 1.0f, 1.0f, 1.0f);
	Core::SetActiveTexture(textures.albedo, "albedoTexture", programDefault, 0);
	Core::SetActiveTexture(textures.normal, "normalTexture", programDefault, 1);
	Core::SetActiveTexture(textures.ao, "aoTexture", programDefault, 2);
	Core::SetActiveTexture(textures.roughness, "roughnessTexture", programDefault, 3);
	Core::SetActiveTexture(textures.metallic, "metallicTexture", programDefault, 4);
	Core::DrawContext(context);

	// KSIÊ¯YC
	//drawMoon(context, glm::vec3(0.8, 0.8, 0.8), planetX, planetZ, time, moonOrbitRadius);

	// THRASH
	drawTrash(planetX, planetZ, time, trashOrbitRadius,scalePlanet);
}

void drawSun(Core::RenderContext& context, glm::mat4 modelMatrix,TextureSet textures) {

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programSun, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programSun, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(programSun, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(programSun, "lightPos"), 0.0f, 0.0f, 0.0f);
	Core::SetActiveTexture(textures.albedo, "sunAlbedo", programSun, 0);
	Core::SetActiveTexture(textures.normal, "sunNormal", programSun, 1);
	Core::DrawContext(context);

}

//void drawMoon(Core::RenderContext& context, glm::vec3 color, float planetX, float planetZ, float time, float moonOrbitRadius) {
//	float moonOrbitSpeed = 1.5f;
//	float moonX = planetX + moonOrbitRadius * cos(moonOrbitSpeed * time);
//	float moonZ = planetZ + moonOrbitRadius * sin(moonOrbitSpeed * time);
//	glm::mat4 modelMatrix = glm::translate(glm::vec3(moonX, 0, moonZ)) * glm::scale(glm::vec3(0.3, 0.3, 0.3));
//	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
//	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
//	glUniform3f(glGetUniformLocation(programDefault, "color"), color.x, color.y, color.z);
//	glUniformMatrix4fv(glGetUniformLocation(programDefault, "transformation"), 1, GL_FALSE, (float*)&transformation);
//	Core::DrawContext(context);
//}

void renderScene(GLFWwindow* window)
{
	//GRANATOWE T£O
	glClearColor(0.0f, 0.0f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 transformation;
	float time = glfwGetTime();
	updateDeltaTime(time);

	glUseProgram(programSun);

	// S£OÑCE
	drawSun(sphereContext, glm::scale(glm::vec3(10.f)) * glm::translate(glm::vec3(0, 0, 0)), textures.sun);

	glUseProgram(programDefault);

	// UK£AD S£ONECZNY - PLANETY (NA RAZIE BEZ KSIÊ¯YCA)
	//MERKURY
	drawPlanet(sphereContext, textures.planets.mercury, 15.0f, 0.4f, time, glm::vec3(0.5),1);
	//WENUS
	drawPlanet(sphereContext, textures.planets.venus, 20.0f, 0.35f, time, glm::vec3(1.f),1.5);
	//ZIEMIA
	drawPlanet(sphereContext, textures.planets.earth, 25.0f, 0.3f, time, glm::vec3(1.3f),2);
	//MARS
	drawPlanet(sphereContext, textures.planets.mars, 30.0f, 0.25f, time, glm::vec3(1.3f), 2);
	//JOWISZ
	drawPlanet(sphereContext, textures.planets.jupiter, 40.0f, 0.2f, time, glm::vec3(2.5f), 3);
	//SATURN - bez pierœcieni (mo¿e dorobiæ póŸniej spróbowaæ?)
	drawPlanet(sphereContext, textures.planets.saturn, 50.0f, 0.15f, time, glm::vec3(2.2f), 3);
	//URAN - tu te¿ teoretycznie pierscienie by mozna dodac
	drawPlanet(sphereContext, textures.planets.uran, 55.0f, 0.1f, time, glm::vec3(1.6f), 2.5);
	//NEPTUN
	drawPlanet(sphereContext, textures.planets.neptune, 60.0f, 0.05f, time, glm::vec3(1.8f), 2.5);

	glm::vec3 initialAsteroidPosition(0.f, -15.f, 0.f);
	float spacing = 5.f;
	float asteroidXOffset = sin(time) * 2.0f;

	// W pêtli
	for (int row = 0; row < 3; ++row)
	{
		for (int col = 0; col < 8; ++col)
		{
			glm::vec3 position = initialAsteroidPosition + glm::vec3(col * spacing, 0.f, row * spacing);
			if (row % 2 == 1)
			{
				position.x += spacing * 0.5f;
			}
			position.x += asteroidXOffset;
			transformation = glm::translate(glm::mat4(1.0f), position) *
				glm::rotate(2.f * time, glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(0.5f * time, glm::vec3(1.0f, 0.0f, 0.0f));
			drawObjectTexture(programDefault,asteroidContext, textures.asteroid, transformation);
		}
	}

	//STATEK
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});
	drawShip(programDefault, shipContext, glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.0004)), textures.spaceship);

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
		throw std::runtime_error("ERROR::ASSIMP::" + std::string(import.GetErrorString()));
	}

	if (scene->mNumMeshes == 0)
	{
		throw std::runtime_error("ERROR::ASSIMP::No meshes found in the model.");
	}

	context.initFromAssimpMesh(scene->mMeshes[0]);
}

// ³aduje tekstury
TextureSet loadTextureSet(const std::string& albedoPath, const std::string& normalPath, const std::string& aoPath, const std::string& roughnessPath, const std::string& metallicPath) {
	TextureSet textureSet;
	textureSet.albedo = Core::LoadTexture(albedoPath.c_str());
	textureSet.normal = Core::LoadTexture(normalPath.c_str());
	textureSet.ao = Core::LoadTexture(aoPath.c_str());
	textureSet.roughness = Core::LoadTexture(roughnessPath.c_str());
	textureSet.metallic = Core::LoadTexture(metallicPath.c_str());
	return textureSet;
}

// funkcja do tekstur
void initTextures() {
	textures.sun.albedo = Core::LoadTexture("./textures/sun/sun_albedo.png");
	textures.sun.normal = Core::LoadTexture("./textures/sun/sun_normal.png");

	textures.spaceship.albedo = Core::LoadTexture("./textures/spaceship/spaceship_albedo.jpg");
	textures.spaceship.normal = Core::LoadTexture("./textures/spaceship/spaceship_normal.jpg");
	textures.spaceship.ao = Core::LoadTexture("./textures/spaceship/spaceship_ao.jpg");
	textures.spaceship.roughness = Core::LoadTexture("./textures/spaceship/spaceship_roughness.jpg");
	textures.spaceship.metallic = Core::LoadTexture("./textures/spaceship/spaceship_metallic.jpg");

	textures.planets.mercury = loadTextureSet("./textures/planets/custom/planet1_albedo.png", "./textures/planets/custom/planet1_normal.png", "./textures/planets/custom/planet1_ao.png", "./textures/planets/custom/planet1_roughness.png", "./textures/planets/custom/planet1_metallic.png");
	textures.planets.venus = loadTextureSet("./textures/planets/custom/planet2_albedo.png", "./textures/planets/custom/planet2_normal.png", "./textures/planets/custom/planet2_ao.png", "./textures/planets/custom/planet2_roughness.png", "./textures/planets/custom/planet2_metallic.png");
	textures.planets.earth = loadTextureSet("./textures/planets/earth/earth_albedo.jpg", "./textures/planets/earth/earth_normal.jpg", "./textures/planets/earth/earth_ao.png", "./textures/planets/earth/earth_roughness.jpg", "./textures/planets/earth/earth_metallic.png");
	textures.planets.mars = loadTextureSet("./textures/planets/mars/mars_albedo.jpg", "./textures/planets/mars/mars_normal.png", "./textures/planets/mars/mars_ao.jpg", "./textures/planets/mars/mars_roughness.jpg", "./textures/planets/mars/mars_metallic.png");
	textures.planets.jupiter = loadTextureSet("./textures/planets/jupiter/jupiter_albedo.jpg", "./textures/planets/jupiter/jupiter_normal.png", "./textures/planets/jupiter/jupiter_ao.jpg", "./textures/planets/jupiter/jupiter_roughness.jpg", "./textures/planets/jupiter/jupiter_metallic.png");
	textures.planets.saturn = loadTextureSet("./textures/planets/custom/planet3_albedo.png", "./textures/planets/custom/planet3_normal.png", "./textures/planets/custom/planet3_ao.png", "./textures/planets/custom/planet3_roughness.png", "./textures/planets/custom/planet3_metallic.png");
	textures.planets.uran = loadTextureSet("./textures/planets/custom/planet5_albedo.jpg", "./textures/planets/custom/planet5_normal.png", "./textures/planets/custom/planet5_ao.jpg", "./textures/planets/custom/planet5_roughness.jpg", "./textures/planets/custom/planet5_metallic.png");
	textures.planets.neptune = loadTextureSet("./textures/planets/neptune/neptune_albedo.jpg", "./textures/planets/neptune/neptune_normal.png", "./textures/planets/neptune/neptune_ao.jpg", "./textures/planets/neptune/neptune_roughness.jpg", "./textures/planets/neptune/neptune_metallic.png");
	textures.trash1 = loadTextureSet("./textures/trash/trash1_albedo.jpg", "./textures/trash/trash1_normal.png", "./textures/trash/trash1_AO.jpg", "./textures/trash/trash1_roughness.jpg", "./textures/trash/trash1_metallic.jpg");
	textures.trash2 = loadTextureSet("./textures/trash/trash2_albedo.jpg", "./textures/trash/trash2_normal.png", "./textures/trash/trash2_AO.jpg", "./textures/trash/trash2_roughness.jpg", "./textures/trash/trash2_metallic.jpg");
	textures.asteroid = loadTextureSet("./textures/asteroid/asteroid_albedo.png", "./textures/asteroid/asteroid_normal.png", "./textures/planets/mars/mars_ao.jpg", "./textures/asteroid/asteroid_roughness.png", "./textures/asteroid/asteroid_metallic.png");
}

void init(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetCursorPos(window, &lastMouseX, &lastMouseY);

	glEnable(GL_DEPTH_TEST);

	programDefault = shaderLoader.CreateProgram("shaders/shader_default.vert", "shaders/shader_default.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_sun.vert", "shaders/shader_sun.frag");

	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/spaceship.fbx", shipContext);
	loadModelToContext("./models/trash1.dae", trash1Context);
	loadModelToContext("./models/trash2.dae", trash2Context);
	loadModelToContext("./models/asteroid.obj", asteroidContext);

	initTextures();
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
	float angleSpeed = 0.05f * deltaTime * 10;
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

	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	double mouseMoveX = mouseX - lastMouseX;
	double mouseMoveY = mouseY - lastMouseY;
	float mouseSensitivity = 0.1f;

	cameraSide += static_cast<float>(mouseMoveX) * mouseSensitivity;
	cameraUp -= static_cast<float>(mouseMoveY) * mouseSensitivity;

	cameraUp = glm::clamp(cameraUp, -89.0f, 89.0f);

	lastMouseX = mouseX;
	lastMouseY = mouseY;

	glm::vec3 front;
	front.x = cos(glm::radians(cameraSide)) * cos(glm::radians(cameraUp));
	front.y = sin(glm::radians(cameraUp));
	front.z = sin(glm::radians(cameraSide)) * cos(glm::radians(cameraUp));
	cameraDir = glm::normalize(front);

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