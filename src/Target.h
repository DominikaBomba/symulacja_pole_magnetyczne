#pragma once
#include <glm/glm.hpp>
#include "Particle.h"
#include <random>

class Target {
public:
    glm::dvec3 position;
    float radius; // Promieñ celu
    bool isHit;

    Target(float r = 0.5f);

    // Losuje now¹ pozycjê celu
    void GenerateNewTarget(float worldHeight, float aspectRatio);

    // Sprawdza kolizjê z cz¹stk¹ (po³o¿enie cz¹stki w promieniu celu)
    bool CheckCollision(const Particle& particle) const;

    // Resetuje stan trafienia
    void Reset();

private:
    std::mt19937 rng; // Generator liczb losowych
};