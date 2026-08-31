#pragma once
#include <cornbreadlib/texturebuffer.h>
#include <cornbreadlib/vertexbuffer.h>
#include <cornbreadlib/shaders.h>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/exponential.hpp>
#include <misc/globals.h>

class Anim_FrameHandler {
    private:
    int CurrentFrame;
    float FrameTime; //Moduloed time, if greater than FrameDuration, reset
    public:

    Anim_FrameHandler(int startframe, int endframe, float frameduration, bool islooping = false) {
        StartFrame = startframe;
        EndFrame = endframe;
        FrameDuration = frameduration;
        isLooping = islooping;

        CurrentFrame = StartFrame;
        FrameTime = 0.0f;
    }

    int StartFrame, EndFrame;
    float FrameDuration;
    bool isLooping;

    void UpdateFrame(float currentTime, float deltaTime) {

        if (isLooping) {

            int TotalFrames = EndFrame - StartFrame + 1;
            CurrentFrame = (int(floor(currentTime / FrameDuration)) % TotalFrames) + StartFrame;

        } else {
            FrameTime += deltaTime;

            if (FrameTime >= FrameDuration) {
                float Loopovers = FrameTime / FrameDuration;
                int LoopoversFloor = floor(Loopovers);

                FrameTime = FrameDuration * (Loopovers - LoopoversFloor);

                if (CurrentFrame + LoopoversFloor <= EndFrame) CurrentFrame += LoopoversFloor;
                else CurrentFrame = EndFrame;
            }

        }

    }

    void ResetAnimation() {
        CurrentFrame = StartFrame;
        FrameTime = 0.0f;
    }

    inline int GetFrame() {
        return CurrentFrame;
    }
};

class Anim_SpriteRenderer {
    public:
    TextureBuffer Sprite;
    int Rows, Columns;
    Anim_SpriteRenderer(const char *texturepath, int rows, int columns, GLenum wrappingS = GL_MIRRORED_REPEAT, GLenum wrappingT = GL_MIRRORED_REPEAT, GLenum min = GL_NEAREST_MIPMAP_LINEAR, GLenum mag = GL_LINEAR, bool isFlippedVert = true) : Sprite(texturepath, wrappingS, wrappingT, min, mag, isFlippedVert) {
        Rows = rows;
        Columns = columns;
    }
    Anim_SpriteRenderer(TextureBuffer &sprite, int rows, int columns) {
        Sprite = std::move(sprite);
        Rows = rows;
        Columns = columns;
    }
    void RenderSprite(Shader &shader, VertexBuffer &VBO, glm::mat4 &Model, glm::mat4 &View, glm::mat4 &Projection, int Frame) {
        shader.use();

        shader.setMat4("model", Model);

        shader.setMat4("view", View);

        shader.setMat4("projection", Projection);

        shader.setBool("isSolidColour", false);

        shader.setBool("isAnimation", true);

        shader.setInt("Columns", Columns);
        shader.setInt("Rows", Rows);

        shader.setInt("Frame", Frame);

        VBO.bind();

        Sprite.bindTexture(0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};

class Animation {
    public:
    Anim_FrameHandler FrameHandler;
    Anim_SpriteRenderer SpriteRenderer;
    
    Animation(int startframe, int endframe, float frameduration, bool islooping, const char *texturepath, int columns, int rows, GLenum wrappingS = GL_MIRRORED_REPEAT, GLenum wrappingT = GL_MIRRORED_REPEAT, GLenum min = GL_NEAREST_MIPMAP_LINEAR, GLenum mag = GL_LINEAR, bool isFlippedVert = true) : FrameHandler(startframe, endframe, frameduration, islooping), SpriteRenderer(texturepath, rows, columns, wrappingS, wrappingT, min, mag, isFlippedVert) {}

    void RenderSprite(Shader &shader, VertexBuffer &VBO, glm::mat4 &Model, glm::mat4 &View, glm::mat4 &Projection) {
        SpriteRenderer.RenderSprite(shader, VBO, Model, View, Projection, FrameHandler.GetFrame());
    }

    void Update() {
        FrameHandler.UpdateFrame(CurrentTime, DeltaTime);
    }
    
    void ResetAnimation() {
        FrameHandler.ResetAnimation();
    }
};