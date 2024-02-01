#pragma once
#include "glm.hpp"
#include "glew.h"
#include "objload.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace Core
{
	struct RenderContext
    {
		GLuint vertexArray;
		GLuint vertexBuffer;
		GLuint vertexIndexBuffer;
		int size = 0;

        void initFromOBJ(obj::Model& model);

		void initFromAssimpMesh(aiMesh* mesh);
	};

	void DrawVertexArray(const float * vertexArray, int numVertices, int elementSize);

	void DrawVertexArrayIndexed(const float * vertexArray, const int * indexArray, int numIndexes, int elementSize);


	struct VertexAttribute
	{
		const void * Pointer;
		int Size;
	};

	struct VertexData
	{
		static const int MAX_ATTRIBS = 8;
		VertexAttribute Attribs[MAX_ATTRIBS];
		int NumActiveAttribs;
		int NumVertices;
	};

	void DrawVertexArray(const VertexData & data);

	void DrawContext(RenderContext& context);

	void DrawSkybox(GLuint program, Core::RenderContext& context, GLuint TextureID, glm::vec3 cameraDir, glm::vec3 cameraPos, float aspectRatio);

	glm::mat4 createCameraMatrix(glm::vec3 cameraDir, glm::vec3 cameraPos);

	glm::mat4 createPerspectiveMatrix(float aspectRatio);
}