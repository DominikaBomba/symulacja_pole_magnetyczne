#include "GuiController.h"
#include <iostream> // Do ew. debugowania

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

bool GuiController::Render(AppState& appstate, bool& simulate, float& Bz, float& dt,
    Particle& particle, float& worldHeight, GLuint trajectoryVBO) {
	bool resetRequested = false;

    // Nowa klatka ImGui (kod startowy z maina)
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (appstate == AppState::MENU) {
        ImGui::Begin("MENU");
        ImGui::Text("Wybierz Tryb");

        if (ImGui::Button("GRA")) {
            appstate = AppState::GAME;
        }
        if (ImGui::Button("SYMULACJA")) {
            appstate = AppState::SIMULATION;
        } ImGui::End();
    }

    if (appstate == AppState::SIMULATION) {

        ImGui::Begin("Sterowanie symulacj¹");
        ImGui::Text("Parametry cz¹stki");

        // --- NOWOŒÆ: STEROWANIE SKAL¥ (ZOOM) ---
        ImGui::Separator();
        ImGui::Text("Widok (Kamera)");
        // Domyœlnie 8.0. Mniejsze wartoœci = przybli¿enie.
        ImGui::SliderFloat("Skala (Zoom) [m]", &worldHeight, 1.0f, 20.0f, "%.1f");

        ImGui::Separator();
        ImGui::Text("Natê¿enie pola (Bz)");

        ImGui::SliderFloat("B [T]", &Bz, -2.0f, 2.0f);

        ImGui::Separator();
        ImGui::Text("Masa (m)");
        ImGui::SliderFloat("m [x10^-25 kg]", &particle.mass, 0.1f, 10.0f, "%.1f");


        ImGui::Separator();
        ImGui::Text("£adunek cz¹stki (q)");
        ImGui::SliderFloat("x10^-16 [C]", &particle.charge, 1.0f, 10.0f, "%.1f");


        ImGui::Separator();
        ImGui::Text("Prêdkoœæ pocz¹tkowa");
        static float v = 1.0f;
        if (ImGui::SliderFloat("v [x10^6 m/s]", &v, 0.1f, 5.0f, "%.1f")) {
            particle.SetSpeed(v);
        }

        ImGui::Separator();

        ImGui::Text("Krok czasowy (dt)");
        ImGui::SliderFloat("dt", &dt, 0.00001f, 0.005f);


        ImGui::Separator();
        if (ImGui::Button("Start")) simulate = true;

        ImGui::SameLine();
        if (ImGui::Button("Stop")) simulate = false;
        ImGui::SameLine();

        ImGui::Separator();

        //spr ¿e nie dzieli przez 0 - denomi to mianownik we wzorze na promien
        float denomi = std::abs(Bz) * std::abs(particle.charge);

        if (denomi > 0.000001f) { // Sprawdzenie, czy mianownik jest bliski zero
            float promien = (particle.mass * v) / denomi;

            ImGui::Text("Promieñ (R): %f mm", promien);
        }
        else {

            ImGui::Text("Promieñ (R): Nieskoñczony (Linia prosta)");
        }

        if (ImGui::Button("Reset")) {
            // Logika C++
            particle.Reset({ 0.0, 0.0 }, { 1.0, 0.0 });

            // Logika wizualna - zg³aszamy ¿¹danie
            resetRequested = true;
        }

        ImGui::End();
    }


    if (appstate == AppState::GAME) {
        ImGui::Begin("Tryb gry");

        ImGui::Text("Wybierz Parametry by trafiæ do celu");

        // --- NOWOŒÆ: TE¯ DODAJEMY ZOOM W GRZE ---
        ImGui::SliderFloat("Zoom", &worldHeight, 1.0f, 20.0f);

        ImGui::Separator();
        ImGui::Text("Natê¿enie pola (Bz)");
        ImGui::SliderFloat("B [T]", &Bz, 0.0f, 2.0f);

        ImGui::Separator();
        ImGui::Text("Masa (m)");
        ImGui::SliderFloat("m [x10^-25 kg]", &particle.mass, 0.1f, 10.0f);


        ImGui::Separator();
        ImGui::Text("£adunek cz¹stki (q)");
        ImGui::SliderFloat("x10^-16 [C]", &particle.charge, 1.0f, 10.0f);


        ImGui::Separator();
        ImGui::Text("Prêdkoœæ pocz¹tkowa");
        static float v = 1.0f;
        if (ImGui::SliderFloat("v [x10^6 m/s]", &v, 0.1f, 5.0f)) {
            particle.SetSpeed(v);
        }

        // Przycisk powrotu, ¿eby nie utkn¹æ w grze
        if (ImGui::Button("Wróæ do Menu")) {
            appstate = AppState::MENU;
        }

        ImGui::End();
    }

    // Renderowanie (finalizacja klatki ImGui)
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	return resetRequested;
}