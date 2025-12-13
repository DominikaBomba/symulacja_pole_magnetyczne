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
        ImGui::Text("Select app mode");

        if (ImGui::Button("GAME")) {
            appstate = AppState::GAME;
            simulate = false;      // Zatrzymujemy symulacjê przy wejœciu
            retryRequested = true; // Wymuszamy reset (czyszczenie œladu)
        }
        if (ImGui::Button("SIMULATION")) {
            appstate = AppState::SIMULATION;
            simulate = false;      // Zatrzymujemy symulacjê przy wejœciu
            retryRequested = true; // Wymuszamy reset
        } ImGui::End();
    }

    if (appstate == AppState::SIMULATION) {

        ImGui::Begin("Simulation control");
        ImGui::Text("Physics parameters");

        

        ImGui::Separator();
        ImGui::Text("Magnetic field (B)");

        ImGui::SliderFloat("[T]", &Bz, -2.0f, 2.0f);

        ImGui::Separator();
        ImGui::Text("Mass (m)");
        ImGui::SliderFloat("[x10^-25 kg]", &particle.mass, 0.1f, 10.0f, "%.1f");


        ImGui::Separator();
        ImGui::Text("Particle charge (q)");
        ImGui::SliderFloat("[x10^-16 C]", &particle.charge, 1.0f, 10.0f, "%.1f");


        ImGui::Separator();
        ImGui::Text("Initial velocity (v)");
        static float v = 1.0f;
        if (ImGui::SliderFloat("[x10^6 m/s]", &v, 0.1f, 5.0f, "%.1f")) {
            particle.SetSpeed(v);
        }

        // ---STEROWANIE SKAL¥ (ZOOM) ---
        ImGui::Separator();
		ImGui::Text("Simulation properties");
        ImGui::Separator();
        ImGui::Text("Zoom");
        // Domyœlnie 8.0. Mniejsze wartoœci = przybli¿enie.
        ImGui::SliderFloat("[mm]", &worldHeight, 1.0f, 20.0f, "%.1f");

        ImGui::Separator();

        ImGui::Text("Time step (dt)");
        ImGui::SliderFloat("[dt]", &dt, 0.00001f, 0.005f, "%.5f");
        
        ImGui::Separator();
        if (ImGui::Button("Start")) simulate = true;

        ImGui::SameLine();
        if (ImGui::Button("Stop")) simulate = false;
        ImGui::SameLine();

        ImGui::Separator();

        //spr ¿e nie dzieli przez 0 - denomi to mianownik we wzorze na promien
        float denomi = std::abs(Bz) * std::abs(particle.charge);

        if (denomi > 0.000001f) { // Sprawdzenie, czy mianownik jest bliski zero
            float radius = (particle.mass * v) / denomi;
            ImGui::Text("Radius (r) %f [mm]", radius);
        }
        else {
            ImGui::Text("Radius (r): infinite");
        }

        if (ImGui::Button("Reset")) {
            // Logika C++
            particle.Reset({ 0.0, 0.0 }, { 1.0, 0.0 });

            // Logika wizualna - zg³aszamy ¿¹danie (retry dzia³a jak reset w symulacji)
            retryRequested = true;
        }
        // Przycisk powrotu, ¿eby nie utkn¹æ w grze
        if (ImGui::Button("Back to Menu")) {
            appstate = AppState::MENU;
            simulate = false; // Zatrzymujemy przy wyjœciu
        }
        ImGui::End();
    }


    if (appstate == AppState::GAME) {
        ImGui::Begin("Game mode");

        ImGui::Text("Adjust parameters to hit the target!");
        ImGui::Separator();
        if (target.isHit) {
            // simulate = false; // To obs³ugujemy teraz w main
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Target hit!");
            ImGui::Separator();
        }

        // BLOKADA INPUTU: parametry zablokowane gdy leci lub trafiono
        bool inputLocked = simulate || target.isHit;

        if (inputLocked) {
            ImGui::TextDisabled("Simulation running...");
        }
        else {
            ImGui::Text("Physics parameters");



            ImGui::Separator();
            ImGui::Text("Magnetic field (B)");

            ImGui::SliderFloat("[T]", &Bz, -2.0f, 2.0f);

            ImGui::Separator();
            ImGui::Text("Mass (m)");
            ImGui::SliderFloat("[x10^-25 kg]", &particle.mass, 0.1f, 10.0f, "%.1f");


            ImGui::Separator();
            ImGui::Text("Particle charge (q)");
            ImGui::SliderFloat("[x10^-16 C]", &particle.charge, 1.0f, 10.0f, "%.1f");


            ImGui::Separator();
            ImGui::Text("Initial velocity (v)");
            static float v = 1.0f;
            if (ImGui::SliderFloat("[x10^6 m/s]", &v, 0.1f, 5.0f, "%.1f")) {
                particle.SetSpeed(v);
            }

            // ---STEROWANIE SKAL¥ (ZOOM) ---
            ImGui::Separator();
            ImGui::Text("Simulation properties");
            ImGui::Separator();
            ImGui::Text("Zoom");
            // Domyœlnie 8.0. Mniejsze wartoœci = przybli¿enie.
            ImGui::SliderFloat("[mm]", &worldHeight, 1.0f, 20.0f, "%.1f");

            ImGui::Separator();

            ImGui::Text("Time step (dt)");
            ImGui::SliderFloat("[dt]", &dt, 0.00001f, 0.005f, "%.5f");

            ImGui::Separator();
            if (ImGui::Button("FIRE PARTICLE!", ImVec2(-1, 40))) {
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

            float radius = (particle.mass * currentV) / denomi;

            ImGui::Text("Radius (r) %f [mm]", radius);
        }
        else {

            ImGui::Text("Radius (r): infinite");
        }

        // --- PRZYCISKI STERUJ¥CE ---
        ImGui::Separator();

        if (ImGui::Button("Try again")) {
            simulate = false;
            retryRequested = true; // Tylko reset pozycji
        }

        if (ImGui::Button("New Game")) {
            simulate = false;
            newGameRequested = true; // Reset + nowy cel
        }

        // Przycisk powrotu, ¿eby nie utkn¹æ w grze
        if (ImGui::Button("Back to Menu")) {
            appstate = AppState::MENU;
            simulate = false; // Zatrzymujemy przy wyjœciu
        }

        ImGui::End();
    }

    // Renderowanie (finalizacja klatki ImGui)
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}