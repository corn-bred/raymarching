#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/exponential.hpp>
#include <cornjam/collision.h>
#include <cornjam/grid.h>
#include <cornjam/inputmanager.h>
#include <misc/globals.h>


class Entity {
    protected:
    glm::vec2 Direction;
    float DirectionRad;
    public:
    glm::vec3 Position;
    glm::vec2 Velocity;
    
    glm::vec2 ScaleLocal;
    glm::vec2 ScaleGlobal;

    void SetDirection(float directionRad) {
        DirectionRad = directionRad;
        Direction = glm::vec2(std::cos(directionRad), std::sin(directionRad));
    }

    void SetDirection(glm::vec2 direction) {
        Direction = direction;
        DirectionRad = glm::atan(direction.y, direction.x);
    }
};

class Player : public Entity {
    public:
    float WallStickStrength; //Strength of being able to stay on a wall
    glm::vec2 MinijumpPushoff;
    int CollisionAxes[2] = {0, 0}; //{x, y}
    glm::vec2 Resistance, AirResistance;
    glm::vec2 Speed, TerminalSpeed;
    AABBHitbox Hitbox;

    Player(glm::vec3 position = glm::vec3(0.0), float directionRad = 0.0, glm::vec2 speed = glm::vec2(0.0), glm::vec2 velocity = glm::vec2(0.0), glm::vec2 terminalSpeed = glm::vec2(1.0), glm::vec2 resistance = glm::vec2(1.0), glm::vec2 airResistance = glm::vec2(1.0), glm::vec2 scaleLocal = glm::vec2(1.0), glm::vec2 scaleGlobal = glm::vec2(1.0), glm::vec2 hitboxPosition = glm::vec2(0.0), glm::vec2 hitboxSize = glm::vec2(1.0), glm::vec2 minijumpPushoff = glm::vec2(1.0, 1.0), float wallStickStrength = 1.0f) : Hitbox(hitboxPosition + glm::vec2(position.x, position.y), hitboxSize) {
        Position = position;
        Resistance = resistance;
        AirResistance = airResistance;
        Speed = speed;
        TerminalSpeed = terminalSpeed;
        DirectionRad = directionRad;
        Direction = glm::vec2(std::cos(DirectionRad), std::sin(DirectionRad));
        ScaleLocal = scaleLocal;
        ScaleGlobal = scaleGlobal;
        MinijumpPushoff = minijumpPushoff;
        WallStickStrength = wallStickStrength;
    }

