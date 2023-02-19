#include "model.h"

model::model()
{
}

model::~model()
{
	glDeleteBuffers(1, &m_ELVAO.m_VAO);
}

void model::initialize(ElementsAndVAOs Elamentvao, GLuint* p, GLuint* t)
{
	m_ELVAO.m_numElements = Elamentvao.m_numElements;
	m_ELVAO.m_VAO = Elamentvao.m_VAO;
	program = p;
	texture = t;
}

void model::addChild(model *child)
{
	childModels.push_back(child);
}

void model::applyTranslation(glm::vec3 translation)
{
	modelPos += translation;
	for (size_t i = 0; i < childModels.size(); i++)
	{
		childModels[i]->applyTranslation(translation);
	}
}

void model::applyRotation(glm::vec3 rotation)
{
	modelRot += rotation;
	for (size_t i = 0; i < childModels.size(); i++)
	{
		childModels[i]->applyRotation(rotation);
	}
}

void model::applyScale(glm::vec3 scale)
{
	modelScl.r = modelScl.r * scale.r;
	modelScl.g = modelScl.g * scale.g;
	modelScl.b = modelScl.b * scale.b;
	for (size_t i = 0; i < childModels.size(); i++)
	{
		childModels[i]->applyScale(scale);
	}
}
