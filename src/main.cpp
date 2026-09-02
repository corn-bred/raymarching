#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cornbreadlib/camera.h>
#include <cornbreadlib/computeshader.h>
#include <cornbreadlib/shaders.h>
#include <cornbreadlib/vertexbuffer.h>

using namespace std;

double AspectRatio = 16.0/9.0;
unsigned int WIDTH = 1200;
unsigned int HEIGHT = float(WIDTH) / AspectRatio;
float FOV = 100.0;
float DeltaTime, LastFrame;
unsigned int FPSCounter, ShownFPS;
int FrameIndex = 0;

float quadVertices[] = {  
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

Camera CameraMain(glm::vec3(0.0, 0.0, 5.05));

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, Camera &camera) { //Spaghetti code GO
    const float cameraSpeed = 2.5f;
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        FrameIndex = 0;
    }
    bool movements[6] = {false}; //W:0 S:1 A:2 D:3 SPACE:4 CONTROL:5
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        movements[0] = true;
        FrameIndex = 0;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        movements[1] = true;
        FrameIndex = 0;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        movements[2] = true;
        FrameIndex = 0;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        movements[3] = true;
        FrameIndex = 0;
    }
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        movements[4] = true;
        FrameIndex = 0;
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        movements[5] = true;
        FrameIndex = 0;
    }
    camera.keyboardprocess(movements, DeltaTime, cameraSpeed);
}

float LastX = -1.0, LastY = -1.0;

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    float xoffset = 0;
    float yoffset = 0;
    if (LastX != -1.0 && LastY != -1.0) {
        xoffset = xpos - LastX;
        yoffset = LastY - ypos;
    }
    
    LastX = xpos;
    LastY = ypos;
    
    CameraMain.mouseprocess(xoffset, yoffset, GL_TRUE);
    FrameIndex = 0;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    FOV -= (float)yoffset;
    if (FOV < 1.0f)
        FOV = 1.0f;
    if (FOV > 170.0f)
        FOV = 170.0f;
    FrameIndex = 0;
}


int main() {
    if (!glfwInit()) {
        cerr << "GLFW initialization failure\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Raymarcher (Starting)", NULL, NULL);
    if (!window) {
        cerr << "GLFW window creation failure\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    glfwSwapInterval(0);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    GLFWvidmode *mode = const_cast<GLFWvidmode*>(glfwGetVideoMode(monitor));
    glfwSetWindowPos(window, (mode->width - WIDTH)/2, (mode->height - HEIGHT)/2);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
    glfwSetCursorPosCallback(window, mouseCallback);
    CameraMain.mouseprocess(0, 0, GL_TRUE);

    glfwSetScrollCallback(window, scrollCallback);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        cerr << "GLAD initialization failure\n";
        glfwTerminate();
        return 1;
    }

    GLuint OutputTexture;

    glGenTextures(1, &OutputTexture);
    glBindTexture(GL_TEXTURE_2D, OutputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    ComputeShader Raymarcher("src/shaders/raymarcher.comp");

    Shader ViewportRenderer("src/shaders/sample.vert", "src/shaders/sample.frag");

    VertexBuffer vboQuad(&quadVertices, sizeof(quadVertices), GL_STATIC_DRAW);
    vboQuad.addAttribute(0, 2, GL_FLOAT, 4, 0);
    vboQuad.addAttribute(1, 2, GL_FLOAT, 4, 2);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processInput(window, CameraMain);
        FrameIndex++;

        float CurrentFrame = glfwGetTime();
        DeltaTime = CurrentFrame - LastFrame;
        if (floor(CurrentFrame) != floor(LastFrame)) {
            ShownFPS = FPSCounter;
            FPSCounter = 0;
        }

        stringstream titlestring;
        titlestring << "Raymarcher (FPS: " << ShownFPS << ")";

        glfwSetWindowTitle(window, titlestring.str().c_str()); 
        LastFrame = CurrentFrame;

        //cout << "Delta: " << DeltaTime * 1000 << " ms" << endl;

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective((float)glm::radians(FOV), (float)AspectRatio, 0.1f, 100.0f);

        CameraMain.updateCamera();
        
        glm::mat4 view = CameraMain.calculateView();

        Raymarcher.bind();

        glBindImageTexture(0, OutputTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

        Raymarcher.setInt("WIDTH", WIDTH);
        Raymarcher.setInt("HEIGHT", HEIGHT);
        Raymarcher.setMat4("invProjection", glm::inverse(projection));
        Raymarcher.setMat4("invView", glm::inverse(view));
        Raymarcher.setVec3("CameraPos", CameraMain.position);
        
        Raymarcher.setInt("tempobj.ObjectType", 1);
        Raymarcher.setVec3("tempobj.Position", glm::vec3(0.0));
        Raymarcher.setFloat("tempobj.Radius", 1.0);

        Raymarcher.use((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);  

        ViewportRenderer.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, OutputTexture);
        ViewportRenderer.setInt("Output", 0);

        vboQuad.bind();

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        
        FPSCounter++;
    }

    glfwTerminate();

    return 0;
}