    void VeloUpdate(GridSpace &grid, int searchRadius, float gravity) { //If anything is farther than searchRadius, discard it from collision testing.
        //First, clamp the velocities
        Velocity.x = glm::clamp(Velocity.x, -TerminalSpeed.x, TerminalSpeed.x);

        
        CollisionAxes[0] = 0;

        Position.x += Velocity.x * (float)(DeltaTime);
        Hitbox.Origin.x = Position.x;

        for(int i = 0; i < (int)grid.Data.size() / 2; i++) { //Cycling through each piece of data, inefficient but works for now

            glm::vec2 DataPosition = grid.getPosition(i); //Get data

            glm::vec2 Diff = glm::abs(DataPosition - glm::vec2(Position.x, Position.y));

            if (Diff.x <= searchRadius && Diff.y <= searchRadius) { //if distance between the grid and the entity < searchRadius, then test. If not, then discard
                AABBHitbox gridHitbox(DataPosition, grid.GridRes);
                if(Collision(Hitbox, gridHitbox)) {
                    float Snapped = snapToSurfaceX(Hitbox, gridHitbox, Velocity.x);
                    Position.x += Snapped;
                    Velocity.x = 0;

                    if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
                        CollisionAxes[0] = -1;
                    else if (Snapped < 0)
                        CollisionAxes[0] = 1;
                        
                }
            }
            
        }
        Hitbox.Origin.x = Position.x; //Update hitbox
        

        CollisionAxes[1] = 0;

        Position.y += Velocity.y * (float)(DeltaTime);
        Hitbox.Origin.y = Position.y;

        for(int i = 0; i < (int)grid.Data.size() / 2; i++) { //Cycling through each piece of data, inefficient but works for now

            glm::vec2 DataPosition = grid.getPosition(i); //Get data

            glm::vec2 Diff = glm::abs(DataPosition - glm::vec2(Position.x, Position.y));

            if (Diff.x <= searchRadius && Diff.y <= searchRadius) { //if distance between the grid and the entity < searchRadius, then test. If not, then discard
                AABBHitbox gridHitbox(DataPosition, grid.GridRes);
                if(Collision(Hitbox, gridHitbox)) {
                    float Snapped = snapToSurfaceY(Hitbox, gridHitbox, Velocity.y);
                    Position.y += Snapped;
                    Velocity.y = 0;

                    if (Snapped > 0)
                        CollisionAxes[1] = -1;
                    else if (Snapped < 0)
                        CollisionAxes[1] = 1;

                }
            }
            
        }
        Hitbox.Origin.y = Position.y; //Update hitbox
        //std::cout << CollisionAxes[0] << ", " << CollisionAxes[1] << std::endl;

        //Update velocities for next loop
        if(CollisionAxes[1] != 0) {
            Velocity.x = Velocity.x * glm::pow(Resistance.x, DeltaTime * 60);
        } else {
            Velocity.x = Velocity.x * glm::pow(0.99, DeltaTime * 60);
        }
        
        //Velocity.y = Velocity.y * glm::pow(Resistance.y, deltaTime * 60);

        if (CollisionAxes[0] != 0 && CollisionAxes[1] == 0) {
            Velocity.y = glm::clamp( float(Velocity.y - (gravity * DeltaTime)), -gravity / (Speed.x * WallStickStrength), gravity / (Speed.x * WallStickStrength));
        } else {
            Velocity.y = Velocity.y - (gravity * DeltaTime);
        }
        
        //std::cout << Position.x << ", " << Position.y << " : ";
        //std::cout << Hitbox.Origin.x << ", " << Hitbox.Origin.y << std::endl;
    }

    glm::mat4 GetTransformMatrix() {
        glm::mat4 Matrix(1.0f);

        Matrix = glm::translate(Matrix, glm::vec3(Position.x, Position.y, 0.0));
        Matrix = glm::scale(Matrix, glm::vec3(ScaleGlobal.x, ScaleGlobal.y, 1.0));
        Matrix = glm::rotate(Matrix, DirectionRad, glm::vec3(0.0, 0.0,1.0));
        Matrix = glm::scale(Matrix, glm::vec3(ScaleLocal.x, ScaleLocal.y, 1.0));
        
        return Matrix;
    }

    void KeyboardUpdate (InputManager &input) {
        if (input.isActionPressed(Action::MoveLeft)) {
            if (CollisionAxes[1] != 0)
                Velocity.x += -Speed.x * (DeltaTime * 60);
            else {
                Velocity.x += -Speed.x * (1 - (AirResistance.x / 2)) * (DeltaTime * 60);
            }
        }

        if (input.isActionPressed(Action::MoveRight)) {
            if (CollisionAxes[1] != 0)
                Velocity.x += Speed.x * (DeltaTime * 60);
            else
                Velocity.x += Speed.x * (1 - (AirResistance.x / 2)) * (DeltaTime * 60);
        }

        if (input.isActionPressedWithin(Action::Jump, 0.1f)) {
            if (CollisionAxes[1] == -1)
                Velocity.y = Speed.y;
            else if (CollisionAxes[0] != 0 && CollisionAxes[1] == 0) { //To stop climb bugging
                Velocity.y = Speed.y * MinijumpPushoff.y;
                Velocity.x = TerminalSpeed.x * -CollisionAxes[0] * MinijumpPushoff[0];
            }
        }
    }
};