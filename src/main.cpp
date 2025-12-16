#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Particle.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cmath>
#include <random> 
#include "TargetRenderer.h"
#include "Target.h"

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

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Particle in a magnetic field simulation", nullptr, nullptr);
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

	// Inicjalizacja zasobów renderujących

    // Cel o promieniu 0.3 mm
    Target target(0.3f);

    //Cel (w trybie gry)
    TargetRenderer targetRenderer;

    // Tło (pole magnetyczne)
    FieldRenderer fieldRenderer;

    // Osie układu współrzędnych
    AxesRenderer axesRenderer;

    // Cząsteczka
    ParticleRenderer particleRenderer;

    //Inicjalizacja ImGui
    GuiController gui;
    gui.Init(window);

    //Cząsteczka
    Particle particle({ 0.0, 0.0 }, { 1.0, 0.0 }, 1.0, 0.1);
    float Bz = -1.0f;
    float dt = 0.0015f;
    bool simulate = false;

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Wysokość świata w mm
    float worldHeight = 8.0f; 

    // Zmienna do obsługi pierwszego wejścia do trybu gry
    bool firstGameEntry = true;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Logika inicjalizacji przy pierwszym wejściu do gry
        if (appstate == AppState::GAME && firstGameEntry) {
            int w, h;
            glfwGetFramebufferSize(window, &w, &h);
            float ratio = (h > 0) ? (float)w / h : 1.0f;

            // Losujemy cel
            target.GenerateNewTarget(worldHeight, ratio);

            // Reset pozycji startowej cząsteczki
            float worldWidth = worldHeight * ratio;
            particle.Reset(glm::dvec2(-worldWidth / 3.0f, 0.0), glm::dvec2(1.0, 0.0));
            particle.SetSpeed(1.0); // Reset domyślnej prędkości

            firstGameEntry = false;
        }

        
        //Logika ruchu cząsteczki
        if (simulate && appstate != AppState::MENU) {
            // Logika pola magnetycznego w trybie gry (pole występuje tylko po prawej stronie ekranu)
            float effectiveB = Bz;
            if (appstate == AppState::GAME) {
                if (particle.position.x < 0.0f) {
                    effectiveB = 0.0f;
                }
            }

            particle.UpdateRK4(dt, effectiveB);

            if (particle.trajectory.size() > Particle::MAX_TRAJECTORY_SIZE)
                particle.trajectory.erase(particle.trajectory.begin());

            particleRenderer.UpdateTrajectory(particle.trajectory);

            //logika kolizji
            if (appstate == AppState::GAME && !target.isHit) {
                if (target.CheckCollision(particle)) {
                    target.isHit = true;
                    simulate = false;
                    std::cout << "TRAFIONY! W " << particle.position.x << ", " << particle.position.y << std::endl;
                }
            }
        }

        //renderowanie

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

        //szerokość dostosowana do proporcji ekranu
    	float worldWidth = worldHeight * aspectRatio;

        // macierz projekcji ortogonalnej (2D)
        glm::mat4 projection = glm::ortho(
            -worldWidth / 2.0f, worldWidth / 2.0f,  // Oś X (dynamiczna szerokość)
            -worldHeight / 2.0f, worldHeight / 2.0f, // Oś Y (stała wysokość)
            -1.0f, 1.0f //nie jest istotne w 2D
        );

        //renderowanie elementów symulacji w zależności od trybu aplikacji
        if (appstate == AppState::SIMULATION || appstate == AppState::GAME) {
            fieldRenderer.Draw(Bz, aspectRatio, appstate == AppState::GAME);
            axesRenderer.Draw(projection);
            if (appstate == AppState::GAME) {
                targetRenderer.Draw(target, projection, worldHeight);
            }
            particleRenderer.Draw(particle, projection);
        }


        //Rysowanie GUI i obsługa RESETU
        bool newGameRequested = false;
        bool retryRequested = false;

        gui.Render(appstate, simulate, Bz, dt, particle, worldHeight, target, newGameRequested, retryRequested);

        bool performReset = false;

        if (newGameRequested) {
            target.GenerateNewTarget(worldHeight, aspectRatio); // Losowanie nowego celu
            performReset = true;
        }

        if (retryRequested) {
            target.Reset(); // Tylko reset trafienia, cel bez zmian
            performReset = true;
        }

        // Wspólna logika resetu (czyszczenie buforów i ustawienie pozycji)
        if (performReset) {
            //Ustalenie pozycji startowej
            glm::dvec2 startPos(0.0, 0.0);
            glm::dvec2 startVel = particle.velocity;

            if (appstate == AppState::GAME) {
                // Pozycja startowa w grze (po lewej stronie)
                startPos = glm::dvec2(-worldWidth / 3.0f, 0.0);

                // Prędkość zawsze w prawo (v, 0)
                double speed = glm::length(particle.velocity);
                if (speed == 0) speed = 1.0;
                startVel = glm::dvec2(speed, 0.0);
            }
            else {
                // Pozycja startowa w symulacji (na środku)
                startPos = glm::dvec2(0.0, 0.0);
                double speed = glm::length(particle.velocity);
                if (speed == 0) speed = 1.0;
                startVel = glm::dvec2(speed, 0.0);
            }

            //czyszczenie pamięci
            particle.Reset(startPos, startVel);
            particleRenderer.ClearTrajectory();
            particleRenderer.UpdateTrajectory(particle.trajectory);

            // W trybie gry po resecie czekamy na "Strzał cząsteczką"
            if (appstate == AppState::GAME) {
                simulate = false;
            }
        }
        glfwSwapBuffers(window);
    }
    return 0;
}