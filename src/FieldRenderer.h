#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Shader.h"

class FieldRenderer {
public:
    FieldRenderer();
    ~FieldRenderer();

    void Draw(const glm::mat4& pv);

private:
    GLuint bgVAO, bgVBO, bgEBO;
    Shader* bgShader; // Wskaünik na shader t≥a

    void InitRenderData();
    void InitShader();
};