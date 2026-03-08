#include "VectorFieldRenderer.h"

// Shader dla linii - przyjmuje pozycjê i kolor wierzcho³ka
const char* VF_VS = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    
    uniform mat4 pv;
    out vec3 Color;
    
    void main() {
        Color = aColor;
        gl_Position = pv * vec4(aPos, 1.0);
    }
)";

const char* VF_FS = R"(
    #version 330 core
    out vec4 FragColor;
    in vec3 Color;
    void main() {
        FragColor = vec4(Color, 1.0);
    }
)";

VectorFieldRenderer::VectorFieldRenderer() {
    Init();
}

VectorFieldRenderer::~VectorFieldRenderer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    delete shader;
}

void VectorFieldRenderer::Init() {
    shader = new Shader(VF_VS, VF_FS);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Alokujemy trochê pamiêci na zapas, u¿ywamy GL_DYNAMIC_DRAW, bo linie bêd¹ siê zmieniaæ co klatkê
    glBufferData(GL_ARRAY_BUFFER, 10000 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    // Atrybut pozycji (3 floaty)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atrybut koloru (3 floaty)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void VectorFieldRenderer::Draw(const glm::mat4& pv, const glm::dvec3& B_field, bool gameMode) {
    if (!shader) return;

    std::vector<float> vertices;
    vertices.reserve(2000); // Rezerwujemy pamiêæ, ¿eby nie alokowaæ jej w pêtli

    // Generujemy siatkê punktów (co 3 jednostki w przestrzeni)
    for (float x = -14.0f; x <= 14.0f; x += 4.0f) {
        for (float y = -6.0f; y <= 6.0f; y += 4.0f) {
            for (float z = -14.0; z <= 14.0f; z += 4.0f) {

                glm::dvec3 effectiveB = B_field;
                // W trybie gry zerujemy pole dla x < 0
                if (gameMode && x < 0.0f) {
                    effectiveB = glm::dvec3(0.0);
                }

                float bLen = glm::length(effectiveB);
                // Jeœli pole jest bliskie zeru, nie rysujemy strza³ki w tym punkcie
                if (bLen < 0.05f) continue;

                glm::vec3 start = glm::vec3(x, y, z);
                glm::vec3 dir = glm::normalize(glm::vec3(effectiveB));

                // D³ugoœæ strza³ki zale¿y od si³y pola (ale ograniczamy j¹, ¿eby nie zas³oni³a ekranu)
                float arrowLen = glm::clamp(bLen * 1.2f, 0.5f, 4.5f);
                glm::vec3 end = start + dir * arrowLen;

                // Ciemny niebieski u podstawy, jasny cyjan na czubku strza³ki
                glm::vec3 colorStart = glm::vec3(0.1f, 0.1f, 0.3f);
                glm::vec3 colorEnd = glm::vec3(0.0f, 0.8f, 1.0f);

                // Wierzcho³ek pocz¹tkowy
                vertices.push_back(start.x); vertices.push_back(start.y); vertices.push_back(start.z);
                vertices.push_back(colorStart.r); vertices.push_back(colorStart.g); vertices.push_back(colorStart.b);

                // Wierzcho³ek koñcowy
                vertices.push_back(end.x); vertices.push_back(end.y); vertices.push_back(end.z);
                vertices.push_back(colorEnd.r); vertices.push_back(colorEnd.g); vertices.push_back(colorEnd.b);
            }
        }
    }

    if (vertices.empty()) return;

    // Przesy³amy nowe linie do GPU
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

    // Rysujemy
    shader->Use();
    shader->SetMat4("pv", pv);

    glBindVertexArray(VAO);
    // Gruboœci linii zazwyczaj nie mo¿na bardzo zmieniaæ w nowym OpenGL, 
    // ale mo¿emy siê upewniæ, ¿e domyœlna gruboœæ jest w³¹czona
    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, (GLsizei)(vertices.size() / 6)); // 6 floatów na wierzcho³ek
    glBindVertexArray(0);
}