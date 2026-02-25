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
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 2. Trajektoria
    glGenVertexArrays(1, &trajectoryVAO);
    glGenBuffers(1, &trajectoryVBO);
    glBindVertexArray(trajectoryVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);
    // Zmieniono na 5000 punktów * 3 wspó³rzêdne
    glBufferData(GL_ARRAY_BUFFER, Particle::MAX_TRAJECTORY_SIZE * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
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

void ParticleRenderer::Draw(const Particle& particle, const glm::mat4& projection) {
    if (!shader) return;

    shader->Use();
    shader->SetMat4("projection", projection);

    // ZMIANA: Zapisujemy dwa punkty do bufora! 1 - Cz¹stka, 2 - Jej cieñ na rzêdnej y = -3.0
    glm::dvec3 pPos = particle.GetPosition();
    float pos[6] = {
        (float)pPos.x, (float)pPos.y, (float)pPos.z,
        (float)pPos.x, -6.0f, (float)pPos.z
    };
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), pos);

    // Rysowanie 1 punktu (Cz¹stka)
    glPointSize(10.0f);
    glBindVertexArray(particleVAO);
    // U¿ywamy natywnej funkcji OpenGL do wys³ania uniformu, by nie musieæ modyfikowaæ Twojego Shader.h
    glUniform3f(glGetUniformLocation(shader->ID, "color"), 0.3f, 0.3f, 0.9f); // Niebieski
    glDrawArrays(GL_POINTS, 0, 1);

    // Rysowanie "Drop Line" - kreski na pod³ogê
    glUniform3f(glGetUniformLocation(shader->ID, "color"), 0.5f, 0.5f, 0.5f); // Szary
    glDrawArrays(GL_LINES, 0, 2);

    // Rysowanie œladu trajektorii
    glPointSize(2.0f);
    glBindVertexArray(trajectoryVAO);
    const auto& traj = particle.GetTrajectory();
    glUniform3f(glGetUniformLocation(shader->ID, "color"), 0.8f, 0.8f, 0.2f); // ¯ó³tawy œlad
    glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)traj.size());

    glBindVertexArray(0);
}

void ParticleRenderer::ClearTrajectory() {
    glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);

  
    glBufferData(GL_ARRAY_BUFFER, Particle::MAX_TRAJECTORY_SIZE * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}