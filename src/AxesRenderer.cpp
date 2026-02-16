#include "AxesRenderer.h"
#include <cmath>

// Shader obs³uguj¹cy 3D
const char* AXIS_VS = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos; // Zmiana na vec3
    uniform mat4 projection; // W Twoim main.cpp to teraz PV (Projection * View)
    void main() {
        gl_Position = projection * vec4(aPos, 1.0);
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
    float range = 20.0f;

    // G£ÓWNE LINIE OSI (X, Y, Z)
    // Oœ X (Czerwona w Draw)
    vertices.push_back(-range); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(range);  vertices.push_back(0.0f); vertices.push_back(0.0f);
    // Oœ Y (Zielona w Draw)
    vertices.push_back(0.0f);   vertices.push_back(-range); vertices.push_back(0.0f);
    vertices.push_back(0.0f);   vertices.push_back(range);  vertices.push_back(0.0f);
    // Oœ Z (Niebieska w Draw)
    vertices.push_back(0.0f);   vertices.push_back(0.0f); vertices.push_back(-range);
    vertices.push_back(0.0f);   vertices.push_back(0.0f); vertices.push_back(range);

    // PODZIA£KA (Tick marks)
    float stepMajor = 1.0f;
    float stepMinor = 0.5f; // Zwiêkszy³em na 0.5mm, ¿eby nie zaciemniaæ widoku w 3D

    for (float i = -range; i <= range; i += stepMinor) {
        if (std::abs(i) < 0.001f) continue;

        bool isMajor = (std::abs(i - round(i / stepMajor) * stepMajor) < 0.0001f);
        float size = isMajor ? 0.15f : 0.06f;

        // Kreski na osi X (widoczne na p³aszczyŸnie XY)
        vertices.push_back(i); vertices.push_back(-size); vertices.push_back(0.0f);
        vertices.push_back(i); vertices.push_back(size);  vertices.push_back(0.0f);

        // Kreski na osi Y (widoczne na p³aszczyŸnie XY)
        vertices.push_back(-size); vertices.push_back(i); vertices.push_back(0.0f);
        vertices.push_back(size);  vertices.push_back(i); vertices.push_back(0.0f);

        // Kreski na osi Z (widoczne na p³aszczyŸnie ZY)
        vertices.push_back(0.0f); vertices.push_back(-size); vertices.push_back(i);
        vertices.push_back(0.0f); vertices.push_back(size);  vertices.push_back(i);
    }

    vertexCount = (int)(vertices.size() / 3); // Dzielimy przez 3, bo mamy x, y, z

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Skok o 3 floaty
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void AxesRenderer::Draw(const glm::mat4& pv) { // pv to projection * view
    if (!shader) return;

    shader->Use();
    shader->SetMat4("projection", pv);

    glBindVertexArray(VAO);

    // Rysujemy g³ówne osie z ró¿nymi kolorami
    // X - Czerwona
    shader->SetVec4("color", glm::vec4(1.0f, 0.3f, 0.3f, 1.0f));
    glDrawArrays(GL_LINES, 0, 2);

    // Y - Zielona
    shader->SetVec4("color", glm::vec4(0.3f, 1.0f, 0.3f, 1.0f));
    glDrawArrays(GL_LINES, 2, 2);

    // Z - Niebieska
    shader->SetVec4("color", glm::vec4(0.3f, 0.3f, 1.0f, 1.0f));
    glDrawArrays(GL_LINES, 4, 2);

    // Rysujemy resztê podzia³ki (szara)
    shader->SetVec4("color", glm::vec4(0.7f, 0.7f, 0.7f, 0.5f));
    glDrawArrays(GL_LINES, 6, vertexCount - 6);

    glBindVertexArray(0);
}