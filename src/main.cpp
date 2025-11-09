#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Particle.h"


#include <glm/glm.hpp> // dla pozycji
#include <glad/glad.h>
// Funkcja pomocnicza do kompilacji shaderów

using namespace std;

//Muszą być Shader7
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



//kiedy zmieniamy rozmiar okna, funkcja jest wywo�ywana 
//(m�wi openGL kt�ra cz�� okna b�dzie renderowana)
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    Particle particle({ 0.0, 0.0 }, { 1.0, 0.0 }, 1.0, 0.1);
    double Bz = 1.0;
    double dt = 0.001;
    bool simulate = false;
    //glfwInit uruchamia nam t� biblioreke GLFW - odpowiada za wygenerowanie okna
    if (!glfwInit()) {
        //cerr to takie cout tylko dla bl�d�w
        cerr << "Inicjacja GLFW si� nie uda�a\n";
        return -1;
    }

    //ustawiamy dobrą wersje GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //funkcja glfwCreateWindow tworzy nam okno 
    //glfwTerminate "sprzata po glfw" zamyka wszsystkie okna itd
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Tu damy tytu�:D", nullptr, nullptr);
    if (!window) {
        cerr << "Nie udało się utworzyć okna GLFW\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "Nie uda�o si� za�adowa� GLAD\n";
        return -1;
    }

    // Vertex shader i fragment shader (możesz wstawić jako stringi)
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

    // Sprawdzenie linkowania
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cerr << "Shader program linking failed:\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
   

    // VAO i VBO dla cząstki
    GLuint particleVAO, particleVBO;
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);

    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2, nullptr, GL_DYNAMIC_DRAW); // miejsce na 2 współrzędne x,y
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // VAO i VBO dla toru cząstki
    GLuint trajectoryVAO, trajectoryVBO;
    glGenVertexArrays(1, &trajectoryVAO);
    glGenBuffers(1, &trajectoryVBO);

    glBindVertexArray(trajectoryVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);
    // Miejsce na 10000 punktów po 2 floaty (x, y) (można potem powiększyć)
    glBufferData(GL_ARRAY_BUFFER, 10000 * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);  



    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Symulacja cząstki");
        ImGui::Text("Zmień parametry");

        ImGui::InputDouble("B[T] - natężenie pola", &Bz);
        ImGui::InputDouble("dt [s]", &dt);
        ImGui::InputDouble("m[jednoskta] - masa cząstki", &particle.mass);
        ImGui::InputDouble("q[jednoskta] - ładunek cząstki", &particle.charge);

        if (ImGui::Button("Start")) simulate = true;
        ImGui::SameLine();
        if (ImGui::Button("Stop")) simulate = false;
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            particle.Reset({ 0.0, 0.0 }, { 1.0, 0.0 });
            // Czyszczenie VBO trajektorii lotu
            glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);
            std::vector<float> empty;
            glBufferSubData(GL_ARRAY_BUFFER, 0, 0, empty.data());
        }

        ImGui::End();
		//Rysowanie trajektorii co 10 kroków (dla lepszej wydajności)
        static int stepCounter = 0;
        if (simulate) {
			particle.UpdateRK4(dt, Bz); // Aktualizacja pozycji cząstki
            stepCounter++;

            if (particle.trajectory.size() > 10000)
                particle.trajectory.erase(particle.trajectory.begin());

            // Co 10 kroków aktualizujemy bufor trajektorii
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

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f); //kolor tła
        glClear(GL_COLOR_BUFFER_BIT);
        // Przygotuj dane do wysłania do VBO
       
        float pos[2] = { (float)particle.position.x, (float)particle.position.y };
        glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), pos);

		// Rysuj cząstkę (point size 10f)
        glPointSize(10.0f);
        glUseProgram(shaderProgram);
        glBindVertexArray(particleVAO);
        glDrawArrays(GL_POINTS, 0, 1);
        glBindVertexArray(0);
        glUseProgram(0);

        // Rysowanie toru cząstki (point size 1f)
        glPointSize(1.0f);
        glUseProgram(shaderProgram);
        glBindVertexArray(trajectoryVAO);
		glDrawArrays(GL_POINTS, 0, (GLsizei)particle.trajectory.size()); // Rysuj linię łączącą punkty toru
        glBindVertexArray(0);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

