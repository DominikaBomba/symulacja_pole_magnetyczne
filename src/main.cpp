#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

using namespace std;
#pragma once
#include <glm/glm.hpp> // dla pozycji
#include <glad/glad.h>

class Charge {
public:
    glm::vec2 position; // pozycja w oknie
    float radius;       // promie� k�ka
    glm::vec3 color;    // kolor (RGB)

    Charge(glm::vec2 pos, float r, glm::vec3 col)
        : position(pos), radius(r), color(col) {
    }

    void Draw() {
        glColor3f(color.r, color.g, color.b); // ustaw kolor

        glBegin(GL_TRIANGLE_FAN); // rysujemy wype�nione k�ko
        glVertex2f(position.x, position.y); // �rodek

        int numSegments = 50; // im wi�cej segment�w, tym bardziej okr�g�e
        for (int i = 0; i <= numSegments; i++) {
            float angle = i * 2.0f * 3.1415926f / numSegments;
            float x = position.x + cos(angle) * radius;
            float y = position.y + sin(angle) * radius;
            glVertex2f(x, y);
        }
        glEnd();
    }
};

//kiedy zmieniamy rozmiar okna, funkcja jest wywo�ywana 
//(m�wi openGL kt�ra cz�� okna b�dzie renderowana)
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main() 
{
    //glfwInit uruchamia nam t� biblioreke GLFW - odpowiada za wygenerowanie okna
    if (!glfwInit()) {
        //cerr to takie cout tylko dla bl�d�w
        cerr << "Inicjacja GLFW si� nie uda�a\n";
        return -1;
    }

    //ustawiamy dobr� wersj� GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //funkcja glfwCreateWindow tworzy nam okno 
    //glfwTerminate "sprz�ta po glfw" zamyka wszsystkie okna itd
    GLFWwindow* window = glfwCreateWindow(800, 600, "Tu damy tytu�:D", nullptr, nullptr);
    if (!window) {
        cerr << "Nie uda�o si� utworzy� okna GLFW\n";
        glfwTerminate(); 
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "Nie uda�o si� za�adowa� GLAD\n";
        return -1;
    }

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Charge electron({ 0.0f, 0.0f }, 0.05f, { 0.0f, 0.0f, 1.0f }); // niebieski elektron

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Witaj w ImGui!");
        ImGui::Text("Gdzie elektron");
        // w p�tli renderowania, zamiast ImGui::Text:
        electron.Draw();
        ImGui::End();

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.2f, 0.8f, 0.3f, 1.0f); //kolor t�a
        glClear(GL_COLOR_BUFFER_BIT);
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
