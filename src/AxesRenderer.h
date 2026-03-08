#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"

class AxesRenderer {
public:
    AxesRenderer();
    ~AxesRenderer();

    // Rysuje osie na podstawie macierzy widoku-projekcji
    void Draw(const glm::mat4& pv);

private:
    GLuint VAO, VBO;
    int vertexCount;
    Shader* shader;

    void InitShader();
    void InitData();
};