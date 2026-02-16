// Particle.h
#pragma once
#include <vector>
#include <glm/glm.hpp>

class Particle {
public:

    std::vector<glm::dvec3> trajectory;
    // Nasz nowy "kontener" na 3D
    struct State {
        glm::dvec3 pos;
        glm::dvec3 vel;

        // Przeciążenia operatorów, żeby RK4 w .cpp wyglądało jak matematyka
        State operator+(const State& other) const { return { pos + other.pos, vel + other.vel }; }
        State operator*(double dt) const { return { pos * dt, vel * dt }; }
    };
    glm::dvec3 GetVelocity() const { return velocity; }
    Particle(const glm::dvec3& pos, const glm::dvec3& vel, double q, double m);
    glm::dvec3 LorentzForce(const glm::dvec3& B) const;
    State GetDerivatives(const State& s, const glm::dvec3& B) const;
    void UpdateRK4(double dt, const glm::dvec3& B);
    void Reset(const glm::dvec3& pos, const glm::dvec3& vel);
    void SetSpeed(double newSpeed);

    // Gettery
    glm::dvec3 GetPosition() const { return position; }
    const std::vector<glm::dvec3>& GetTrajectory() const { return trajectory; }

private:
    glm::dvec3 position;
    glm::dvec3 velocity;
    double charge;
    double mass;
    static const size_t MAX_TRAJECTORY_SIZE = 5000;
};