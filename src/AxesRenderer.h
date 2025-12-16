#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"

class AxesRenderer {
public:
    AxesRenderer();
    ~AxesRenderer();

	// Rysuje osie na podstawie macierzy projekcji (skali ekranu)
    void Draw(const glm::mat4& projection);

private:
    GLuint VAO, VBO;
    int vertexCount;
    Shader* shader;

    void InitShader();
    void InitData();
};