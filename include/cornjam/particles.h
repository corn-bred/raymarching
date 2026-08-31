#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <cornbreadlib/SSBO.h>
#include <cornbreadlib/computeshader.h>
#include <cornbreadlib/shaders.h>
#include <cornbreadlib/texturebuffer.h>
#include <cornbreadlib/vertexbuffer.h>
#include <time.h>

#include "spritesheet.h"
#include <misc/globals.h>

#define PI 3.14159265358979323846

inline float randomFloat(float offset) {
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0);
}


struct Particle {
    glm::vec2 Position; //8 bytes = 8 =/8
    glm::vec2 Origin;   //8 bytes = 16 =/8
    glm::vec2 Velocity; //8 bytes = 24 =/8
    float Duration;     //4 bytes = 28 =/4
    float MaxDuration;  //4 bytes = 32 =/4
    float Size;         //4 bytes = 36 =/4
    uint32_t Flags;     //4 bytes = 40 =/4
    float Speed;        //4 bytes = 44 !/8
    float padding;       //4 bytes = 44 =/8
};

#define FLAG_IS_ACTIVE (1u << 0) // 1
#define FLAG_IS_LOOPING   (1u << 1) // 2
#define FLAG_HAS_GRAVITY (1u << 2) // 4

#define RENDERING_SOLID_COLOUR 0
#define RENDERING_TEXTURE 1
#define RENDERING_ANIMATED 2

class Particles {
    private:
    ComputeShader mainCompute;
    Shader Renderer;
    ShaderStorageBuffer SSBO;

    int RenderingType = 0; //0: Solid Colour, 1: Texture, 2: Animated
    glm::vec3 SolidColour = glm::vec3(1.0);
    TextureBuffer *Texture = nullptr;
    Animation *Spritesheet = nullptr;

    VertexBuffer DummyVBO;

    public:
    const int ParticleNum = 0;

    Particles(const char *computePath, int particleNum, glm::vec2 origin, float maxDuration, float size, float speed, bool hasGravity, bool isLooping) : mainCompute(computePath), Renderer("src/shaders/particles.vert", "src/shaders/particles.frag"), ParticleNum(particleNum), SSBO(nullptr, 0, GL_DYNAMIC_DRAW), DummyVBO(nullptr, 0, GL_STATIC_DRAW) {
        std::vector<Particle> particles(ParticleNum);

        for (int i = 0; i < ParticleNum; i++) {
            particles[i].Origin = origin;
            particles[i].Position = origin;
            double RandomDouble = randomFloat(i);
            particles[i].Velocity = glm::vec2(cos(RandomDouble * PI * 2) * speed, sin(RandomDouble * PI * 2) * speed);
            particles[i].Duration = (float(i) / float(particleNum)) * maxDuration;
            particles[i].MaxDuration = maxDuration;
            particles[i].Size = size;
            
            particles[i].Speed = speed;

            particles[i].padding = 0.0f;

            particles[i].Flags = FLAG_IS_ACTIVE;
            if (hasGravity) particles[i].Flags |= FLAG_HAS_GRAVITY;
            if (isLooping) particles[i].Flags |= FLAG_IS_LOOPING;
        }

        ShaderStorageBuffer tempSSBO(particles.data(), ParticleNum * sizeof(Particle), GL_DYNAMIC_DRAW);
        SSBO = std::move(tempSSBO);
        mainCompute.bind();
        SSBO.bindToShader(0);
    }

    //BRUHH deconstructor is TECHNICALLY custom because of the variables inside

    Particles(Particles&& other) noexcept :
        mainCompute(std::move(other.mainCompute)),
        Renderer(std::move(other.Renderer)),
        SSBO(std::move(other.SSBO)),
        DummyVBO(std::move(other.DummyVBO)),
        Texture(other.Texture),
        Spritesheet(other.Spritesheet),
        RenderingType(other.RenderingType),
        SolidColour(other.SolidColour) {
        
        other.Texture = nullptr;
        other.Spritesheet = nullptr;
    } //do allat

    Particles& operator=(Particles&& other) noexcept {
        if (this != &other) {
            // move assignment
            mainCompute = std::move(other.mainCompute);
            Renderer = std::move(other.Renderer);
            SSBO = std::move(other.SSBO);
            DummyVBO = std::move(other.DummyVBO);
            
            // move all the pointer classes and stuff
            Texture = other.Texture;
            Spritesheet = other.Spritesheet;
            
            RenderingType = other.RenderingType;
            SolidColour = other.SolidColour;
            
            // then destroy
            other.Texture = nullptr;
            other.Spritesheet = nullptr;
        }
        return *this;
    }

    void Update(glm::vec2 gravity) {
        mainCompute.bind();
        
        mainCompute.setFloat("uDeltaTime", DeltaTime);
        mainCompute.setVec2("uGravity", gravity);
        mainCompute.setFloat("uCurrentTime", CurrentTime);

        mainCompute.use((ParticleNum + 255) / 256, 1, 1, GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    }

    void RenderSolidColourState(glm::vec3 colour) {
        RenderingType = RENDERING_SOLID_COLOUR;
        SolidColour = colour;
    }

    void RenderTextureState(TextureBuffer &texture) {
        RenderingType = RENDERING_TEXTURE;
        Texture = &texture;
    }

    void RenderAnimatedState(Animation &animation) {
        RenderingType = RENDERING_ANIMATED;
        Spritesheet = &animation;
    }

    void Render(glm::mat4 &view, glm::mat4 &projection) {
        Renderer.use();
        Renderer.setMat4("view", view);
        Renderer.setMat4("projection", projection);

        switch(RenderingType) {
            case 0:
            Renderer.setBool("isSolidColour", true);
            Renderer.setVec3("Colour", SolidColour);
            break;

            case 1:
            Renderer.setBool("isSolidColour", false);
            Texture->bindTexture(0);
            break;

            case 2:
            Renderer.setBool("isSolidColour", false);
            Renderer.setBool("isAnimation", true);
            Renderer.setInt("Columns", Spritesheet->SpriteRenderer.Columns);
            Renderer.setInt("Rows", Spritesheet->SpriteRenderer.Rows);
            Renderer.setInt("Frame", Spritesheet->FrameHandler.GetFrame());
            Spritesheet->SpriteRenderer.Sprite.bindTexture(0);
            break;

            default:
            Renderer.setBool("isSolidColour", true);
            Renderer.setVec3("Colour", glm::vec3(1.0));
            break;
        }

        DummyVBO.bind();

        glDrawArrays(GL_TRIANGLES, 0, ParticleNum * 6);
    }
};