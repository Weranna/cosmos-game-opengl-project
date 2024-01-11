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

namespace texture {

	GLuint sun[2];
	GLuint spaceship[5];		// PRZYKLAD [0] - ALBEDO, [1] - NORMAL ... ZEBY BYLO MNIEJ ZMIENNYCH

	GLuint mercury[5];
	GLuint venus[5];
	GLuint earth[5];
	GLuint mars[5];
	GLuint jupiter[5];
	GLuint saturn[5];
	GLuint uran[5];
	GLuint neptune[5];

	GLuint trash1[5];
	GLuint trash2[5];
}

GLuint programDefault;
GLuint programSun;

Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;
Core::RenderContext trash1Context;
Core::RenderContext trash2Context;

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


void drawShip(GLuint program, Core::RenderContext& context, glm::mat4 modelMatrix, GLuint texture_ID[5]) {

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform1f(glGetUniformLocation(program, "exposition"), exposition);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(program, "lightPos"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(program, "lightColor"), 1.0f, 1.0f, 1.0f);
	Core::SetActiveTexture(texture_ID[0], "albedoTexture", program, 0);
	Core::SetActiveTexture(texture_ID[1], "normalTexture", program, 1);
	Core::SetActiveTexture(texture_ID[2], "aoTexture", program, 2);
	Core::SetActiveTexture(texture_ID[3], "roughnessTexture", program, 3);
	Core::SetActiveTexture(texture_ID[4], "metallicTexture", program, 4);

	Core::DrawContext(context);

}

