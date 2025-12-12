#include "Target.h"
#include <iostream>
#include <ctime>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp>

Target::Target(float r) : radius(r), isHit(false) {
    // Inicjalizacja generatora losowego (raz)
    rng.seed(static_cast<unsigned int>(std::time(0)));
}

void Target::GenerateNewTarget(float worldHeight, float aspectRatio) {
    // 1. Definicja rozs¹dnego obszaru losowania
    // U¿ywamy worldHeight do dynamicznego skalowania. Cel powinien byæ
    // w polu widzenia, ale poza punktem startu (0,0).

    float worldWidth = worldHeight * aspectRatio;

    // Ustawiamy maksymalny i minimalny dystans od centrum
    // Min_dist: aby nie kolidowa³ z cz¹stk¹ w punkcie startowym.
    float min_dist_from_center = 2.0f; // minimalnie 2.0 metra od (0,0)
    float max_range = (std::min(worldWidth, worldHeight) / 2.0f) * 0.8f; // Maksymalnie 80% widocznego obszaru

    if (max_range < min_dist_from_center) {
        max_range = min_dist_from_center + 1.0f;
    }

    std::uniform_real_distribution<double> dist_angle(0.0, 2.0 * glm::pi<double>());
    std::uniform_real_distribution<double> dist_radius(min_dist_from_center, max_range);

    // Losowanie w biegunowym uk³adzie wspó³rzêdnych, ¿eby unikn¹æ (0,0)
    double r = dist_radius(rng);
    double angle = dist_angle(rng);

    position.x = r * std::cos(angle);
    position.y = r * std::sin(angle);

    isHit = false;

    std::cout << "Nowy Cel: (" << position.x << ", " << position.y << "), Promien: " << radius << std::endl;
}

bool Target::CheckCollision(const Particle& particle) const {
    if (isHit) return true; // Ju¿ trafiony

    // Obliczamy kwadrat odleg³oœci (szybsze ni¿ liczenie pierwiastka)
    double dx = particle.position.x - position.x;
    double dy = particle.position.y - position.y;
    double distanceSq = dx * dx + dy * dy;

    // Zak³adamy, ¿e promieñ cz¹stki jest zaniedbywalnie ma³y (punkt).
    // U¿ywamy tylko promienia celu.
    double radiusSq = radius * radius;

    if (distanceSq <= radiusSq) {
        // Kolizja!
        return true;
    }
    return false;
}

void Target::Reset() {
    isHit = false;
}