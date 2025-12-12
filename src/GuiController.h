#pragma once
#include <glad/glad.h> // Potrzebne dla GLuint (trajectoryVBO)
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Particle.h"
#include <vector>
#include <cmath>

// Przenosimy enum tutaj, aby by³ dostêpny dla Main i GuiController
enum AppState {
    MENU, SIMULATION, GAME
};

class GuiController {
public:
    GuiController();
    ~GuiController();

    void Init(GLFWwindow* window);
    void Shutdown();

    // G³ówna pêtla rysuj¹ca interfejs.
    // Przyjmujemy referencje do zmiennych, które GUI mo¿e modyfikowaæ.
    // trajectoryVBO przekazujemy po to, aby przycisk Reset móg³ wyczyœciæ bufor graficzny.
    bool Render(AppState& appstate, bool& simulate, float& Bz, float& dt,
        Particle& particle, float& worldHeight, GLuint trajectoryVBO);
};