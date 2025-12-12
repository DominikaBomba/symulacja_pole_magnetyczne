#pragma once
#include <glm/glm.hpp>
#include "Particle.h"
#include <random>

class Target {
public:
    glm::dvec2 position;
    float radius; // Promieñ celu [m]
    bool isHit;

    Target(float r = 0.5f);

    // Losuje now¹ pozycjê w zakresie (np. +/- 3.0 m), pomijaj¹c okolice (0,0)
    void GenerateNewTarget(float worldHeight, float aspectRatio);

    // Sprawdza kolizjê z cz¹stk¹ (po³o¿enie cz¹stki w promieniu celu)
    bool CheckCollision(const Particle& particle) const;

    // Resetuje stan trafienia
    void Reset();

private:
    std::mt19937 rng; // Generator liczb losowych
};