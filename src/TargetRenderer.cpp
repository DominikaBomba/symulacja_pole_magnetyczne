#include "TargetRenderer.h"
#include <vector>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>

// Shader Wierzcho≥kÛw z obs≥ugπ oúwietlenia (Normalne) i Macierzy Modelu
const char* TARGET_VS = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    
    uniform mat4 pv;
    uniform mat4 model;
    
    out vec3 Normal;
    
    void main() { 
        // Wektor normalny - dla bardzo prostych transformacji to po prostu u≥oøenie powierzchni
        Normal = mat3(transpose(inverse(model))) * aNormal;  
        gl_Position = pv * model * vec4(aPos, 1.0); 
    }
)";

// Shader FragmentÛw ze sztucznym úwiat≥em dodajπcym g≥Íbi
const char* TARGET_FS = R"(
    #version 330 core
    out vec4 FragColor;
    
    in vec3 Normal;
    uniform vec3 objectColor;
    
    void main() { 
        // èrÛd≥o úwiat≥a padajπce "z prawej gÛry"
        vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0)); 
        vec3 norm = normalize(Normal);
        
        // Iloczyn skalarny okreúla, jak mocno úwiat≥o uderza w danπ úcianÍ sfery
        float diff = max(dot(norm, lightDir), 0.2); // 0.2 to jasnoúÊ cienia (ambient)
        vec3 result = objectColor * diff;
        
        FragColor = vec4(result, 1.0); 
    }
)";

TargetRenderer::TargetRenderer() {
    InitShader();
    InitBuffers();
}

TargetRenderer::~TargetRenderer() {
    glDeleteVertexArrays(1, &targetVAO);
    glDeleteBuffers(1, &targetVBO);
    glDeleteBuffers(1, &targetEBO);
    delete shader;
}

void TargetRenderer::InitShader() {
    shader = new Shader(TARGET_VS, TARGET_FS);
}

void TargetRenderer::InitBuffers() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Parametry siatki sferycznej (im wiÍcej, tym g≥adsza kula)
    int sectorCount = 30;
    int stackCount = 30;
    float radius = 1.0f; // Tworzymy sferÍ jednostkowπ

    // 1. Obliczanie wierzcho≥kÛw
    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stackCount;
        float xy = radius * cos(stackAngle);
        float z = radius * sin(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * glm::pi<float>() / sectorCount;

            float x = xy * cos(sectorAngle);
            float y = xy * sin(sectorAngle);

            // WspÛ≥rzÍdne POZYCJI (3 floaty)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // WspÛ≥rzÍdne NORMALNYCH do oúwietlenia (dla sfery to po prostu pozycja / promieÒ)
            vertices.push_back(x / radius);
            vertices.push_back(y / radius);
            vertices.push_back(z / radius);
        }
    }

    // 2. £πczenie wierzcho≥kÛw w trÛjkπty
    for (int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    indexCount = indices.size();

    // 3. Wys≥anie danych do karty graficznej
    glGenVertexArrays(1, &targetVAO);
    glGenBuffers(1, &targetVBO);
    glGenBuffers(1, &targetEBO);

    glBindVertexArray(targetVAO);

    glBindBuffer(GL_ARRAY_BUFFER, targetVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, targetEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Atrybut pozycji
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atrybut wektora normalnego
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void TargetRenderer::Draw(const Target& target, const glm::mat4& pv, float worldHeight) {
    if (!shader) return;

    shader->Use();
    shader->SetMat4("pv", pv);

    // OBLICZANIE MACIERZY MODELU - Skalujemy sferÍ jednostkowπ do odpowiedniego promienia 
    // i przesuwamy w wylosowane miejsce
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(target.position));
    model = glm::scale(model, glm::vec3(target.radius));
    shader->SetMat4("model", model);

    // Dynamiczna zmiana koloru celu
    if (target.isHit) {
        glUniform3f(glGetUniformLocation(shader->ID, "objectColor"), 0.3f, 0.9f, 0.3f); // Jasnozielony
    }
    else {
        glUniform3f(glGetUniformLocation(shader->ID, "objectColor"), 0.9f, 0.3f, 0.3f); // Czerwony
    }

    // Rysujemy po≥πczone trÛjkπty
    glBindVertexArray(targetVAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}