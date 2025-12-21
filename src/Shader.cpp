#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const char* vertexSource, const char* fragmentSource) {
    // Kompilacja Vertex Shadera
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSource, NULL);
    glCompileShader(vertex);
    CheckCompileErrors(vertex, "VERTEX");

    // Kompilacja Fragment Shadera
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSource, NULL);
    glCompileShader(fragment);
    CheckCompileErrors(fragment, "FRAGMENT");

    // Linkowanie Programu
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    CheckCompileErrors(ID, "PROGRAM");

	// usuwanie uzytych shaderów
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::Use() {
    glUseProgram(ID);
}

void Shader::SetFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::CheckCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "BLAD::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "BLAD::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}