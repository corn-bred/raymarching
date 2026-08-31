#pragma once
#include <cornbreadlib/texturebuffer.h>
#include <cornbreadlib/vertexbuffer.h>
#include <cornbreadlib/shaders.h>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/exponential.hpp>
#include <string>
#include <vector>

struct Glyph {
    glm::vec2 TexOrigin; //8 =/4
    glm::vec2 TexEnd; //16 =/8
};

class Font {
    private:
    std::vector<Glyph> _Glyphs;
    public:

    TextureBuffer texture;

    float Width, Height;

    Font(const char *texturePath, int columns, int rows, float width, float height, bool isFlippedVert = true) : texture(texturePath, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR, isFlippedVert), Width(width), Height(height) {
        float TexWidth = 1.0, TexHeight = 1.0;

        unsigned int TotalChars = rows * columns;

        _Glyphs.resize(TotalChars);

        glm::vec2 Scale = glm::vec2(TexWidth / columns, TexHeight / rows);

        for (int i = 0; i < TotalChars; i++) {
            glm::vec2 GridPosition = glm::vec2(i % columns, floor(float(i) / columns));
            
            glm::vec2 TexOrigin = glm::vec2(GridPosition.x / columns, GridPosition.y / rows);

            glm::vec2 TexEnd = TexOrigin + Scale;

            _Glyphs[i]= {TexOrigin, TexEnd};
        }
    }

    const Glyph &GetGlyph(char c) const{
        unsigned int index = static_cast<unsigned int> (c) - 33;
        if (index >= _Glyphs.size()) index = 0;
        //std::cout << index << " = " << c << std::endl;
        return _Glyphs[index];
    }
};


class TextRenderer {
    VertexBuffer _VBO;
    Font _Font;
    Shader _Shader;
    public:
    TextRenderer(const char *texturePath, int columns, int rows, float width, float height, bool isFlippedVert = true) : _Font(texturePath, columns, rows, width, height, isFlippedVert), _VBO(nullptr, 0, GL_DYNAMIC_DRAW), _Shader("src/shaders/text.vert", "src/shaders/text.frag") {}
    

    void RenderText(const std::string text,  glm::vec2 position, float scale, glm::mat4 &view, glm::mat4 &projection) {
        

        std::vector<float> Vertices;
        Vertices.reserve(text.length() * 24); // 6 vertices, and each vertex = 4 floats (x, y, u, v) = 24 floats
        
        glm::vec2 DrawingPosition = position;

        glm::vec2 GlyphScale = glm::vec2(_Font.Width * scale, _Font.Height * scale);

        for (char c : text) {
            if (c == '\n') {
                DrawingPosition.x = position.x;
                DrawingPosition.y -= _Font.Height * scale;
                continue;
            }

            if (c == ' ') {
                DrawingPosition.x += GlyphScale.x * 1.0f;
                continue;
            }

            const Glyph &glyph = _Font.GetGlyph(c);

            //std::cout << glyph.TexOrigin.x << ", " << glyph.TexOrigin.y << std::endl;
            //std::cout << glyph.TexEnd.x << ", " << glyph.TexEnd.y << std::endl;

            glm::vec2 Origin = DrawingPosition;
            glm::vec2 OriginEnd = DrawingPosition + GlyphScale;

            glm::vec2 TexCoord1 = glm::vec2(glyph.TexOrigin.x, 1.0 - glyph.TexEnd.y);
            glm::vec2 TexCoord2 = glm::vec2(glyph.TexEnd.x, 1.0 - glyph.TexOrigin.y);

            Vertices.push_back(Origin.x); Vertices.push_back(Origin.y);
            Vertices.push_back(TexCoord1.x); Vertices.push_back(TexCoord1.y);

            Vertices.push_back(OriginEnd.x); Vertices.push_back(Origin.y);
            Vertices.push_back(TexCoord2.x); Vertices.push_back(TexCoord1.y);

            Vertices.push_back(OriginEnd.x); Vertices.push_back(OriginEnd.y);
            Vertices.push_back(TexCoord2.x); Vertices.push_back(TexCoord2.y);

            Vertices.push_back(Origin.x); Vertices.push_back(Origin.y);
            Vertices.push_back(TexCoord1.x); Vertices.push_back(TexCoord1.y);

            Vertices.push_back(OriginEnd.x); Vertices.push_back(OriginEnd.y);
            Vertices.push_back(TexCoord2.x); Vertices.push_back(TexCoord2.y);

            Vertices.push_back(Origin.x); Vertices.push_back(OriginEnd.y);
            Vertices.push_back(TexCoord1.x); Vertices.push_back(TexCoord2.y);

            DrawingPosition.x += GlyphScale.x * 1.0f;
        }

        _VBO.bind();

        glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(float), Vertices.data(), GL_DYNAMIC_DRAW);

        _VBO.addAttribute(0, 2, GL_FLOAT, 4, 0);
        _VBO.addAttribute(1, 2, GL_FLOAT, 4, 2);

        _Shader.use();

        _Shader.setMat4("view", view);

        _Shader.setMat4("projection", projection);

        _Font.texture.bindTexture(0);

        _VBO.bind();

        glDrawArrays(GL_TRIANGLES, 0, Vertices.size() / 4);
    }
};