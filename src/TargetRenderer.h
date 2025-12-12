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
    void Draw(const Target& target, const glm::mat4& projection, float worldHeight);

private:
    Shader* shader;
    GLuint targetVAO, targetVBO;

    void InitShader();
    void InitBuffers();
};