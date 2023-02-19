#pragma once

#include "model.h"

class Renderer
{
private:
	// Program object - to host shaders
	GLuint m_program{ 0 };
	GLuint m_program2{ 0 };
	
	// Vertex Array Object to wrap all render settings
	GLuint m_VAO{ 0 };

	// Number of elments to use when rendering
	//GLuint m_numElements{ 0 };

	//std::vector<ElementsAndVAOs> m_ELVAO;

	std::vector<model> models;

	GLuint texture{ 0 };
	GLuint TerrianTexture{ 0 };


	glm::vec3 CamaraPos{glm::vec3(-13.82f, 5.0f, 1.886f)};

	//float rotAngle{0};

	bool m_wireframe{ false };
	bool m_cullFace{ true };	
public:
	Renderer();
	~Renderer();

	// Draw GUI
	void DefineGUI();

	// Create the program. This is the compiled shaders.
	bool CreateProgram(std::string vertexName, std::string fragmentName, GLuint &program);

	// Create and / or load geometry, this is like 'level load'
	bool InitialiseGeometry();

	// Render the scene
	void Render(const Helpers::Camera& camera, float deltaTime, glm::vec3 camarapos);

	ElementsAndVAOs newElementsAndVAOs(GLuint VAO, GLuint Elements);

	GLuint& getProgram() { return m_program; };
	GLuint& getProgram2() { return m_program2; };
};
