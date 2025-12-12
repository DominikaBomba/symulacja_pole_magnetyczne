#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Particle.h"
#include <glm/glm.hpp>
// glm::ortho - transformacje przestrzeni aby nie zniekształcać obrazu
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cmath>
#include <random> 

// Nagłówki
#include "Shader.h"
#include "FieldRenderer.h"
#include "AxesRenderer.h"
#include "ParticleRenderer.h"
#include "GuiController.h"

using namespace std;


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    if (!glfwInit()) {
        cerr << "Inicjacja GLFW się nie udała\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    AppState appstate = AppState::MENU;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Symulacja cząstki w polu magnetycznym", nullptr, nullptr);
    if (!window) {
        cerr << "Nie udało się utworzyć okna GLFW\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "Nie udało się załadować GLAD\n";
        return -1;
    }

    // --- INICJALIZACJA ZASOBÓW (REFRACTOR) ---

    // Tło (klasa FieldRenderer)
    FieldRenderer fieldRenderer;

    // Osie (klasa AxesRenderer)
    AxesRenderer axesRenderer;

	// Cząstka (klasa ParticleRenderer)
    ParticleRenderer particleRenderer;

    GuiController gui;      // <--- Tworzymy kontroler
    gui.Init(window);       // <--- Inicjalizujemy ImGui

    Particle particle({ 0.0, 0.0 }, { 1.0, 0.0 }, 1.0, 0.1);
    float Bz = -1.0f;
    float dt = 0.00025f;
    bool simulate = false;

	float worldHeight = 8.0f; // Wysokość świata w metrach (do zoomu)

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // ----------------------------------------------------------
        // Aktualizacja cząstki
        // ----------------------------------------------------------
        if (simulate && appstate != AppState::MENU) {
            particle.UpdateRK4(dt, Bz);

            if (particle.trajectory.size() > Particle::MAX_TRAJECTORY_SIZE)
                particle.trajectory.erase(particle.trajectory.begin());

            particleRenderer.UpdateTrajectory(particle.trajectory);
        }

        // ----------------------------------------------------------
        // Renderowanie
        // ----------------------------------------------------------

        // rysowanie tła
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

		// obliczanie macierzy projekcji (kamery) aby uniknąć zniekształceń obrazu

        // aktualne wymiary okna
        int currentW, currentH;
        glfwGetFramebufferSize(window, &currentW, &currentH);

        // Zabezpieczenie jeśli zminimalizujemy okno
        if (currentH == 0) currentH = 1;

        // proporcje ekranu
        float aspectRatio = (float)currentW / (float)currentH;

        float worldWidth = worldHeight * aspectRatio; //szerokość dostosowana do proporcji ekranu

        // macierz projekcji ortogonalnej (2D)
        glm::mat4 projection = glm::ortho(
            -worldWidth / 2.0f, worldWidth / 2.0f,  // Oś X (dynamiczna szerokość)
            -worldHeight / 2.0f, worldHeight / 2.0f, // Oś Y (stała wysokość)
            -1.0f, 1.0f //nie jest istotne w 2D
        );

        if (appstate == AppState::SIMULATION || appstate == AppState::GAME) {
            fieldRenderer.Draw(Bz, aspectRatio);
			axesRenderer.Draw(projection);
            particleRenderer.Draw(particle, projection);
        }


        // 3. Rysowanie GUI i obsługa RESETU
        bool shouldReset = gui.Render(appstate, simulate, Bz, dt, particle, worldHeight, 0);
        // Przekazujemy 0 jako ostatni parametr, bo GUI już nie czyści bufora bezpośrednio.
        if (shouldReset) {
            // 1. Fizyka: Czyścimy wektor trajektorii w pamięci RAM
            particle.trajectory.clear();

            // Dodajemy punkt startowy (0,0), żeby wektor nie był pusty
            particle.trajectory.push_back(particle.position);

            // 2. Grafika: "Sierocimy" stary bufor (wyrzucamy śmieci z pamięci karty)
            particleRenderer.ClearTrajectory();

            // 3. Aktualizujemy GPU tym jednym, czystym punktem startowym
            particleRenderer.UpdateTrajectory(particle.trajectory);
        }

        glfwSwapBuffers(window);
    }
    return 0;
}