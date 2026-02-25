#include "Particle.h"

Particle::Particle(
    const glm::dvec3& pos,
    const glm::dvec3& vel,
    double q,  // Zmieniono na double
    double m)  // Zmieniono na double
    : position(pos), velocity(vel), charge(q), mass(m)
{
    trajectory.push_back(position);
}

glm::dvec3 Particle::LorentzForce(const glm::dvec3& B) const {
    return charge * glm::cross(velocity, B);
}

Particle::State Particle::GetDerivatives(const State& s, const glm::dvec3& B) const {
  
    glm::dvec3 acceleration = (charge * glm::cross(s.vel, B)) / mass;

    // Zwracamy prędkość (zmiana pozycji) i przyspieszenie (zmiana prędkości)
    return { s.vel, acceleration };
}
void Particle::UpdateRK4(double dt, const glm::dvec3& B) {
  
  auto f = [&](const State& s) -> State {
        return GetDerivatives(s, B);
    };

    State y = { position, velocity };

    // Obliczenia współczynników k
    State k1 = f(y);
    State k2 = f(y + k1 * (0.5 * dt));
    State k3 = f(y + k2 * (0.5 * dt));
    State k4 = f(y + k3 * dt);

    // Nowy stan
    State increment = (k1 + k2 * 2.0 + k3 * 2.0 + k4) * (dt / 6.0);
    y = y + increment;

    // Przypisanie i zapis trajektorii
    position = y.pos;
    velocity = y.vel;

    // Opcjonalne: usuwanie najstarszych punktów, by nie zapchać RAMu
    if (trajectory.size() >= MAX_TRAJECTORY_SIZE) {
        for (int i = 0; i < 100; ++i) {
            trajectory.pop_front();
        }
    }
    trajectory.push_back(position);
}

void Particle::Reset(const glm::dvec3& pos, const glm::dvec3& vel) {
    position = pos;
    velocity = vel;
    trajectory.clear();
    trajectory.push_back(position);
}

void Particle::SetSpeed(double newSpeed) {
    double currentSpeed = glm::length(velocity);
    if (currentSpeed > 0.0)
        velocity = glm::normalize(velocity) * newSpeed;
    else
        velocity = glm::dvec3(newSpeed, 0.0, 0.0); // jeśli prędkość jest 0, nadaj w osi X
}