void drawObjectTexture(Core::RenderContext& context, GLuint texture_ID[5], glm::mat4 modelMatrix) {

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(programDefault, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programDefault, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform1f(glGetUniformLocation(programDefault, "exposition"), exposition);
	glUniform3f(glGetUniformLocation(programDefault, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(programDefault, "lightPos"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(programDefault, "lightColor"), 1.0f, 1.0f, 1.0f);
	Core::SetActiveTexture(texture_ID[0], "albedoTexture", programDefault, 0);
	Core::SetActiveTexture(texture_ID[1], "normalTexture", programDefault, 1);
	Core::SetActiveTexture(texture_ID[2], "aoTexture", programDefault, 2);
	Core::SetActiveTexture(texture_ID[3], "roughnessTexture", programDefault, 3);
	Core::SetActiveTexture(texture_ID[4], "metallicTexture", programDefault, 4);
	Core::DrawContext(context);

}

void drawTrash(float planetX, float planetZ, float time, float orbitRadius, glm::vec3 scalePlanet) {
	float orbitSpeed = 1.5f;
	float rotationSpeedY = 2.0f;
	float rotationSpeedX = 0.5f;

	int numberOfTrash = ceil(scalePlanet.r);

	for (int i = 1; i <= numberOfTrash; ++i) {
		
		float trashX1 = planetX + orbitRadius * cos(orbitSpeed * time + i * 100);
		float trashZ1 = planetZ + orbitRadius * sin(orbitSpeed * time + i * 50);

		float trashX2 = planetX + orbitRadius * cos(orbitSpeed * time - i * 50);
		float trashZ2 = planetZ + orbitRadius * sin(orbitSpeed * time - i * 100);

		glm::mat4 modelMatrix1 = glm::translate(glm::vec3(trashX1, 0.5f, trashZ1)) *
			glm::rotate(rotationSpeedY * time, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(rotationSpeedX * time, glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(scalePlanet / 10.0f);

		glm::mat4 modelMatrix2 = glm::translate(glm::vec3(trashX2, -0.5f, trashZ2)) *
			glm::rotate(rotationSpeedY * time, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(rotationSpeedX * time, glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(scalePlanet/ 2.f);

		// Rysuj obiekty z zastosowanymi macierzami modelu
		drawObjectTexture(trash1Context, texture::trash1, modelMatrix1);
		drawObjectTexture(trash2Context, texture::trash2, modelMatrix2);
	}
}



void drawPlanet(Core::RenderContext& context, GLuint texture_ID[5], float planetOrbitRadius, float planetOrbitSpeed, float time, glm::vec3 scalePlanet, float orbitRadius) {
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
	Core::SetActiveTexture(texture_ID[0], "albedoTexture", programDefault, 0);
	Core::SetActiveTexture(texture_ID[1], "normalTexture", programDefault, 1);
	Core::SetActiveTexture(texture_ID[2], "aoTexture", programDefault, 2);
	Core::SetActiveTexture(texture_ID[3], "roughnessTexture", programDefault, 3);
	Core::SetActiveTexture(texture_ID[4], "metallicTexture", programDefault, 4);
	Core::DrawContext(context);

	// KSIÊ¯YC
	//drawMoon(context, glm::vec3(0.8, 0.8, 0.8), planetX, planetZ, time, moonOrbitRadius);

	// THRASH
	drawTrash(planetX, planetZ, time, orbitRadius,scalePlanet);
}

void drawSun(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint texture1_ID, GLuint texture2_ID) {

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programSun, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programSun, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(programSun, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(programSun, "lightPos"), 0.0f, 0.0f, 0.0f);
	Core::SetActiveTexture(texture1_ID, "sunAlbedo", programSun, 0);
	Core::SetActiveTexture(texture2_ID, "sunNormal", programSun, 1);

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
	drawSun(sphereContext, glm::scale(glm::vec3(10.f)) * glm::translate(glm::vec3(0, 0, 0)), texture::sun[0], texture::sun[1]);

	glUseProgram(0);
	glUseProgram(programDefault);

	// UK£AD S£ONECZNY - PLANETY (NA RAZIE BEZ KSIÊ¯YCA)
	//MERKURY
	drawPlanet(sphereContext, texture::mercury, 15.0f, 0.4f, time, glm::vec3(0.5),1);
	//WENUS
	drawPlanet(sphereContext, texture::venus, 20.0f, 0.35f, time, glm::vec3(1.f),1.5);
	//ZIEMIA
	drawPlanet(sphereContext, texture::earth, 25.0f, 0.3f, time, glm::vec3(1.3f),2);
	//MARS
	drawPlanet(sphereContext, texture::mars, 30.0f, 0.25f, time, glm::vec3(1.3f), 2);
	//JOWISZ
	drawPlanet(sphereContext, texture::jupiter, 40.0f, 0.2f, time, glm::vec3(2.5f), 3);
	//SATURN - bez pierœcieni (mo¿e dorobiæ póŸniej spróbowaæ?)
	drawPlanet(sphereContext, texture::saturn, 50.0f, 0.15f, time, glm::vec3(2.2f), 3);
	//URAN - tu te¿ teoretycznie pierscienie by mozna dodac
	drawPlanet(sphereContext, texture::uran, 55.0f, 0.1f, time, glm::vec3(1.6f), 2.5);
	//NEPTUN
	drawPlanet(sphereContext, texture::neptune, 60.0f, 0.05f, time, glm::vec3(1.8f), 2.5);
	
	//DODATKOWE PLANETY S¥ W TEKSTURACH, W RAZIE CZEGO ZNAJDÊ WIÊCEJ
	//NIE BÊDE ICH DODAWAÆ, BO I TAK MUSIMY USTALIÆ JAK CHCEMY ¯EBY TO WYGL¥DA£O
	//TAK SAMO NIE DODAM ASTEROID I ŒMIECI, MODELE I TEKSTURY S¥ W FOLDERACH

	//STATEK
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});
	drawShip(programDefault, shipContext, glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.0004)), texture::spaceship);

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
	programSun = shaderLoader.CreateProgram("shaders/shader_sun.vert", "shaders/shader_sun.frag");

	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/spaceship.fbx", shipContext);
	loadModelToContext("./models/trash1.dae", trash1Context);
	loadModelToContext("./models/trash2.dae", trash2Context);

	texture::sun[0] = Core::LoadTexture("./textures/sun/sun_albedo.png");
	texture::sun[1] = Core::LoadTexture("./textures/sun/sun_normal.png");

	texture::spaceship[0] = Core::LoadTexture("./textures/spaceship/spaceship_albedo.jpg");
	texture::spaceship[1] = Core::LoadTexture("./textures/spaceship/spaceship_normal.jpg");
	texture::spaceship[2] = Core::LoadTexture("./textures/spaceship/spaceship_ao.jpg");
	texture::spaceship[3] = Core::LoadTexture("./textures/spaceship/spaceship_roughness.jpg");
	texture::spaceship[4] = Core::LoadTexture("./textures/spaceship/spaceship_metallic.jpg");

	texture::mercury[0] = Core::LoadTexture("./textures/planets/custom/planet1_albedo.png");
	texture::mercury[1] = Core::LoadTexture("./textures/planets/custom/planet1_normal.png");
	texture::mercury[2] = Core::LoadTexture("./textures/planets/custom/planet1_ao.png");
	texture::mercury[3] = Core::LoadTexture("./textures/planets/custom/planet1_roughness.png");
	texture::mercury[4] = Core::LoadTexture("./textures/planets/custom/planet1_metallic.png");

	texture::venus[0] = Core::LoadTexture("./textures/planets/custom/planet2_albedo.png");
	texture::venus[1] = Core::LoadTexture("./textures/planets/custom/planet2_normal.png");
	texture::venus[2] = Core::LoadTexture("./textures/planets/custom/planet2_ao.png");
	texture::venus[3] = Core::LoadTexture("./textures/planets/custom/planet2_roughness.png");
	texture::venus[4] = Core::LoadTexture("./textures/planets/custom/planet2_metallic.png");

	texture::earth[0] = Core::LoadTexture("./textures/planets/earth/earth_albedo.jpg");
	texture::earth[1] = Core::LoadTexture("./textures/planets/earth/earth_normal.jpg");
	texture::earth[2] = Core::LoadTexture("./textures/planets/earth/earth_ao.png");
	texture::earth[3] = Core::LoadTexture("./textures/planets/earth/earth_roughness.jpg");
	texture::earth[4] = Core::LoadTexture("./textures/planets/earth/earth_metallic.png");

	texture::mars[0] = Core::LoadTexture("./textures/planets/mars/mars_albedo.jpg");
	texture::mars[1] = Core::LoadTexture("./textures/planets/mars/mars_normal.png");
	texture::mars[2] = Core::LoadTexture("./textures/planets/mars/mars_ao.jpg");
	texture::mars[3] = Core::LoadTexture("./textures/planets/mars/mars_roughness.jpg");
	texture::mars[4] = Core::LoadTexture("./textures/planets/mars/mars_metallic.png");

	texture::jupiter[0] = Core::LoadTexture("./textures/planets/jupiter/jupiter_albedo.jpg");
	texture::jupiter[1] = Core::LoadTexture("./textures/planets/jupiter/jupiter_normal.png");
	texture::jupiter[2] = Core::LoadTexture("./textures/planets/jupiter/jupiter_ao.jpg");
	texture::jupiter[3] = Core::LoadTexture("./textures/planets/jupiter/jupiter_roughness.jpg");
	texture::jupiter[4] = Core::LoadTexture("./textures/planets/jupiter/jupiter_metallic.png");

	texture::saturn[0] = Core::LoadTexture("./textures/planets/custom/planet3_albedo.png");
	texture::saturn[1] = Core::LoadTexture("./textures/planets/custom/planet3_normal.png");
	texture::saturn[2] = Core::LoadTexture("./textures/planets/custom/planet3_ao.png");
	texture::saturn[3] = Core::LoadTexture("./textures/planets/custom/planet3_roughness.png");
	texture::saturn[4] = Core::LoadTexture("./textures/planets/custom/planet3_metallic.png");

	texture::uran[0] = Core::LoadTexture("./textures/planets/custom/planet5_albedo.jpg");
	texture::uran[1] = Core::LoadTexture("./textures/planets/custom/planet5_normal.png");
	texture::uran[2] = Core::LoadTexture("./textures/planets/custom/planet5_ao.jpg");
	texture::uran[3] = Core::LoadTexture("./textures/planets/custom/planet5_roughness.jpg");
	texture::uran[4] = Core::LoadTexture("./textures/planets/custom/planet5_metallic.png");

	texture::neptune[0] = Core::LoadTexture("./textures/planets/neptune/neptune_albedo.jpg");
	texture::neptune[1] = Core::LoadTexture("./textures/planets/neptune/neptune_normal.png");
	texture::neptune[2] = Core::LoadTexture("./textures/planets/neptune/neptune_ao.jpg");
	texture::neptune[3] = Core::LoadTexture("./textures/planets/neptune/neptune_roughness.jpg");
	texture::neptune[4] = Core::LoadTexture("./textures/planets/neptune/neptune_metallic.png");

	texture::trash1[0] = Core::LoadTexture("./textures/trash/trash1_albedo.jpg");
	texture::trash1[1] = Core::LoadTexture("./textures/trash/trash1_normal.png");
	texture::trash1[2] = Core::LoadTexture("./textures/trash/trash1_AO.jpg");
	texture::trash1[3] = Core::LoadTexture("./textures/trash/trash1_roughness.jpg");
	texture::trash1[4] = Core::LoadTexture("./textures/trash/trash1_metallic.jpg");

	texture::trash2[0] = Core::LoadTexture("./textures/trash/trash2_albedo.jpg");
	texture::trash2[1] = Core::LoadTexture("./textures/trash/trash2_normal.png");
	texture::trash2[2] = Core::LoadTexture("./textures/trash/trash2_AO.jpg");
	texture::trash2[3] = Core::LoadTexture("./textures/trash/trash2_roughness.jpg");
	texture::trash2[4] = Core::LoadTexture("./textures/trash/trash2_metallic.jpg");
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