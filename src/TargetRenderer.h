#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Target.h"

class TargetRenderer {
public:
    TargetRenderer();
    ~TargetRenderer();

    // Rysuje cel
    void Draw(const Target& target, const glm::mat4& pv, float worldHeight);

private:
    Shader* shader;
	GLuint targetVAO, targetVBO, targetEBO; //element buffer object do rysowania kulki jako siatki trójk¹tów
    unsigned int indexCount;

    void InitShader();
    void InitBuffers();
};