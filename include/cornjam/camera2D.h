#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/exponential.hpp>
#include <cornjam/entity.h>

class Camera2D {
    private:
    glm::vec2 Direction;
    float DirectionRad;
    public:
    glm::vec3 Position;
    glm::vec2 Scale;

    //Construction
    Camera2D(glm::vec3 position, glm::vec2 scale, glm::vec2 directionVec2) : Position(position), Scale(scale) {
        SetDirection(directionVec2);
    }
    Camera2D(glm::vec3 position, glm::vec2 scale, float directionRad) : Position(position), Scale(scale), DirectionRad(directionRad) {
        SetDirection(directionRad);
    }

    //Direction stuff
    void SetDirection(float directionRad) {
        DirectionRad = directionRad;
        Direction = glm::vec2(std::cos(directionRad), std::sin(directionRad));
    }

    void SetDirection(glm::vec2 direction) {
        Direction = glm::normalize(direction);
        DirectionRad = glm::atan(direction.y, direction.x);
    }

    inline float GetDirectionRad() {
        return DirectionRad;
    }

    inline glm::vec2 GetDirection() {
        return Direction;
    }

    //Utilities I guess

    glm::vec3 CameraToEntity(Entity &entity, int ScreenWidth, int ScreenHeight, float lerp = 1.0) {
        glm::vec3 Pos = glm::vec3(0.0);
        Pos.x = entity.Position.x + (entity.ScaleLocal.x * entity.ScaleGlobal.x) / 2 - ScreenWidth / 2;
        Pos.y = entity.Position.y + (entity.ScaleLocal.y * entity.ScaleGlobal.y) / 2 - ScreenHeight / 2;

        //Holy lerping
        Pos.x = Position.x * (1.0 - lerp) + Pos.x * lerp;
        Pos.y = Position.y * (1.0 - lerp) + Pos.y * lerp;
        return Pos;
    }

    glm::vec2 DirTowardsEntity_Vec2(Entity &entity, int ScreenWidth, int ScreenHeight) {
        glm::vec2 DirectionVec2 = glm::normalize(glm::vec2(entity.Position.x, entity.Position.y) - glm::vec2(Position.x + ScreenWidth / 2, Position.y + ScreenHeight / 2));
        return DirectionVec2;
    }

    float DirTowardsEntity_Radian(Entity &entity, int ScreenWidth, int ScreenHeight) {
        glm::vec2 DirectionVec2 = glm::normalize(glm::vec2(entity.Position.x, entity.Position.y) - glm::vec2(Position.x + ScreenWidth / 2, Position.y + ScreenHeight / 2));
        return (glm::atan(DirectionVec2.x, DirectionVec2.y));
    }

    //View Matrix Creation

    glm::mat4 GetViewMatrix(int ScreenWidth, int ScreenHeight) {
        glm::mat4 Matrix(1.0f);

        Matrix = glm::translate(Matrix, glm::vec3(ScreenWidth / 2, ScreenHeight / 2, 0.0));
        Matrix = glm::rotate(Matrix, -DirectionRad, glm::vec3(0.0, 0.0, 1.0));
        Matrix = glm::scale(Matrix, glm::vec3(1.0 / Scale.x, 1.0 / Scale.y, 1.0));
        Matrix = glm::translate(Matrix, glm::vec3(-ScreenWidth / 2, -ScreenHeight / 2, 0.0));

        Matrix = glm::translate(Matrix, glm::vec3(-Position.x, -Position.y, 0.0));
        
        return Matrix;
    }
};