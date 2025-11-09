#pragma once
#include <glm/glm.hpp>
#include <vector>

class Particle {
public:
    glm::dvec2 position;   // [m]
    glm::dvec2 velocity;   // [m/s]
    double charge;         // [C]
    double mass;           // [kg]
    std::vector<glm::dvec2> trajectory;

    Particle(
        const glm::dvec2& pos = glm::dvec2(0.0, 0.0),
        const glm::dvec2& vel = glm::dvec2(0.0, 0.0),
        double q = 1.0,
        double m = 1.0);

    // Si³a Lorentza: F = q * (v × B)
    glm::dvec2 LorentzForce(double Bz) const;

    // Pochodne [dx/dt, dy/dt, dvx/dt, dvy/dt]
    glm::dvec4 Derivatives(double Bz) const;

    // Aktualizacja metod¹ Runge–Kutta 4 rzêdu
    void UpdateRK4(double dt, double Bz);

    void Reset(const glm::dvec2& pos, const glm::dvec2& vel);
};
