#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <deque>
#include "Shader.h"
#include "Particle.h"

class ParticleRenderer {
public:
    ParticleRenderer();
    ~ParticleRenderer();

    // Aktualizuje bufor trajektorii 
    void UpdateTrajectory(const std::deque<glm::dvec3>& trajectory);

    // Rysuje cz¹stkê i jej œlad
    void Draw(const Particle& particle, const glm::mat4& projection);

	//czyœci bufor trajektorii i resetuje w pamiêci GPU
    void ClearTrajectory();

    GLuint GetTrajectoryVBO() const { return trajectoryVBO; }

private:
    Shader* shader;

    // Bufory dla samej cz¹stki (punktu)
    GLuint particleVAO, particleVBO;

    // Bufory dla trajektorii (ogona)
    GLuint trajectoryVAO, trajectoryVBO;

    void InitShader();
    void InitBuffers();
};
