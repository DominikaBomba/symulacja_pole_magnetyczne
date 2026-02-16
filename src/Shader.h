#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    GLuint ID; // Identyfikator programu w OpenGL

    // Konstruktor
    Shader(const char* vertexSource, const char* fragmentSource);

    // Aktywacja shadera
    void Use();

    // Funkcje pomocnicze do wysy³ania zmiennych (Uniforms)
    void SetFloat(const std::string& name, float value) const;
    void SetMat4(const std::string& name, const glm::mat4& mat) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
private:
    // Funkcja pomocnicza do sprawdzania b³êdów kompilacji
    void CheckCompileErrors(GLuint shader, std::string type);
};