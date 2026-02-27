#include "ParticleRenderer.h"
#include <glm/gtc/type_ptr.hpp>

// Shader cz¹stki (Vertex)
const char* PARTICLE_VS = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    
    uniform mat4 projection;

    void main() { 
        gl_Position = projection * vec4(aPos, 1.0); 
    }
)";

// Shader cz¹stki (Fragment) (Niebieski kolor)
const char* PARTICLE_FS = R"(
    #version 330 core
    uniform vec3 color; // ZMIANA: Dodajemy uniform na kolor
    out vec4 FragColor;
    void main() { 
        FragColor = vec4(color, 1.0); 
    }
)";

ParticleRenderer::ParticleRenderer() {
    InitShader();
    InitBuffers();
}

ParticleRenderer::~ParticleRenderer() {
    glDeleteVertexArrays(1, &particleVAO);
    glDeleteBuffers(1, &particleVBO);
    glDeleteVertexArrays(1, &trajectoryVAO);
    glDeleteBuffers(1, &trajectoryVBO);
    delete shader;
}

void ParticleRenderer::InitShader() {
    shader = new Shader(PARTICLE_VS, PARTICLE_FS);
}
void ParticleRenderer::InitBuffers() {
    // 1. Cz¹stka
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);
    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    // Zmieniono na 6 floaty (x, y, z) - pozycja cz¹steczki oraz pozycja "cienia" na pod³odze
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 2. Trajektoria
    glGenVertexArrays(1, &trajectoryVAO);
    glGenBuffers(1, &trajectoryVBO);
    glBindVertexArray(trajectoryVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);
    // Zmieniono na 5000 punktów * 3 wspó³rzêdne
    glBufferData(GL_ARRAY_BUFFER, Particle::ABSOLUTE_MAX_TRAJECTORY * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void ParticleRenderer::UpdateTrajectory(const std::deque<glm::dvec3>& trajectory) {
    std::vector<float> points;
    points.reserve(trajectory.size() * 3);
    for (const auto& p : trajectory) {
        points.push_back((float)p.x);
        points.push_back((float)p.y);
        points.push_back((float)p.z);
    }
    glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(float), points.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleRenderer::Draw(const Particle& particle, const glm::mat4& projection, bool drawInitialDir, const glm::vec3& initialDir) {
    if (!shader) return;

    shader->Use();
    shader->SetMat4("projection", projection);

    glm::dvec3 pPos = particle.GetPosition();

    // Obliczamy koniec ró¿owej linii (kierunek startowy "wyrzutni")
    glm::vec3 dirEnd = glm::vec3(0.0f);
    if (glm::length(initialDir) > 0.0f) {
        dirEnd = glm::normalize(initialDir) * 2.0f; // D³ugoœæ wskaŸnika to 2.0
    }

    // Pakujemy 4 punkty do bufora
    float pos[12] = {
        (float)pPos.x, (float)pPos.y, (float)pPos.z,        // Punkt 0: Cz¹stka
        (float)pPos.x, -6.0f, (float)pPos.z,                // Punkt 1: Cieñ na pod³odze (upewnij siê, ¿e masz tu swoje -6.0f)
        0.0f, 0.0f, 0.0f,                                   // Punkt 2: Pocz¹tek ró¿owej linii (ŒRODEK UK£ADU)
        dirEnd.x, dirEnd.y, dirEnd.z                        // Punkt 3: Koniec ró¿owej linii
    };

    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), pos);

    // 1. Rysowanie Cz¹stki (Punkt 0)
    glPointSize(10.0f);
    glBindVertexArray(particleVAO);
    glUniform3f(glGetUniformLocation(shader->ID, "color"), 0.3f, 0.3f, 0.9f); // Niebieski
    glDrawArrays(GL_POINTS, 0, 1);

    // 2. Rysowanie Linii Rzutowania na pod³ogê (Punkty 0 i 1)
    glUniform3f(glGetUniformLocation(shader->ID, "color"), 0.5f, 0.5f, 0.5f); // Szary
    glDrawArrays(GL_LINES, 0, 2);

    // 3. Rysowanie Linii Kierunku "Wyrzutni" (Punkty 2 i 3) 
    if (drawInitialDir) {
        //Wy³¹czamy test g³êbokoœci. Linia narysuje siê na wierzchu osi.
        glDisable(GL_DEPTH_TEST);

        //Pogrubiamy liniê
        glLineWidth(2.0f);

        glUniform3f(glGetUniformLocation(shader->ID, "color"), 1.0f, 0.4f, 0.8f); // Ró¿owy 
        glDrawArrays(GL_LINES, 2, 2);

        //Przywracamy standardowe ustawienia dla reszty obiektów
        glLineWidth(1.0f);
        glEnable(GL_DEPTH_TEST);
    }

    // 4. Rysowanie œladu trajektorii
    glPointSize(2.0f);
    glBindVertexArray(trajectoryVAO);
    const auto& traj = particle.GetTrajectory();
    glUniform3f(glGetUniformLocation(shader->ID, "color"), 0.8f, 0.8f, 0.2f); // ¯ó³tawy œlad
    glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)traj.size());

    glBindVertexArray(0);
}

void ParticleRenderer::ClearTrajectory() {
    glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);

  
    glBufferData(GL_ARRAY_BUFFER, Particle::ABSOLUTE_MAX_TRAJECTORY * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}