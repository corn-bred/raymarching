#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class AABBHitbox {
    public:
    glm::vec2 Origin;
    glm::vec2 Scale;
    AABBHitbox(glm::vec2 origin, glm::vec2 scale);
};

bool Collision(AABBHitbox &a, AABBHitbox &b);

float snapToSurfaceX(AABBHitbox &a, AABBHitbox &b, float xVelocity);

float snapToSurfaceY(AABBHitbox &a, AABBHitbox &b, float yVelocity);