#pragma once

#include "ExternalLibraryHeaders.h"
#include "Helper.h"
#include "Mesh.h"
#include "Camera.h"
#include <vector>

struct ElementsAndVAOs
{
	GLuint m_VAO{ 0 };
	GLuint m_numElements{ 0 };
};

class model
{
private:
	ElementsAndVAOs m_ELVAO;
	GLuint* program;
	GLuint* texture;
	std::vector<model*> childModels;
	glm::vec3 modelPos = glm::vec3(0, 0, 0);
	glm::vec3 modelRot = glm::vec3(0, 0, 0);
	glm::vec3 modelScl = glm::vec3(1, 1, 1);
public:
	model();
	~model();

	void initialize(ElementsAndVAOs Elamentvao, GLuint* p, GLuint* t);

	void addChild(model *child);
	void applyTranslation(glm::vec3 translation);
	void applyRotation(glm::vec3 rotation);
	void applyScale(glm::vec3 scale);

	glm::vec3 getPos() { return modelPos; };
	glm::vec3 getRot() { return modelRot; };
	glm::vec3 getScl() { return modelScl; };
	ElementsAndVAOs* getElementsAndVAOs() { return &m_ELVAO; };
	GLuint getProgram() { return *program; };
	GLuint getTexture() { return *texture; };
};