#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>


#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Render_Sprite.h"
#include "Texture.h"
#include "Structures.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <random>
#include <chrono>

std::map<std::string, std::map<int, bool>> trashDisplayInfoMap = {
	{"Mercury", {{0, true}, {1, true}, {2, true}, {3, true}}},
	{"Venus", {{0, true}, {1, true}, {2, true}, {3, true}}},
	{"Earth", {{0, true}, {1, true}, {2, true}, {3, true}}},
	{"Mars", {{0, true}, {1, true}, {2, true}, {3, true}}},
	{"Jupiter", {{0, true}, {1, true}, {2, true}, {3, true}}},
	{"Saturn", {{0, true}, {1, true}, {2, true}, {3, true}}},
	{"Uran", {{0, true}, {1, true}, {2, true}, {3, true}}},
	{"Neptun", {{0, true}, {1, true}, {2, true}, {3, true}}},
};

std::vector<std::vector<glm::vec3>> asteroidPositions(4, std::vector<glm::vec3>(8, glm::vec3(0.f, 0.f, 0.f)));

std::map<int, std::pair<glm::vec3, bool>> circlePositions{
		{1, {glm::vec3(-58.f, -50.f, 30.f), false}},
		{2, {glm::vec3(-58.f, -50.f, -30.f), false}},
		{3, {glm::vec3(58.f, -50.f, 30.f), false}},
		{4, {glm::vec3(58.f, -50.f, -30.f), false}},
		{5, {glm::vec3(30.f, -50.f, 58.f), false}},
		{6, {glm::vec3(-30.f, -50.f, 58.f), false}},
		{7, {glm::vec3(30.f, -50.f, -58.f), false}},
		{8, {glm::vec3(-30.f, -50.f, -58.f), false}}
};

Textures textures;
TextureSprite sprites;
Planets planets;
Laser laser;
Contexts contexts;

GLuint programDefault;
GLuint programSun;
GLuint programSprite;
GLuint programSkybox;
GLuint skyboxTexture;

GLuint programBlur;
GLuint programBloomFinal;

Core::Shader_Loader shaderLoader;
Core::RenderSprite* renderSprite;
Core::RenderSprite* renderSpriteEnd;
Core::RenderSprite* renderSpriteStart;

glm::vec3 cameraPos = glm::vec3(20.f, 0, 0);
glm::vec3 cameraDir = glm::vec3(-1.f, 0.f, 0.f);
glm::vec3 spaceshipPos = glm::vec3(40.f, -20.f, 0);
glm::vec3 spaceshipDir = glm::vec3(-1.f, 0.f, 0.f);

glm::vec3 spotlightPos = glm::vec3(0, 0, 0);
glm::vec3 spotlightConeDir = glm::vec3(0, 0, 0);
glm::vec3 spotlightColor = glm::vec3(0.4, 0.4, 0.9) * 3;
float spotlightPhi = 3.14 / 4;

bool showMissions = false;
bool hideInstruction = false;
bool missionsComplete = false;
bool raceCompleted = false;

float aspectRatio = 1.f;
float exposition = 1.f;

glm::vec3 lightColor = glm::vec3(0.9, 0.7, 0.8) * 100;

double lastMouseX = 0.0;
double lastMouseY = 0.0;

float cameraSide = 0.0f;
float cameraUp = 0.0f;
float spaceshipSide = 0.0f;
float spaceshipUp = 0.0f;

float lastTime = -1.f;
float deltaTime = 0.f;

float spaceshipRadius = 0.5f;
int trashDestroyed = 0;
int circleVisited = 0;

unsigned int bloomTexture;

std::chrono::time_point<std::chrono::steady_clock> start_time;
std::chrono::time_point<std::chrono::steady_clock> end_time;

bool bloom = true;
float exposure = 0.8f;
unsigned int hdrFBO;
unsigned int pingpongFBO[2];
unsigned int pingpongColorbuffers[2];
unsigned int colorBuffers[2];
unsigned int quadVAO = 0;
unsigned int quadVBO;

