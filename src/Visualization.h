#ifndef VISUALIZATION_H
#define VISUALIZATION_H

class Core;

#include <thread>
#include <atomic>
#include <mutex>
#include "CoolantLoop.h"

// Include OpenGL and GLFW headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Include GLM for matrix operations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Visualization {
public:
    Visualization(Core& core, CoolantLoop& coolantLoop, std::atomic<bool>& running);
    ~Visualization();

    void start();

private:
    void renderLoop();

    Core& core;
    CoolantLoop& coolantLoop;
    std::atomic<bool>& running;

    GLFWwindow* window;

    // Synchronization
    std::mutex& coreMutex;
    std::mutex& coolantMutex;

    // OpenGL-related
    GLuint shaderProgram;
    GLuint VAO_core, VBO_core;
    GLuint VAO_coolant, VBO_coolant;

    // Methods for drawing
    void drawCore();
    void drawCoolantLoop();
    void drawUI();

    // OpenGL setup methods
    bool initializeOpenGL();
    bool setupShaders();
    void setupBuffers();
    void cleanup();
};

#endif // VISUALIZATION_H