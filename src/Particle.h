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
        double m = 1.0)
        : position(pos), velocity(vel), charge(q), mass(m)
    {
        trajectory.push_back(position);
    }

    // Si³a Lorentza: F = q * (v × B)
    glm::dvec2 LorentzForce(double Bz) const {
        double Fx = charge * velocity.y * Bz;
        double Fy = -charge * velocity.x * Bz;
        return glm::dvec2(Fx, Fy);
    }

    // Pochodne [dx/dt, dy/dt, dvx/dt, dvy/dt]
    glm::dvec4 Derivatives(double Bz, double masa) const {
        glm::dvec2 F = LorentzForce(Bz);
        return glm::dvec4(velocity.x, velocity.y, F.x / masa, F.y / masa);
    }

    // Aktualizacja metod¹ Runge–Kutta 4 rzêdu
    void UpdateRK4(double dt, double Bz, double masa) {
        auto f = [&](const glm::dvec4& s) -> glm::dvec4 {
            glm::dvec2 v(s.z, s.w);
            glm::dvec2 F(charge * v.y * Bz, -charge * v.x * Bz);
            return glm::dvec4(v.x, v.y, F.x / masa, F.y / masa);
            };

        glm::dvec4 y(position.x, position.y, velocity.x, velocity.y);
        glm::dvec4 k1 = f(y);
        glm::dvec4 k2 = f(y + glm::dvec4(0.5 * dt * k1.x, 0.5 * dt * k1.y, 0.5 * dt * k1.z, 0.5 * dt * k1.w));
        glm::dvec4 k3 = f(y + glm::dvec4(0.5 * dt * k2.x, 0.5 * dt * k2.y, 0.5 * dt * k2.z, 0.5 * dt * k2.w));
        glm::dvec4 k4 = f(y + glm::dvec4(dt * k3.x, dt * k3.y, dt * k3.z, dt * k3.w));

        // y += (dt / 6.0) * (k1 + 2*k2 + 2*k3 + k4);
        glm::dvec4 increment = (dt / 6.0) * glm::dvec4(
            k1.x + 2.0 * k2.x + 2.0 * k3.x + k4.x,
            k1.y + 2.0 * k2.y + 2.0 * k3.y + k4.y,
            k1.z + 2.0 * k2.z + 2.0 * k3.z + k4.z,
            k1.w + 2.0 * k2.w + 2.0 * k3.w + k4.w
        );
        y += increment;

        position.x = y.x;
        position.y = y.y;
        velocity.x = y.z;
        velocity.y = y.w;

        trajectory.push_back(position);
    }

    void Reset(const glm::dvec2& pos, const glm::dvec2& vel) {
        position = pos;
        velocity = vel;
        trajectory.clear();
        trajectory.push_back(position);
    }
};
