// Particle.h
#pragma once
#include <deque> //kolejka dwukierunkowa żeby zoptymalizować usuwanie najstarszych punktów trajektorii
#include <glm/glm.hpp>

class Particle {
public:
	//dajemy trajectory size do public, żeby renderer mógł z niego korzystać
    //constexpr, bo to stała, która nie będzie się zmieniać w czasie działania programu
	static constexpr size_t ABSOLUTE_MAX_TRAJECTORY = 50000; 

    std::deque<glm::dvec3> trajectory;
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
    const std::deque<glm::dvec3>& GetTrajectory() const { return trajectory; }

    double GetCharge() const { return charge; }
    void SetCharge(double q) { charge = q; }

    double GetMass() const { return mass; }
    void SetMass(double m) {
        if (m > 0.0) mass = m;
    }

    size_t GetMaxTrajectorySize() const { return maxTrajectorySize; }
    void SetMaxTrajectorySize(size_t newSize);

private:
    glm::dvec3 position;
    glm::dvec3 velocity;
    double charge;
    double mass;

	size_t maxTrajectorySize = 5000; // Domyślna maksymalna długość trajektorii, można ją zmieniać przez GUI
};