void updateDeltaTime(float time) {
	if (lastTime < 0) {
		lastTime = time;
		return;
	}

	deltaTime = time - lastTime;
	if (deltaTime > 0.1) deltaTime = 0.1;
	lastTime = time;
}

void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void drawObjectTexture(GLuint program,Core::RenderContext& context, TextureSet textures, glm::mat4 modelMatrix) {

	glm::mat4 viewProjectionMatrix = Core::createPerspectiveMatrix(aspectRatio) * Core::createCameraMatrix(cameraDir, cameraPos);
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	glUseProgram(programDefault);
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

void drawTrash(float planetX, float planetZ, float time, float orbitRadius, glm::vec3 scalePlanet,std::string planetName) {
	
	float orbitSpeed = 1.f;
	int id=0;
	const auto& trashProps = planets.trashProperties[planetName];

	for (int i = 0; i < trashProps.size() && i < 4; ++i) {
		if (trashProps[i].destroyed && trashDisplayInfoMap[planetName][i]) {
			trashDisplayInfoMap[planetName][i] = false;
			trashDestroyed++;
		}
	}

	planets.trashProperties[planetName].clear();

	for (int i = 1; i <= 2; ++i) {

		float trashX1 = planetX + orbitRadius * cos(orbitSpeed * time + i * 100);
		float trashZ1 = planetZ + orbitRadius * sin(orbitSpeed * time + i * 50);
		glm::vec3 trashPos1(trashX1, 0.5f, trashZ1);
		glm::mat4 modelMatrix1 = glm::translate(glm::vec3(trashX1, 0.5f, trashZ1)) *
			glm::rotate(2.f * time, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(0.5f * time, glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(glm::vec3(0.7f));

		planets.trashProperties[planetName].push_back({ trashPos1, 2.f});

		float trashX2 = planetX + orbitRadius * cos(orbitSpeed * time - i * 50);
		float trashZ2 = planetZ + orbitRadius * sin(orbitSpeed * time - i * 100);
		glm::vec3 trashPos2(trashX2, -0.5f, trashZ2);
		glm::mat4 modelMatrix2 = glm::translate(glm::vec3(trashX2, -0.5f, trashZ2)) *
			glm::rotate(2.f * time, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(0.5f * time, glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(glm::vec3(1.5f));

		planets.trashProperties[planetName].push_back({ trashPos2,  2.f });

		if (trashDisplayInfoMap[planetName][id]) drawObjectTexture(programDefault, contexts.trash1Context, textures.trash1, modelMatrix1);
		if (trashDisplayInfoMap[planetName][id + 1]) drawObjectTexture(programDefault, contexts.trash2Context, textures.trash2, modelMatrix2);

		id=id+2;
	}
}



void drawPlanet(Core::RenderContext& context, TextureSet textures, float planetOrbitRadius, float planetOrbitSpeed, float time, glm::vec3 scalePlanet, float trashOrbitRadius, std::string& planetName) {
	float planetX = planetOrbitRadius * cos(planetOrbitSpeed * time);
	float planetZ = planetOrbitRadius * sin(planetOrbitSpeed * time);
	
	planets.planetsProperties[planetName] = { glm::vec3(planetX,0.f,planetZ), trashOrbitRadius - 1.f };
	glm::mat4 modelMatrix = glm::translate(glm::vec3(planetX, 0, planetZ)) * glm::scale(scalePlanet);
	glm::mat4 viewProjectionMatrix = Core::createPerspectiveMatrix(aspectRatio) * Core::createCameraMatrix(cameraDir, cameraPos);
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	glUseProgram(programDefault);
	glUniformMatrix4fv(glGetUniformLocation(programDefault, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programDefault, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform1f(glGetUniformLocation(programDefault, "exposition"), exposition);
	glUniform3f(glGetUniformLocation(programDefault, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(programDefault, "lightPos"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(programDefault, "lightColor"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(programDefault, "spotlightPos"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
	glUniform3f(glGetUniformLocation(programDefault, "spotlightConeDir"), spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);
	glUniform3f(glGetUniformLocation(programDefault, "spotlightColor"), spotlightColor.r, spotlightColor.g, spotlightColor.b);
	glUniform1f(glGetUniformLocation(programDefault, "spotlightPhi"), spotlightPhi);
	Core::SetActiveTexture(textures.albedo, "albedoTexture", programDefault, 0);
	Core::SetActiveTexture(textures.normal, "normalTexture", programDefault, 1);
	Core::SetActiveTexture(textures.ao, "aoTexture", programDefault, 2);
	Core::SetActiveTexture(textures.roughness, "roughnessTexture", programDefault, 3);
	Core::SetActiveTexture(textures.metallic, "metallicTexture", programDefault, 4);
	Core::DrawContext(context);

	// THRASH
	drawTrash(planetX, planetZ, time, trashOrbitRadius,scalePlanet,planetName);
}

glm::vec3 sunDirection = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)); // You can adjust the direction
void drawSun(Core::RenderContext& context, glm::mat4 modelMatrix,TextureSet textures) {

	glm::mat4 viewProjectionMatrix = Core::createPerspectiveMatrix(aspectRatio) * Core::createCameraMatrix(cameraDir, cameraPos);
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUseProgram(programSun);

	glUniform3f(glGetUniformLocation(programSun, "lightDir"), sunDirection.x, sunDirection.y, sunDirection.z);
	glUniformMatrix4fv(glGetUniformLocation(programSun, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programSun, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(programSun, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(programSun, "lightPos"), 0.0f, 0.0f, 0.0f);
	Core::SetActiveTexture(textures.albedo, "sunAlbedo", programSun, 0);
	Core::SetActiveTexture(textures.normal, "sunNormal", programSun, 1);
	Core::DrawContext(context);

}

bool checkCollision(glm::vec3 object1Pos, float object1Radius) {
	float distance;

	for (const auto& pair : planets.planetsProperties) {
			distance = glm::length(object1Pos - pair.second.coordinates);
			if (distance < (object1Radius + pair.second.orbit)) return true;
		}
	for (const auto& row : asteroidPositions) {
		for (const auto& asteroidPos : row) {
			distance = glm::length(object1Pos - asteroidPos);
			if (distance < (object1Radius + 1.5f)) return true;
		}
	}
	for ( auto& planetEntry : planets.trashProperties) {
		const std::string& planetName = planetEntry.first;
		for ( auto& trashInfo : planetEntry.second) {
			distance = glm::length(object1Pos - trashInfo.coordinates);
			if (distance < (object1Radius + trashInfo.orbit)) {
				trashInfo.destroyed = true;
				return true;
			}
		}
	}
	for (auto& pair : circlePositions) {
		glm::vec3& position = pair.second.first;
		bool& visited = pair.second.second;
		float distance = glm::length(object1Pos - position);
		if (!visited && distance < (object1Radius + 5.f)) {
			visited = true;
			circleVisited++;
			if (circleVisited == 1) start_time = std::chrono::steady_clock::now();
			if (circleVisited == 8) {
				end_time = std::chrono::steady_clock::now();
				std::chrono::duration<double> elapsed_seconds = end_time - start_time;
				if (elapsed_seconds.count() < 15.0) {
					raceCompleted = true;
					std::cout << "skonczyles wyscig";
				}
				else
				{
					circleVisited = 0;
					for (auto& pair : circlePositions) {
						pair.second.second = false;
					}
				}
			}
		}
	}

	return false;
}

void renderScene(GLFWwindow* window)
{
	glClearColor(0.0f, 0.0f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 transformation;
	float time = glfwGetTime();
	updateDeltaTime(time);

	Core::DrawSkybox(programSkybox, contexts.skyboxContext, skyboxTexture, cameraDir, cameraPos, aspectRatio);
	glClear(GL_DEPTH_BUFFER_BIT);

	// S£OÑCE
	glm::vec3 sunPosition = glm::vec3(0, 0, 0);
	planets.planetsProperties["Sun"] = { sunPosition, 30.f };
	drawSun(contexts.sphereContext, glm::scale(glm::vec3(30.f)) * glm::translate(sunPosition), textures.sun);

	// UK£AD S£ONECZNY - PLANETY (NA RAZIE BEZ KSIÊ¯YCA)
	drawPlanet(contexts.sphereContext, textures.planets.mercury, 15.0f * 5, 0.2f, time, glm::vec3(0.5 * 9), 1 * 9, std::string("Mercury"));
	drawPlanet(contexts.sphereContext, textures.planets.venus, 20.0f * 5, 0.175f, time, glm::vec3(1.f * 9), 1.5 * 9, std::string("Venus"));
	drawPlanet(contexts.sphereContext, textures.planets.earth, 25.0f * 5, 0.15f, time, glm::vec3(1.3f * 9), 2 * 9, std::string("Earth"));
	drawPlanet(contexts.sphereContext, textures.planets.mars, 30.0f * 5, 0.125f, time, glm::vec3(1.3f * 9), 2 * 9, std::string("Mars"));
	drawPlanet(contexts.sphereContext, textures.planets.jupiter, 40.0f * 5, 0.1f, time, glm::vec3(2.5f * 9), 3 * 9, std::string("Jupiter"));
	drawPlanet(contexts.sphereContext, textures.planets.saturn, 50.0f * 5, 0.075f, time, glm::vec3(2.2f * 9), 3 * 9, std::string("Saturn"));
	drawPlanet(contexts.sphereContext, textures.planets.uran, 55.0f * 5, 0.05f, time, glm::vec3(1.6f * 9), 2.5 * 9, std::string("Uran"));
	drawPlanet(contexts.sphereContext, textures.planets.neptune, 60.0f * 5, 0.025f, time, glm::vec3(1.8f * 9), 2.5 * 9, std::string("Neptun"));

	//LICZBY LOSOWE
	glm::vec3 initialAsteroidPosition(0.f, 40.f, 0.f);
	float offset = sin(time) * 2.0f;

	std::default_random_engine generator; // Inicjalizacja generatora liczb losowych
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f); // Zakres losowych wartoœci od -1.0 do 1.0

	//asteroidy
	for (int row = 0; row < 3; ++row)
	{
		for (int col = 0; col < 8; ++col)
		{
			glm::vec3 position = initialAsteroidPosition + glm::vec3(col * 10.f, 0.f, row * 10.f);
			if (row % 2 == 1) position.x += 10.f * 0.5f;
			position.x += offset;

			// Dodaj losowoœæ do pozycji asteroidy
			position.x += distribution(generator) * 5.f;
			position.y += distribution(generator) * 5.f;
			position.z += distribution(generator) * 5.f;

			asteroidPositions[row][col] = position;

			transformation = glm::translate(glm::mat4(1.0f), position) *
				glm::rotate(2.f * time, glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(0.5f * time, glm::vec3(1.0f, 0.0f, 0.0f));

			drawObjectTexture(programDefault, contexts.asteroidContext, textures.asteroid, transformation);
		}
	}

	glm::vec3 position;

	position = glm::vec3(-58.f + 3 * sin(time*2), -50.f, -8.f);
	position.z += 15.f * cos(time*2);
	transformation = glm::translate(position) * glm::rotate(2.f * time, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(0.5f * time, glm::vec3(1.0f, 0.0f, 0.0f))*glm::scale(glm::vec3(2.f));
	drawObjectTexture(programDefault, contexts.asteroidContext, textures.asteroid, transformation);
	asteroidPositions[3][0] = position;

	position = glm::vec3(58.f + 3 * sin(time * 2), -50.f, -8.f);
	position.z += 15.f * cos(time * 2);
	transformation = glm::translate(position) * glm::rotate(2.f * time, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(0.5f * time, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(2.f));
	drawObjectTexture(programDefault, contexts.asteroidContext, textures.asteroid, transformation);
	asteroidPositions[3][1] = position;

	position = glm::vec3(-8.f, -50.f, 58.f + 3 * sin(time * 2));
	position.x += 15.f * cos(time * 2);
	transformation = glm::translate(position) * glm::rotate(2.f * time, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(0.5f * time, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(2.f));
	drawObjectTexture(programDefault, contexts.asteroidContext, textures.asteroid, transformation);
	asteroidPositions[3][2] = position;

	position = glm::vec3(-8.f, -50.f, -58.f + 3 * sin(time * 2));
	position.x += 15.f * cos(time * 2);
	transformation = glm::translate(position) * glm::rotate(2.f * time, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(0.5f * time, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(2.f));
	drawObjectTexture(programDefault, contexts.asteroidContext, textures.asteroid, transformation);
	asteroidPositions[3][3] = position;

	// tor wyœcigowy
	transformation = glm::translate(glm::vec3(0.f, -50.f, 0.f))*glm::scale( glm::vec3(50.f))* glm::rotate(glm::radians(270.f), glm::vec3(1.0f, 0.f, 0.0f));
	drawObjectTexture(programDefault, contexts.barierContext, textures.barier, transformation);
	transformation = glm::translate(glm::vec3(0.f, -50.f, 0.f)) * glm::scale(glm::vec3(70.f)) * glm::rotate(glm::radians(270.f), glm::vec3(1.0f, 0.f, 0.0f));
	drawObjectTexture(programDefault, contexts.barierContext, textures.barier, transformation);

	TextureSet texture;
	bool visited;

	// Rysuj pierwsze cztery okrêgi
	auto it = circlePositions.begin();
	for (int i = 0; i < 4; ++i, ++it) {
		const glm::vec3& pos = it->second.first;
		bool visited = it->second.second;
		if (visited) texture = textures.circle_dark; else texture = textures.circle_bright;
		// Rysuj okr¹g z odpowiedni¹ tekstur¹ i obrótem
		drawObjectTexture(programDefault, contexts.circleContext, texture,
			glm::translate(pos) * glm::scale(glm::vec3(15.f)) * glm::rotate(glm::radians(270.f), glm::vec3(1.0f, 0.f, 0.0f)));
	}

	// Rysuj kolejne cztery okrêgi
	for (; it != circlePositions.end(); ++it) {
		const glm::vec3& pos = it->second.first;
		bool visited = it->second.second;
		if (visited) texture = textures.circle_dark; else texture = textures.circle_bright;
		// Rysuj okr¹g z odpowiedni¹ tekstur¹ i innym obrótem
		drawObjectTexture(programDefault, contexts.circleContext, texture,
			glm::translate(pos) * glm::scale(glm::vec3(15.f)) * glm::rotate(glm::radians(270.f), glm::vec3(1.0f, 0.f, 0.0f)) * glm::rotate(glm::radians(90.f), glm::vec3(0.f, 0.f, 1.0f)));
	}
	//STATEK
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 spaceshipCameraRotationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});
	drawObjectTexture(programDefault, contexts.shipContext, textures.spaceship, glm::translate(spaceshipPos) * spaceshipCameraRotationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.0004)));

	// Czy wystrzelono laser
	if (laser.isActive)
	{
		float currentTime = glfwGetTime();
		float elapsedTime = currentTime - laser.startTime;

		if (elapsedTime < laser.duration)
		{
			float laserSpeed = 50.f;
			laser.position += laser.direction * laserSpeed * deltaTime;

			// Rysuj laser
			glm::mat4 laserModelMatrix = glm::translate(laser.position) * glm::scale(glm::vec3(0.0003));
			drawObjectTexture(programDefault, contexts.laserContext, textures.laser, laserModelMatrix);
			if(checkCollision(laser.position,0.5f)) laser.isActive = false;
		}
		else laser.isActive = false;
	}

	if (!hideInstruction)
	{
		renderSpriteStart->DrawSprite(programSprite, 740.0f, 880.0f);
	}

	if (showMissions)
	{
		renderSprite->DrawSprite(programSprite, 740.0f, 580.0f);
	}

	if (trashDestroyed == 10)
	{
		renderSprite->UpdateSprite(sprites.sprite_2);
		missionsComplete = true;
	}
	if (missionsComplete)
	{
		renderSpriteEnd->DrawSprite(programSprite, 740.0f, 580.0f);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	bool horizontal = true, first_iteration = true;
	unsigned int amount = 10;
	glUseProgram(programBlur);
	for (unsigned int i = 0; i < amount; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
		glUniform1i(glGetUniformLocation(programBlur, "horizontal"), horizontal);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
		renderQuad();
		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(programBloomFinal);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
	glUniform1i(glGetUniformLocation(programBloomFinal, "bloom"), bloom);
	glUniform1f(glGetUniformLocation(programBloomFinal, "exposure"), exposure);
	renderQuad();

	glUseProgram(0);
	glfwSwapBuffers(window);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
	std::cout << width << height;
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
	textures.sun.albedo = Core::LoadTexture("./textures/sun/sun_albedo.jpg");
	textures.sun.normal = Core::LoadTexture("./textures/sun/sun_normal.jpg");

	textures.spaceship.albedo = Core::LoadTexture("./textures/spaceship/spaceship_albedo.jpg");
	textures.spaceship.normal = Core::LoadTexture("./textures/spaceship/spaceship_normal.jpg");
	textures.spaceship.ao = Core::LoadTexture("./textures/spaceship/spaceship_ao.jpg");
	textures.spaceship.roughness = Core::LoadTexture("./textures/spaceship/spaceship_roughness.jpg");
	textures.spaceship.metallic = Core::LoadTexture("./textures/spaceship/spaceship_metallic.jpg");

	textures.planets.mercury = loadTextureSet("./textures/planets/mercury/planet1_albedo.png", "./textures/planets/mercury/planet1_normal.png", "./textures/planets/mercury/planet1_ao.png", "./textures/planets/mercury/planet1_roughness.png", "./textures/planets/mercury/planet1_metallic.png");
	textures.planets.venus = loadTextureSet("./textures/planets/venus/planet2_albedo.png", "./textures/planets/venus/planet2_normal.png", "./textures/planets/venus/planet2_ao.png", "./textures/planets/venus/planet2_roughness.png", "./textures/planets/venus/planet2_metallic.png");
	textures.planets.venus = loadTextureSet("./textures/planets/venus/planet2_albedo.png", "./textures/planets/venus/planet2_normal.png", "./textures/planets/venus/planet2_ao.png", "./textures/planets/venus/planet2_roughness.png", "./textures/planets/venus/planet2_metallic.png");
	textures.planets.earth = loadTextureSet("./textures/planets/earth/earth_albedo.jpg", "./textures/planets/earth/earth_normal.jpg", "./textures/planets/earth/earth_ao.png", "./textures/planets/earth/earth_roughness.jpg", "./textures/planets/earth/earth_metallic.png");
	textures.planets.mars = loadTextureSet("./textures/planets/mars/mars_albedo.jpg", "./textures/planets/mars/mars_normal.png", "./textures/planets/mars/mars_ao.jpg", "./textures/planets/mars/mars_roughness.jpg", "./textures/planets/mars/mars_metallic.png");
	textures.planets.jupiter = loadTextureSet("./textures/planets/jupiter/jupiter_albedo.jpg", "./textures/planets/jupiter/jupiter_normal.png", "./textures/planets/jupiter/jupiter_ao.jpg", "./textures/planets/jupiter/jupiter_roughness.jpg", "./textures/planets/jupiter/jupiter_metallic.png");
	textures.planets.saturn = loadTextureSet("./textures/planets/saturn/planet3_albedo.png", "./textures/planets/saturn/planet3_normal.png", "./textures/planets/saturn/planet3_ao.png", "./textures/planets/saturn/planet3_roughness.png", "./textures/planets/saturn/planet3_metallic.png");
	textures.planets.uran = loadTextureSet("./textures/planets/uranus/planet5_albedo.jpg", "./textures/planets/uranus/planet5_normal.png", "./textures/planets/uranus/planet5_ao.jpg", "./textures/planets/uranus/planet5_roughness.jpg", "./textures/planets/uranus/planet5_metallic.png");
	textures.planets.neptune = loadTextureSet("./textures/planets/neptune/neptune_albedo.jpg", "./textures/planets/neptune/neptune_normal.png", "./textures/planets/neptune/neptune_ao.jpg", "./textures/planets/neptune/neptune_roughness.jpg", "./textures/planets/neptune/neptune_metallic.png");
	textures.trash1 = loadTextureSet("./textures/trash/trash1_albedo.jpg", "./textures/trash/trash1_normal.png", "./textures/trash/trash1_AO.jpg", "./textures/trash/trash1_roughness.jpg", "./textures/trash/trash1_metallic.jpg");
	textures.trash2 = loadTextureSet("./textures/trash/trash2_albedo.jpg", "./textures/trash/trash2_normal.png", "./textures/trash/trash2_AO.jpg", "./textures/trash/trash2_roughness.jpg", "./textures/trash/trash2_metallic.jpg");
	textures.asteroid = loadTextureSet("./textures/asteroid/asteroid_albedo.png", "./textures/asteroid/asteroid_normal.png", "./textures/planets/mars/mars_ao.jpg", "./textures/asteroid/asteroid_roughness.png", "./textures/asteroid/asteroid_metallic.png");
	textures.barier = loadTextureSet("./textures/barier/barier_albedo.jpeg", "./textures/barier/barier_normal.png", "./textures/planets/barier/barier_ao.png", "./textures/barier/barier_roughness.jpeg", "./textures/barier/barier_metallic.png");
	textures.laser = loadTextureSet("./textures/spaceship/laser_albedo.jpg","./textures/spaceship/laser_normal.png","./textures/spaceship/laser_ao.jpg","./textures/spaceship/laser_roughness.jpg","./textures/spaceship/laser_metallic.jpg");
	textures.circle_bright = loadTextureSet("./textures/circle/circle_albedo_bright.jpg", "./textures/circle/circle_normal.png", "./textures/circle/circle_ao.jpg", "./textures/circle/circle_roughness.jpg", "./textures/circle/circle_metallic.jpg");
	textures.circle_dark = loadTextureSet("./textures/circle/circle_albedo_dark.jpg", "./textures/circle/circle_normal.png", "./textures/circle/circle_ao.jpg", "./textures/circle/circle_roughness.jpg", "./textures/circle/circle_metallic.jpg");

	sprites.sprite_1 = Core::LoadTexture("./img/mission_board_1.png");
	sprites.sprite_2 = Core::LoadTexture("./img/mission_board_2.png");
	sprites.sprite_3 = Core::LoadTexture("./img/mission_board_3.png");
	sprites.sprite_4 = Core::LoadTexture("./img/mission_board_4.png");
	sprites.sprite_end = Core::LoadTexture("./img/mission_board_end.png");
	sprites.sprite_start = Core::LoadTexture("./img/instruction.png");

	std::string skyboxFilepaths[6] = {
	"./textures/skybox/skybox_right.png",
	"./textures/skybox/skybox_left.png",
	"./textures/skybox/skybox_top.png",
	"./textures/skybox/skybox_bot.png",
	"./textures/skybox/skybox_front.png",
	"./textures/skybox/skybox_back.png"
	};
	skyboxTexture = Core::LoadSkybox(skyboxFilepaths);
}

void initBloom() {
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}
	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1920, 1080);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}
	glUseProgram(programBlur);
	glUniform1i(glGetUniformLocation(programBlur, "image"), 0);

	glUseProgram(programBloomFinal);
	glUniform1i(glGetUniformLocation(programBloomFinal, "scene"), 0);
	glUniform1i(glGetUniformLocation(programBloomFinal, "bloomBlur"), 1);

}

void init(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetCursorPos(window, &lastMouseX, &lastMouseY);

	glEnable(GL_DEPTH_TEST);

	programDefault = shaderLoader.CreateProgram("shaders/shader_default.vert", "shaders/shader_default.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_sun.vert", "shaders/shader_sun.frag");
	programSprite = shaderLoader.CreateProgram("shaders/shader_sprite.vert", "shaders/shader_sprite.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");

	programBlur = shaderLoader.CreateProgram("shaders/shader_blur.vert", "shaders/shader_blur.frag");
	programBloomFinal = shaderLoader.CreateProgram("shaders/shader_bloom_final.vert", "shaders/shader_bloom_final.frag");

	loadModelToContext("./models/sphere.obj", contexts.sphereContext);
	loadModelToContext("./models/spaceship.fbx", contexts.shipContext);
	loadModelToContext("./models/trash1.dae", contexts.trash1Context);
	loadModelToContext("./models/trash2.dae", contexts.trash2Context);
	loadModelToContext("./models/asteroid.obj", contexts.asteroidContext);
	loadModelToContext("./models/laser.glb", contexts.laserContext);
	loadModelToContext("./models/cube.obj", contexts.skyboxContext);
	loadModelToContext("./models/barier.fbx", contexts.barierContext);
	loadModelToContext("./models/circle.dae", contexts.circleContext);

	initTextures();

	renderSprite = new Core::RenderSprite();
	renderSpriteEnd = new Core::RenderSprite();
	renderSpriteStart = new Core::RenderSprite();

	renderSprite->UpdateSprite(sprites.sprite_1);
	renderSpriteEnd->UpdateSprite(sprites.sprite_end);
	renderSpriteStart->UpdateSprite(sprites.sprite_start);
		
	initBloom();
}

void shutdown(GLFWwindow* window)
{
	delete renderSprite;
	delete renderSpriteEnd;
	delete renderSpriteStart;
	shaderLoader.DeleteProgram(programDefault);
	glDeleteTextures(1, &skyboxTexture);
}

//obsluga wejscia
void processInput(GLFWwindow* window)
{
	float angleSpeed = 0.05f * deltaTime * 10;
	float moveSpeed = 0.6f * deltaTime * 60;
	
	// Kopiuj aktualn¹ pozycjê statku do nowej zmiennej
	glm::vec3 newSpaceshipPos = spaceshipPos;

	// SprawdŸ kolizjê po dodaniu wartoœci

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) hideInstruction = true;
	if (hideInstruction == true) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) newSpaceshipPos += spaceshipDir * moveSpeed;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) newSpaceshipPos -= spaceshipDir * moveSpeed;
		if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) showMissions = true; else showMissions = false;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !laser.isActive)
		{
			laser.isActive = true;
			laser.position = spaceshipPos;
			laser.direction = spaceshipDir;
			laser.startTime = glfwGetTime();
		}
	}
	// Jeœli nie wykryto kolizji, zaktualizuj pozycjê statku
	if (checkCollision(newSpaceshipPos, spaceshipRadius) == false) spaceshipPos = newSpaceshipPos;

	cameraPos = spaceshipPos - 1.5 * spaceshipDir + glm::vec3(0, 1, 0) * 0.5f;
	cameraDir = spaceshipDir;

	spotlightPos = spaceshipPos + 0.2 * spaceshipDir;
	spotlightConeDir = spaceshipDir;
	
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	double mouseMoveX = mouseX - lastMouseX;
	double mouseMoveY = mouseY - lastMouseY;
	float mouseSensitivity = 0.1f;

	cameraSide += static_cast<float>(mouseMoveX) * mouseSensitivity;
	cameraUp -= static_cast<float>(mouseMoveY) * mouseSensitivity;

	spaceshipSide += static_cast<float>(mouseMoveX) * mouseSensitivity;
	spaceshipUp -= static_cast<float>(mouseMoveY) * mouseSensitivity;

	cameraUp = glm::clamp(cameraUp, -89.0f, 89.0f);
	spaceshipUp = glm::clamp(cameraUp, -89.0f, 89.0f);

	lastMouseX = mouseX;
	lastMouseY = mouseY;

	glm::quat cameraRotation = glm::quat(glm::vec3(glm::radians(cameraUp), glm::radians(-cameraSide), 0.0f));
	cameraDir = glm::lerp(cameraDir, glm::rotate(cameraRotation, glm::vec3(0.0f, 0.0f, -1.0f)), 0.1f);

	glm::quat spaceshipRotation = glm::quat(glm::vec3(glm::radians(spaceshipUp), glm::radians(-spaceshipSide), 0.0f));
	spaceshipDir = glm::lerp(spaceshipDir, glm::rotate(spaceshipRotation, glm::vec3(0.0f, 0.0f, -1.0f)), 0.1f);

}

void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		renderScene(window);
		glfwPollEvents();
	}
}