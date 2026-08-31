#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/exponential.hpp>
#include <vector>
#include <cornbreadlib/shaders.h>
#include <cornbreadlib/utility.h>
#include <cornbreadlib/vertexbuffer.h>

class GridSpace {
    public:
    glm::vec2 GridRes;
    glm::vec3 Origin;
    std::vector<int> Data;

    GridSpace(glm::vec2 pixelResolution = glm::vec2(0.0), glm::vec3 origin = glm::vec3(0.0)) {
        GridRes = pixelResolution;
        Origin = origin;
    }

    void AddSquare(int arrayX, int arrayY) {
        Data.push_back(arrayX);
        Data.push_back(arrayY);
    }

    glm::mat4 GetTransformMatrix(unsigned int index) {
        int arrayX = Data.at(index * 2);
        int arrayY = Data.at(index * 2 + 1);
        glm::mat4 Matrix(1.0f);

        Matrix = glm::translate(Matrix, Origin);
        Matrix = glm::translate(Matrix, glm::vec3(GridRes.x * arrayX, GridRes.y * arrayY, 0.0));
        Matrix = glm::scale(Matrix, glm::vec3(GridRes.x, GridRes.y, 1.0));
        
        return Matrix;
    }

    inline glm::vec2 getData(unsigned int index) { return glm::vec2 ( Data.at(index * 2), Data.at(index * 2 + 1) ); }

    inline glm::vec2 getPosition(unsigned int index) {
        return glm::vec2(glm::vec2(Origin.x, Origin.y) + glm::vec2(GridRes.x * getData(index).x, GridRes.y * getData(index).y));
    }

    void RenderAll(Shader &shader, VertexBuffer &vertexbuffer, glm::mat4 &View, glm::mat4 &Projection) {
        for (int i = 0; i < floor(Data.size() / 2.0) ; i++) {
            shader.use();

            glm::mat4 Model = GetTransformMatrix(i);
            shader.setMat4("model", Model);

            shader.setMat4("view", View);

            shader.setMat4("projection", Projection);

            shader.setBool("isSolidColour", true);
            shader.setVec3("Colour", glm::vec3(0.15, 1.0, 0.05));

            vertexbuffer.bind();

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
};