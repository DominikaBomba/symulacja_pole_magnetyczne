#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" 

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Particle.h"
#include <glm/glm.hpp>
#include <vector>
#include <vector>
#include <cmath>
#include <random> 

using namespace std;

enum AppState {
    MENU, SIMULATION, GAME
};

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

GLuint LoadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    // Odwracamy obraz w pionie, bo OpenGL ma (0,0) na dole
    stbi_set_flip_vertically_on_load(true);

  
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, STBI_rgb);

    if (data) {
        GLenum format = 0;

        if (nrComponents == 1)      format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA; 

        if (format == 0) {
            cerr << "Nieznany format obrazu: " << path << " (komponentow: " << nrComponents << ")" << endl;
            stbi_image_free(data);
            return 0;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        cout << "Zaladowano teksture: " << path << " (komponentow: " << nrComponents << ")" << endl;
    }
    else {
        cerr << "Nie udalo sie zaladowac tekstury: " << path << endl;
        stbi_image_free(data);
    }

    return textureID;
}

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
    // Usuniecie shaderow po zlinkowaniu
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    const char* bgVertexSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    const char* bgFragmentSource = R"(
        #version 330 core
        out vec4 FragColor;
        in vec2 TexCoord;
        uniform sampler2D texture1;
        void main() {
            FragColor = texture(texture1, TexCoord);
        }
    )";

    GLuint bgVS = CompileShader(GL_VERTEX_SHADER, bgVertexSource);
    GLuint bgFS = CompileShader(GL_FRAGMENT_SHADER, bgFragmentSource);
    GLuint bgShaderProgram = glCreateProgram();
    glAttachShader(bgShaderProgram, bgVS);
    glAttachShader(bgShaderProgram, bgFS);
    glLinkProgram(bgShaderProgram);
    glDeleteShader(bgVS);
    glDeleteShader(bgFS);
    float bgVertices[] = {
        // pozycje          // tekstury
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f, // prawy gorny
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f, // prawy dolny
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, // lewy dolny
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f  // lewy gorny
    };
    unsigned int bgIndices[] = {
        0, 1, 3, // pierwszy trojkat
        1, 2, 3  // drugi trojkat
    };

    GLuint bgVAO, bgVBO, bgEBO;
    glGenVertexArrays(1, &bgVAO);
    glGenBuffers(1, &bgVBO);
    glGenBuffers(1, &bgEBO);

    glBindVertexArray(bgVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgVertices), bgVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bgIndices), bgIndices, GL_STATIC_DRAW);

    // Atrybut pozycji
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Atrybut tekstury
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);



    GLuint textureBackground1 = LoadTexture("C:\\Users\\Dominika Bomba\\Desktop\\elektron_w_polu\\background.png");
    GLuint textureBackground2 = LoadTexture("background.png"); // Zakladamy, ze ten plik istnieje
    int selectedBackground = 0; // 0 = TLO1, 1 = TLO2

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

    
    Particle particle({ 0.0, 0.0 }, { 1.0, 0.0 }, 1.0, 0.1);
    float Bz = -1.0f;
    float dt = 0.00025f;
    bool simulate = false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Nowa klatka ImGui
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

            ImGui::Begin("Sterowanie symulacją");
            ImGui::Text("Parametry cząstki");

            ImGui::Separator();
            ImGui::Text("Natężenie pola (Bz)");
        
            ImGui::SliderFloat("B [T]", &Bz, -2.0f, 2.0f);

            ImGui::Separator();
            ImGui::Text("Masa (m)");
            ImGui::SliderFloat("m [x10^-25 kg]", &particle.mass, 0.1f, 10.0f, "%.1f");


            ImGui::Separator();
            ImGui::Text("Ładunek cząstki (q)");
            ImGui::SliderFloat("x10^-16 [C]", &particle.charge, 1.0f, 10.0f, "%.1f");


            ImGui::Separator();
            ImGui::Text("Prędkość początkowa");
            static float v = 1.0f;
            if (ImGui::SliderFloat("v [x10^6 m/s]", &v, 0.1f, 5.0f, "%.1f")) {
                particle.SetSpeed(v);
            }

            ImGui::Separator();
            
            ImGui::Text("Wybierz tlo:");
            ImGui::RadioButton("TLO1", &selectedBackground, 0);
            ImGui::SameLine();
            ImGui::RadioButton("TLO2", &selectedBackground, 1);

        

            ImGui::Separator();
            

            ImGui::Text("Krok czasowy (dt)");
            ImGui::SliderFloat("dt", &dt, 0.00001f, 0.005f);


            ImGui::Separator();
            if (ImGui::Button("Start")) simulate = true;

            ImGui::SameLine();
            if (ImGui::Button("Stop")) simulate = false;
            ImGui::SameLine();

            ImGui::Separator();

            //spr że nie dzieli przez 0 - denomi to mianownik we wzorze na promien
            float denomi = std::abs(Bz) * std::abs(particle.charge);

            if (denomi > 0.000001f) { // Sprawdzenie, czy mianownik jest bliski zero
                float promien = (particle.mass * v) / denomi;




                ImGui::Text("Promień (R): %f mm", promien); 
            }
            else {
               
                ImGui::Text("Promień (R): Nieskończony (Linia prosta)");
            }
          

            if (ImGui::Button("Reset")) {
                particle.Reset({ 0.0, 0.0 }, { 1.0, 0.0 });
                glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);
                std::vector<float> empty;
                glBufferSubData(GL_ARRAY_BUFFER, 0, 0, empty.data());
            }

            ImGui::End();
        }


        if (appstate == AppState::
            GAME) {
            ImGui::Begin("Tryb gry");

            ImGui::Text("Wybierz Parametry by trafić do celu");


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
            ImGui::End();
        }






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
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (appstate == AppState::SIMULATION) {
            glUseProgram(bgShaderProgram);
            glActiveTexture(GL_TEXTURE0);
            if (selectedBackground == 0) {
                glBindTexture(GL_TEXTURE_2D, textureBackground1);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, textureBackground2);
            }

            glUniform1i(glGetUniformLocation(bgShaderProgram, "texture1"), 0);

            glBindVertexArray(bgVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // 2. RYSOWANIE CZASTKI I TORU
        // (Rysujemy na tle, wiec po narysowaniu tla)
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

    glDeleteVertexArrays(1, &particleVAO);
    glDeleteVertexArrays(1, &trajectoryVAO);
    glDeleteVertexArrays(1, &bgVAO);
    glDeleteBuffers(1, &particleVBO);
    glDeleteBuffers(1, &trajectoryVBO);
    glDeleteBuffers(1, &bgVBO);
    glDeleteBuffers(1, &bgEBO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}