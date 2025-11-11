#pragma once
#include <glm/glm.hpp>
#include <vector>

class Particle {
public:
    glm::dvec2 position;   // [m]
    glm::dvec2 velocity;   // [m/s]
    float charge;         // [C]
    float mass;           // [kg]
    std::vector<glm::dvec2> trajectory;

    Particle(
        const glm::dvec2& pos = glm::dvec2(0.0, 0.0),
        const glm::dvec2& vel = glm::dvec2(0.0, 0.0),
        float q = 1.0,
        float m = 1.0);

    // Si�a Lorentza: F = q * (v � B)
    glm::dvec2 LorentzForce(float Bz) const;

    // Pochodne [dx/dt, dy/dt, dvx/dt, dvy/dt]
    glm::dvec4 Derivatives(float Bz) const;

    // Aktualizacja metod� Runge�Kutta 4 rz�du
    void UpdateRK4(float dt, float Bz);

    void Reset(const glm::dvec2& pos, const glm::dvec2& vel);

    void SetSpeed(double newSpeed);

};