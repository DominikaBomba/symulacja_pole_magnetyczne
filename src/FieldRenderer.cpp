#include "FieldRenderer.h"
#include <glm/gtc/type_ptr.hpp>

const char* BG_VERTEX_SOURCE = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    
    uniform mat4 pv;
    out vec3 WorldPos;
    
    void main() {
        WorldPos = aPos;
        gl_Position = pv * vec4(aPos, 1.0);
    }
)";

const char* BG_FRAGMENT_SOURCE = R"(
    #version 330 core
    out vec4 FragColor;
    in vec3 WorldPos;

    void main() {
        vec2 coord = WorldPos.xz;
        
        // Obliczanie linii siatki
        vec2 grid = abs(fract(coord - 0.5) - 0.5);
        vec2 derivative = fwidth(coord); 
        vec2 lineWidth = derivative; // Gruboœæ linii
        
        vec2 drawWidth = smoothstep(lineWidth, vec2(0.0), grid);
        float line = max(drawWidth.x, drawWidth.y);
        
        // Pogrubienie g³ównych osi przestrzeni (X=0 oraz Z=0)
        if(abs(WorldPos.x) < 0.05 || abs(WorldPos.z) < 0.05) line = 1.0;

        // Fading - siatka powoli zanika z odleg³oœci¹ od œrodka uk³adu
        float dist = length(WorldPos.xz);
        float fade = 1.0 - smoothstep(15.0, 40.0, dist);
        
        vec3 clearColor = vec3(0.1, 0.1, 0.1); // T³o sceny
        vec3 gridColor  = vec3(0.4, 0.4, 0.4); // Szary kolor siatki
        
        // Mieszamy kolory i w ogóle nie u¿ywamy przezroczystoœci (omijamy problemy z depth buforem!)
        vec3 finalColor = mix(clearColor, gridColor, line * fade);
        FragColor = vec4(finalColor, 1.0); 
    }
)";

FieldRenderer::FieldRenderer() {
    InitShader();
    InitRenderData();
}
FieldRenderer::~FieldRenderer() {
    glDeleteVertexArrays(1, &bgVAO);
    glDeleteBuffers(1, &bgVBO);
    glDeleteBuffers(1, &bgEBO);
    delete bgShader;
}
void FieldRenderer::InitShader() {
    bgShader = new Shader(BG_VERTEX_SOURCE, BG_FRAGMENT_SOURCE);
}
void FieldRenderer::InitRenderData() {
    // Tworzymy gigantyczn¹ pod³ogê na wysokoœci Y = -3.0
    float floorY = -6.0f;
    float size = 100.0f;

    // Tylko wspó³rzêdne wierzcho³ków (bez tekstur)
    float bgVertices[] = {
         size, floorY,  size, // prawy daleki
         size, floorY, -size, // prawy bliski
        -size, floorY, -size, // lewy bliski
        -size, floorY,  size  // lewy daleki
    };
    unsigned int bgIndices[] = {
        0, 1, 3,
        1, 2, 3
    };

    glGenVertexArrays(1, &bgVAO);
    glGenBuffers(1, &bgVBO);
    glGenBuffers(1, &bgEBO);

    glBindVertexArray(bgVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgVertices), bgVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bgIndices), bgIndices, GL_STATIC_DRAW);

    // ZMIANA: Stride z 5 na 3, bo zrezygnowaliœmy z uv coordów
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void FieldRenderer::Draw(const glm::mat4& pv) {
    if (bgShader) {
        bgShader->Use();
        bgShader->SetMat4("pv", pv);
        glBindVertexArray(bgVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}