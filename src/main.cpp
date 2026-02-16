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
//ZMIANY 
float cameraDistance = 30.0f;
float cameraAngleX = 0.5f; // Kąt obrotu góra/dół
float cameraAngleY = 0.5f; // Kąt obrotu lewo/prawo
double lastMouseX, lastMouseY;
bool firstMouse = true;


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // Przybliżanie/oddalanie
    extern float cameraDistance;
    cameraDistance -= (float)yoffset * 2.0f;
    if (cameraDistance < 1.0f) cameraDistance = 1.0f;
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

    //kamera

    
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Particle in a magnetic field simulation", nullptr, nullptr);
    if (!window) {
        cerr << "Nie udało się utworzyć okna GLFW\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);


    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);

   
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "Nie udało się załadować GLAD\n";
        return -1;
    }

	
    Target target(0.3f);

    TargetRenderer targetRenderer;

    FieldRenderer fieldRenderer;

    AxesRenderer axesRenderer;

    ParticleRenderer particleRenderer;

    Particle particle({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, 1.0, 0.1);
    
    GuiController gui;
    gui.Init(window);

    glm::dvec3 B_field(0.0, 0.0, -1.0);
    float dt = 0.0015f;
    bool simulate = false;
    float worldHeight = 8.0f;
    bool firstGameEntry = true;

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    particleRenderer.UpdateTrajectory(particle.trajectory);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // --- 1. OBSŁUGA MYSZKI (Arcball Camera) ---
        // Obracamy kamerę tylko gdy nie klikamy w menu ImGui
        if (!ImGui::GetIO().WantCaptureMouse) {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                double mouseX, mouseY;
                glfwGetCursorPos(window, &mouseX, &mouseY);
                if (!firstMouse) {
                    cameraAngleY += (float)(mouseX - lastMouseX) * 0.01f;
                    cameraAngleX += (float)(mouseY - lastMouseY) * 0.01f;

                    // Ograniczenie obrotu góra/dół (blokada "fikołka")
                    if (cameraAngleX > 1.5f) cameraAngleX = 1.5f;
                    if (cameraAngleX < -1.5f) cameraAngleX = -1.5f;
                }
                lastMouseX = mouseX;
                lastMouseY = mouseY;
                firstMouse = false;
            }
            else {
                firstMouse = true;
            }
        }

        // --- 2. OBLICZENIA MACIERZY WIDOKU I PROJEKCJI ---
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (height == 0) height = 1;
        float aspectRatio = (float)width / (float)height;

        // Pozycja kamery wyliczona z kątów i dystansu (zoom)
        float camX = cameraDistance * cos(cameraAngleX) * sin(cameraAngleY);
        float camY = cameraDistance * sin(cameraAngleX);
        float camZ = cameraDistance * cos(cameraAngleX) * cos(cameraAngleY);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 200.0f);
        glm::mat4 view = glm::lookAt(
            glm::vec3(camX, camY, camZ),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        glm::mat4 pv = projection * view;

        // --- 3. LOGIKA GRY I FIZYKI ---
        if (appstate == AppState::GAME && firstGameEntry) {
            target.GenerateNewTarget(worldHeight, aspectRatio);
            float worldWidth = worldHeight * aspectRatio;
            particle.Reset(glm::dvec3(-worldWidth / 3.0, 0.0, 0.0), glm::dvec3(1.0, 0.0, 0.0));
            particleRenderer.ClearTrajectory();
            firstGameEntry = false;
        }

        if (simulate && appstate != AppState::MENU) {

            glm::dvec3 effectiveB = B_field;
            if (appstate == AppState::GAME && particle.GetPosition().x < 0.0) {
                effectiveB = glm::dvec3(0.0);
            }
            particle.UpdateRK4(dt, effectiveB);
            particleRenderer.UpdateTrajectory(particle.GetTrajectory());

            if (appstate == AppState::GAME && !target.isHit) {
                if (target.CheckCollision(particle)) {
                    target.isHit = true;
                    simulate = false;
                }
            }
        }

        // --- 4. RENDEROWANIE ---
        glViewport(0, 0, width, height);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (appstate == AppState::SIMULATION || appstate == AppState::GAME) {
            // Tło (pole) - rysujemy jako pierwsze (często bez testu głębi)
            fieldRenderer.Draw(B_field.z, aspectRatio, appstate == AppState::GAME);

            // Obiekty 3D
            axesRenderer.Draw(pv);
            if (appstate == AppState::GAME) {
                targetRenderer.Draw(target, pv, worldHeight);
            }
            particleRenderer.Draw(particle, pv);
        }

        // --- 5. GUI I RESET ---
        bool newGameRequested = false;
        bool retryRequested = false;
        gui.Render(appstate, simulate, B_field, dt, particle, worldHeight, target, newGameRequested, retryRequested);

        if (newGameRequested || retryRequested) {
            if (newGameRequested) target.GenerateNewTarget(worldHeight, aspectRatio);
            else target.Reset();

            float worldWidth = worldHeight * aspectRatio;
            glm::dvec3 startPos = (appstate == AppState::GAME) ? glm::dvec3(-worldWidth / 3.0, 0.0, 0.0) : glm::dvec3(0.0);
            particle.Reset(startPos, glm::dvec3(1.0, 0.0, 0.0));
            particleRenderer.ClearTrajectory();
            particleRenderer.UpdateTrajectory(particle.GetTrajectory());
            if (appstate == AppState::GAME) simulate = false;
        }

        glfwSwapBuffers(window);
    }
    return 0;
}