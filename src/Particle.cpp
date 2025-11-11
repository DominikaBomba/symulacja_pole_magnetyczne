#include "Particle.h"

Particle::Particle(
    const glm::dvec2 & pos,
    const glm::dvec2 & vel,
    float q,
    float m)
    : position(pos), velocity(vel), charge(q), mass(m)
{
    trajectory.reserve(1024);
    trajectory.push_back(position);
}

glm::dvec2 Particle::LorentzForce(float Bz) const {
    float Fx = charge * velocity.y * Bz;
    float Fy = -charge * velocity.x * Bz;
    return glm::dvec2(Fx, Fy);
}

glm::dvec4 Particle::Derivatives(float Bz) const {
    glm::dvec2 F = LorentzForce(Bz);
    return glm::dvec4(velocity.x, velocity.y, F.x / mass, F.y / mass);
}

void Particle::UpdateRK4(float dt, float Bz) {
    auto f = [&](const glm::dvec4& s) -> glm::dvec4 {
        glm::dvec2 v(s.z, s.w);
        glm::dvec2 F(charge * v.y * Bz, -charge * v.x * Bz);
        return glm::dvec4(v.x, v.y, F.x / mass, F.y / mass);
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

void Particle::Reset(const glm::dvec2& pos, const glm::dvec2& vel) {
    position = pos;
    velocity = vel;
    trajectory.clear();
    //kilka wst�pnych punkt�w w trajektorii �eby po resecie nie by�o anomalii 
    for (int i = 0; i < 10; ++i) {
        trajectory.push_back(position);
    }
}
void Particle::SetSpeed(double newSpeed) {
    double currentSpeed = glm::length(velocity);
    if (currentSpeed > 0.0)
        velocity = glm::normalize(velocity) * newSpeed;
    else
        velocity = glm::dvec2(newSpeed, 0.0); // jeśli prędkość była 0, nadaj w osi X
}
