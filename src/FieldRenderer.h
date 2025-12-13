#pragma once
#include <glad/glad.h>
#include "Shader.h"

class FieldRenderer {
public:
    FieldRenderer();
    ~FieldRenderer();

    void Draw(float Bz, float aspectRatio, bool gameMode);

private:
    GLuint bgVAO, bgVBO, bgEBO;
    Shader* bgShader; // Wskaünik na shader t≥a

    void InitRenderData();
    void InitShader();
};