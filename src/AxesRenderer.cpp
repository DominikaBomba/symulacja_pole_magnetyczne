#include "AxesRenderer.h"
#include <cmath>

// shader dla linii
const char* AXIS_VS = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * vec4(aPos, 0.0, 1.0);
    }
)";

const char* AXIS_FS = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec4 color;
    void main() {
        FragColor = color;
    }
)";

AxesRenderer::AxesRenderer() {
    InitShader();
    InitData();
}

AxesRenderer::~AxesRenderer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    delete shader;
}

void AxesRenderer::InitShader() {
    shader = new Shader(AXIS_VS, AXIS_FS);
}

void AxesRenderer::InitData() {
    std::vector<float> vertices;

    // Zakres rysowania osi (od -20mm do +20mm)
    float range = 20.0f;

    // 1. G£ÓWNE OSIE (X i Y)
    // Oœ X
    vertices.push_back(-range); vertices.push_back(0.0f);
    vertices.push_back(range);  vertices.push_back(0.0f);
    // Oœ Y
    vertices.push_back(0.0f);   vertices.push_back(-range);
    vertices.push_back(0.0f);   vertices.push_back(range);

    // 2. PODZIA£KA (TICKS)
    // SKALA: 1.0 jednostka OpenGL = 1.0 mm fizycznie
    float stepMajor = 1.0f;  // Du¿a kreska co 1 mm
    float stepMinor = 0.1f;  // Ma³a kreska co 0.1 mm

    for (float i = -range; i <= range; i += stepMinor) {
        if (std::abs(i) < 0.001f) continue; // Pomiñ œrodek

        // Sprawdzamy czy to pe³ny milimetr
        bool isMajor = (std::abs(i - round(i / stepMajor) * stepMajor) < 0.0001f);

        float size = isMajor ? 0.1f : 0.04f;

        // Oœ X (pionowe kreseczki)
        vertices.push_back(i); vertices.push_back(-size);
        vertices.push_back(i); vertices.push_back(size);

        // Oœ Y (poziome kreseczki)
        vertices.push_back(-size); vertices.push_back(i);
        vertices.push_back(size);  vertices.push_back(i);
    }

    vertexCount = vertices.size() / 2;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void AxesRenderer::Draw(const glm::mat4& projection) {
    if (shader) {
        shader->Use();
        shader->SetMat4("projection", projection);

        // Ustawiamy kolor (jasnoszary)
        int colorLoc = glGetUniformLocation(shader->ID, "color");
        glUniform4f(colorLoc, 0.7f, 0.7f, 0.7f, 0.5f);

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, vertexCount);
        glBindVertexArray(0);
    }
}