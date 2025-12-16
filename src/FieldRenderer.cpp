#include "FieldRenderer.h"

// Vertex Shader t³a
const char* BG_VERTEX_SOURCE = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";

// Fragment Shader t³a
const char* BG_FRAGMENT_SOURCE = R"(
        #version 330 core
        out vec4 FragColor;
        in vec2 TexCoord;

        // Parametry przekazywane z programu
        uniform float Bz;
        uniform float aspectRatio;
		uniform bool gameMode;// flaga trybu gry

        void main() {
            // kolor t³a (ciemnoszary)
            vec3 backgroundColor = vec3(0.1, 0.1, 0.1);
            vec3 patternColor = vec3(0.35, 0.35, 0.4); // Lekko niebieskawy szary dla symboli

            // Jeœli pole równe 0 rysujemy czyste t³o
            if (abs(Bz) == 0) {
                FragColor = vec4(backgroundColor, 1.0);
                return;
            }
			
			// W trybie gry rysujemy t³o tylko po prawej stronie ekranu
			if (gameMode && TexCoord.x < 0.5) {
				FragColor = vec4(backgroundColor, 1.0);
				return;
			}

            // Skalowanie œwiata
            // Ustalamy, ¿e wysokoœæ widoku to zawsze 4.0 jednostki
            float worldH = 4.0;
            float worldW = worldH * aspectRatio;
            
            // Przeliczamy UV (0..1) na pozycjê w œwiecie (-W/2..W/2, -H/2..H/2)
            vec2 worldPos = vec2(
                (TexCoord.x - 0.5) * worldW,
                (TexCoord.y - 0.5) * worldH
            );

            // Tworzenie siatki (Grid)
            // Im silniejsze pole (wiêksze |Bz|), tym gêstsza siatka (mniejszy gridSize)
            float density = sqrt(abs(Bz)); // Pierwiastek, ¿eby zmiany by³y ³agodniejsze
            float gridSize = 0.8 / density; // Bazowy rozmiar kratki
            
            // Fract zwraca u³amkow¹ czêœæ liczby -> tworzy powtarzalny wzór 0..1 w ka¿dej kratce
            vec2 cell = fract(worldPos / gridSize); 
            
            // Przesuwamy œrodek uk³adu wspó³rzêdnych kratki do jej centrum (zakres -0.5 do 0.5)
            vec2 p = cell - 0.5;

            float shape = 0.0; // 0 = t³o, 1 = wzór
            float thickness = 0.01 * density; // Gruboœæ kresek skalowana z polem

            if (Bz > 0.0) {
                // rysowanie kropki (Dla B > 0) 
                float dist = length(p);
                // smoothstep wyg³adza krawêdzie (antyaliasing)
                float radius = 0.03 * density; 
                shape = 1.0 - smoothstep(radius - 0.02, radius + 0.02, dist);
            } 
            else {
                // rysowanie X (Dla B < 0)
                float d1 = abs(p.x - p.y);
                float d2 = abs(p.x + p.y);
                
                float dist = min(d1, d2);
                
                // Ograniczamy d³ugoœæ ramion X
                // Kszta³tujemy go w kwadracie o boku 0.6
                if (abs(p.x) < 0.1 && abs(p.y) < 0.1) {
                    shape = 1.0 - smoothstep(thickness - 0.02, thickness + 0.02, dist);
                }
            }

            // Mieszanie koloru t³a z kolorem wzoru
            vec3 finalColor = mix(backgroundColor, patternColor, shape);
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
}

void FieldRenderer::Draw(float Bz, float aspectRatio, bool gameMode) {
    if (bgShader) {
        bgShader->Use();
        bgShader->SetFloat("Bz", Bz);
        bgShader->SetFloat("aspectRatio", aspectRatio);
		bgShader->SetFloat("gameMode", gameMode);

        glBindVertexArray(bgVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}