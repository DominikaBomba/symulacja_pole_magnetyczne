#include "TargetRenderer.h"
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

//Shadery celu
const char* TARGET_VS = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    uniform mat4 projection;
    void main() { 
        gl_Position = projection * vec4(aPos, 0.0, 1.0); 
    }
)";

const char* TARGET_FS = R"(
    #version 330 core
    out vec4 FragColor;
    void main() { 
        FragColor = vec4(0.9, 0.3, 0.3, 1.0); // Czerwony 
    }
)";

TargetRenderer::TargetRenderer() {
    InitShader();
    InitBuffers();
}

TargetRenderer::~TargetRenderer() {
    glDeleteVertexArrays(1, &targetVAO);
    glDeleteBuffers(1, &targetVBO);
    delete shader;
}

void TargetRenderer::InitShader() {
    shader = new Shader(TARGET_VS, TARGET_FS);
}

void TargetRenderer::InitBuffers() {
    // Inicjalizacja bufora dla celu (pojedynczy punkt)
    glGenVertexArrays(1, &targetVAO);
    glGenBuffers(1, &targetVBO);

    glBindVertexArray(targetVAO);
    glBindBuffer(GL_ARRAY_BUFFER, targetVBO);
    // Rezerwujemy pamiêæ na 1 punkt (2 floaty: x, y)
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void TargetRenderer::Draw(const Target& target, const glm::mat4& projection, float worldHeight) {
    if (!shader) return;

    shader->Use();
    shader->SetMat4("projection", projection);

    // 1. Aktualizacja pozycji celu
    float pos[2] = { (float)target.position.x, (float)target.position.y };
    glBindBuffer(GL_ARRAY_BUFFER, targetVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), pos);

    glPointSize(target.isHit ? 40.0f : 30.0f);

    glBindVertexArray(targetVAO);
    glDrawArrays(GL_POINTS, 0, 1);
    glBindVertexArray(0);
}