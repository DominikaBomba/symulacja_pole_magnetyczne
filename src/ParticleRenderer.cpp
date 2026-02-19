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
    out vec4 FragColor;
    void main() { 
        FragColor = vec4(0.3, 0.3, 0.9, 1.0); 
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
    // Zmieniono na 3 floaty (x, y, z)
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 2. Trajektoria
    glGenVertexArrays(1, &trajectoryVAO);
    glGenBuffers(1, &trajectoryVBO);
    glBindVertexArray(trajectoryVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);
    // Zmieniono na 5000 punktów * 3 wspó³rzêdne
    glBufferData(GL_ARRAY_BUFFER, 5000 * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void ParticleRenderer::UpdateTrajectory(const std::vector<glm::dvec3>& trajectory) {
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
    glm::dvec3 pPos = particle.GetPosition();
    //Aktualizacja pozycji cz¹stki (co klatkê)
    float pos[3] = { (float)pPos.x, (float)pPos.y, (float)pPos.z };
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), pos);

    //Rysowanie cz¹stki
    glPointSize(10.0f);
    glBindVertexArray(particleVAO);
    glDrawArrays(GL_POINTS, 0, 1);

    //Rysowanie œladu (dane s¹ ju¿ w buforze dziêki UpdateTrajectory)
    glPointSize(2.0f);
    glBindVertexArray(trajectoryVAO);
    const auto& traj = particle.GetTrajectory();
    glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)traj.size()); // LINE_STRIP wygl¹da lepiej ni¿ POINTS

    glBindVertexArray(0);
}

void ParticleRenderer::ClearTrajectory() {
    glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);

  
    glBufferData(GL_ARRAY_BUFFER, 5000 * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}