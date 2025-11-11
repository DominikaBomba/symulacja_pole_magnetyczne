#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Particle.h"
#include <glm/glm.hpp>
#include <vector>

using namespace std;

// ----------------------------------------------------------
// Pomocnicza funkcja do kompilacji shaderów
// ----------------------------------------------------------
GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cerr << "Shader compilation failed:\n" << infoLog << endl;
    }
    return shader;
}

// ----------------------------------------------------------
// Callback zmiany rozmiaru okna
// ----------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// ----------------------------------------------------------
// MAIN
// ----------------------------------------------------------
int main()
{
    // Inicjalizacja GLFW
    if (!glfwInit()) {
        cerr << "Inicjacja GLFW się nie udała\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    // ----------------------------------------------------------
    // Shadery
    // ----------------------------------------------------------
    const char* vertexSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        void main() { gl_Position = vec4(aPos, 0.0, 1.0); }
    )";

    const char* fragmentSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main() { FragColor = vec4(0.3, 0.3, 0.9, 1.0); }
    )";

    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cerr << "Shader program linking failed:\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ----------------------------------------------------------
    // Bufory dla cząstki i trajektorii
    // ----------------------------------------------------------
    GLuint particleVAO, particleVBO;
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);

    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    GLuint trajectoryVAO, trajectoryVBO;
    glGenVertexArrays(1, &trajectoryVAO);
    glGenBuffers(1, &trajectoryVBO);

    glBindVertexArray(trajectoryVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);
    glBufferData(GL_ARRAY_BUFFER, 10000 * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // ----------------------------------------------------------
    // Obiekt cząstki
    // ----------------------------------------------------------
    Particle particle({ 0.0, 0.0 }, { 1.0, 0.0 }, 1.0, 0.1);
    float Bz = 1.0f;
    float dt = 0.00025f;
    bool simulate = false;

    // ----------------------------------------------------------
    // ImGui
    // ----------------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ----------------------------------------------------------
    // Pętla główna
    // ----------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Nowa klatka ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Sterowanie symulacją");
        ImGui::Text("Parametry cząstki");

        ImGui::Separator();
        ImGui::Text("Natężenie pola (Bz)");
        ImGui::SliderFloat("B [T]", &Bz, 0.0f, 2.0f);

        ImGui::Separator();
        ImGui::Text("Masa (m)");
        ImGui::SliderFloat("m [x10^-25 kg]", &particle.mass, 0.1f, 10.0f);


        ImGui::Separator();
        ImGui::Text("Ładunek cząstki (q)");
        ImGui::SliderFloat("x10^-16 [C]", &particle.charge, 1.0f, 10.0f);


        ImGui::Separator();
        ImGui::Text("Prędkość początkowa");
        static float v = 1.0f;
        if (ImGui::SliderFloat("v [x10^6 m/s]", &v, 0.1f, 5.0f)) {
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
        
        float promien = (particle.mass * v) / (Bz * particle.charge);
        ImGui::Text("Promień: %.3f", promien);



        if (ImGui::Button("Reset")) {
            particle.Reset({ 0.0, 0.0 }, { 1.0, 0.0 });
            glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);
            std::vector<float> empty;
            glBufferSubData(GL_ARRAY_BUFFER, 0, 0, empty.data());
        }

        ImGui::End();

        // ----------------------------------------------------------
        // Aktualizacja cząstki
        // ----------------------------------------------------------
        static int stepCounter = 0;
        if (simulate) {
            particle.UpdateRK4(dt, Bz);
            stepCounter++;

            if (particle.trajectory.size() > 10000)
                particle.trajectory.erase(particle.trajectory.begin());

            if (stepCounter % 10 == 0) {
                std::vector<float> points;
                points.reserve(particle.trajectory.size() * 2);
                for (auto& p : particle.trajectory) {
                    points.push_back((float)p.x);
                    points.push_back((float)p.y);
                }

                glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(float), points.data());
            }
        }

        // ----------------------------------------------------------
        // Renderowanie
        // ----------------------------------------------------------
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Rysowanie cząstki
        float pos[2] = { (float)particle.position.x, (float)particle.position.y };
        glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), pos);
        glPointSize(10.0f);
        glBindVertexArray(particleVAO);
        glDrawArrays(GL_POINTS, 0, 1);

        // Rysowanie toru
        glPointSize(2.0f);
        glBindVertexArray(trajectoryVAO);
        glDrawArrays(GL_POINTS, 0, (GLsizei)particle.trajectory.size());

        glBindVertexArray(0);
        glUseProgram(0);

        // ImGui rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // ----------------------------------------------------------
    // Sprzątanie
    // ----------------------------------------------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
