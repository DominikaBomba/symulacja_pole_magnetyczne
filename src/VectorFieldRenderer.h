#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"

class VectorFieldRenderer {
public:
    VectorFieldRenderer();
    ~VectorFieldRenderer();

    // Przyjmuje macierz kamery, wektor pola oraz flagê trybu gry
    void Draw(const glm::mat4& pv, const glm::dvec3& B_field, bool gameMode);

private:
    GLuint VAO, VBO;
    Shader* shader;

    void Init();
};