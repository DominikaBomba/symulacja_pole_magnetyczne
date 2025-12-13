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
	// w polu widzenia (jedynie po prawej stronie ekranu w trybie gry).

    float worldWidth = worldHeight * aspectRatio;

    float minX = 0.0f;
    float maxX = (worldWidth / 2.0f);

    // Y: od do³u do góry minus margines
    float maxY = (worldHeight / 2.0f);
    float minY = -maxY;

    // Zabezpieczenie gdyby ekran by³ bardzo ma³y
    if (maxX < minX) maxX = minX;

    std::uniform_real_distribution<double> dist_x(minX, maxX);
    std::uniform_real_distribution<double> dist_y(minY, maxY);

    position.x = dist_x(rng);
    position.y = dist_y(rng);

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