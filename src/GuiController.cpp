#include "GuiController.h"
#include <glm/glm.hpp>

GuiController::GuiController() {}

GuiController::~GuiController() {
    Shutdown();
}

void GuiController::Init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void GuiController::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GuiController::Render(AppState& appstate, bool& simulate, glm::dvec3& B, float& dt,
    Particle& particle, float& worldHeight, Target& target,
    bool& newGameRequested, bool& retryRequested) {

    newGameRequested = false;
    retryRequested = false;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (appstate == AppState::MENU) {
        ImGui::Begin("MENU");
        ImGui::Text("Select app mode");

        if (ImGui::Button("GAME", ImVec2(200, 40))) {
            appstate = AppState::GAME;
            simulate = false;
            retryRequested = true;
        }
        if (ImGui::Button("SIMULATION", ImVec2(200, 40))) {
            appstate = AppState::SIMULATION;
            simulate = false;
            retryRequested = true;
        }
        ImGui::End();
    }

    // Wspólna logika dla SIMULATION i GAME (z lekkimi ró¿nicami)
    if (appstate == AppState::SIMULATION || appstate == AppState::GAME) {

        ImGui::Begin(appstate == AppState::SIMULATION ? "Simulation Control" : "Game Mode");

        if (appstate == AppState::GAME && target.isHit) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "TARGET HIT!");
            ImGui::Separator();
        }

        bool inputLocked = (appstate == AppState::GAME && (simulate || target.isHit));

        if (inputLocked) {
            ImGui::TextDisabled("Controls locked during flight...");
        }
        else {
            ImGui::Text("Physics parameters");
            ImGui::Separator();

            // Pole Magnetyczne - teraz jako 3 komponenty
            ImGui::Text("Magnetic field B [T]");
            float b_float[3] = { (float)B.x, (float)B.y, (float)B.z };
            if (ImGui::SliderFloat3("##B", b_float, -2.0f, 2.0f)) {
                B.x = b_float[0]; B.y = b_float[1]; B.z = b_float[2];
            }

            // Masa i ³adunek (u¿ywamy zmiennych pomocniczych, bo pola w Particle s¹ private)
            ImGui::Separator();
            static float m = 0.1f;
            static float q = 1.0f;

            ImGui::Text("Mass (m) [x10^-25 kg]");
            if (ImGui::SliderFloat("##m", &m, 0.1f, 10.0f)) {
                // Tutaj przyda³by siê setter w Particle, np. particle.SetMass(m);
            }

            ImGui::Text("Charge (q) [x10^-16 C]");
            if (ImGui::SliderFloat("##q", &q, -10.0f, 10.0f)) {
                // Tutaj przyda³by siê setter w Particle, np. particle.SetCharge(q);
            }

            ImGui::Separator();
            ImGui::Text("Initial velocity (v)");
            static float v = 1.0f;
            if (ImGui::SliderFloat("[x10^6 m/s]", &v, 0.1f, 5.0f)) {
                particle.SetSpeed((double)v);
            }
        }

        ImGui::Separator();
        ImGui::Text("View & Time");
        ImGui::SliderFloat("Zoom [mm]", &worldHeight, 1.0f, 20.0f);
        ImGui::SliderFloat("Time step [dt]", &dt, 0.00001f, 0.005f, "%.5f");

        ImGui::Separator();

        if (appstate == AppState::SIMULATION) {
            if (ImGui::Button("Start")) simulate = true;
            ImGui::SameLine();
            if (ImGui::Button("Stop")) simulate = false;
            ImGui::SameLine();
            if (ImGui::Button("Reset")) retryRequested = true;
        }
        else {
            if (!inputLocked && ImGui::Button("FIRE!", ImVec2(-1, 40))) simulate = true;
            if (ImGui::Button("Try Again")) { simulate = false; retryRequested = true; }
            if (ImGui::Button("New Game")) { simulate = false; newGameRequested = true; }
        }

        // Obliczanie promienia (uproszczone do p³aszczyzny prostopad³ej)
        double v_mag = glm::length(particle.GetVelocity());
        double B_mag = glm::length(B);
        // q i m trzeba by pobraæ z cz¹stki (dodaj gettery!)
        // float radius = (m * v_mag) / (std::abs(q) * B_mag);

        ImGui::Separator();
        if (ImGui::Button("Back to Menu")) {
            appstate = AppState::MENU;
            simulate = false;
        }
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}