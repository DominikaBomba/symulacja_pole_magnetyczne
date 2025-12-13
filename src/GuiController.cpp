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

// ZMIANA: Funkcja teraz zwraca void, a flagi resetu przekazuje przez referencje
void GuiController::Render(AppState& appstate, bool& simulate, float& Bz, float& dt,
    Particle& particle, float& worldHeight, Target& target,
    bool& newGameRequested, bool& retryRequested) {

    // Reset flag na pocz¹tku klatki
    newGameRequested = false;
    retryRequested = false;

    // Nowa klatka ImGui (kod startowy z maina)
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (appstate == AppState::MENU) {
        ImGui::Begin("MENU");
        ImGui::Text("Wybierz Tryb");

        if (ImGui::Button("GRA")) {
            appstate = AppState::GAME;
            simulate = false;      // Zatrzymujemy symulacjê przy wejœciu
            retryRequested = true; // Wymuszamy reset (czyszczenie œladu)
        }
        if (ImGui::Button("SYMULACJA")) {
            appstate = AppState::SIMULATION;
            simulate = false;      // Zatrzymujemy symulacjê przy wejœciu
            retryRequested = true; // Wymuszamy reset
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
        ImGui::SliderFloat("dt", &dt, 0.00001f, 0.005f, "%.5f");


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

            // Logika wizualna - zg³aszamy ¿¹danie (retry dzia³a jak reset w symulacji)
            retryRequested = true;
        }
        // Przycisk powrotu, ¿eby nie utkn¹æ w grze
        if (ImGui::Button("Wróæ do Menu")) {
            appstate = AppState::MENU;
            simulate = false; // Zatrzymujemy przy wyjœciu
        }
        ImGui::End();
    }


    if (appstate == AppState::GAME) {
        ImGui::Begin("Tryb gry");

        ImGui::Text("Wybierz Parametry by trafiæ do celu");
        ImGui::Text("(Pole dzia³a tylko po prawej stronie!)");

        if (target.isHit) {
            // simulate = false; // To obs³ugujemy teraz w main
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "BRAWO! Cel Trafiony!");
            ImGui::Separator();
        }

        // BLOKADA INPUTU: parametry zablokowane gdy leci lub trafiono
        bool inputLocked = simulate || target.isHit;

        if (inputLocked) {
            ImGui::TextDisabled("Symulacja w toku - parametry zablokowane");
        }
        else {
            ImGui::Separator();
            ImGui::Text("Widok (Kamera)");

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
            // DODANO: Wybór kroku czasowego w grze
            ImGui::Text("Krok czasowy (dt)");
            ImGui::SliderFloat("dt", &dt, 0.00001f, 0.005f, "%.5f");

            ImGui::Separator();
            if (ImGui::Button("WYSTRZEL CZASTKE!", ImVec2(-1, 40))) {
                simulate = true;
            }
        }

        ImGui::Separator();

        //spr ¿e nie dzieli przez 0 - denomi to mianownik we wzorze na promien
        float denomi = std::abs(Bz) * std::abs(particle.charge);

        if (denomi > 0.000001f) { // Sprawdzenie, czy mianownik jest bliski zero
            // U¿ywamy aktualnej prêdkoœci cz¹stki do obliczeñ
            float currentV = glm::length(particle.velocity);
            if (currentV == 0) currentV = 1.0f;

            float promien = (particle.mass * currentV) / denomi;

            ImGui::Text("Teoretyczny Promieñ (R): %f mm", promien);
        }
        else {

            ImGui::Text("Promieñ (R): Nieskoñczony (Linia prosta)");
        }

        // --- PRZYCISKI STERUJ¥CE ---
        ImGui::Separator();

        if (ImGui::Button("Spróbuj jeszcze raz")) {
            simulate = false;
            retryRequested = true; // Tylko reset pozycji
        }

        if (ImGui::Button("Nowa Gra (Losuj cel)")) {
            simulate = false;
            newGameRequested = true; // Reset + nowy cel
        }

        // Przycisk powrotu, ¿eby nie utkn¹æ w grze
        if (ImGui::Button("Wróæ do Menu")) {
            appstate = AppState::MENU;
            simulate = false; // Zatrzymujemy przy wyjœciu
        }

        ImGui::End();
    }

    // Renderowanie (finalizacja klatki ImGui)
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}