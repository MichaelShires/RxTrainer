// Visualization.cpp

#include "Visualization.h"
#include <iostream>
#include <vector>
#include "Core.h"
#include <cmath>    // For trigonometric functions
#include <limits>   // For min and max temperature tracking

void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error [" << error << "]: " << description << std::endl;
}

// Updated Vertex Shader Source
const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in float aValue;

out float value;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
    gl_PointSize = 30.0; // Set point size here
    value = aValue;
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 330 core
in float value;
out vec4 FragColor;

void main()
{
    // Map value to color gradient from blue to green to red
    vec3 color;
    if (value < 0.5) {
        // From blue to green
        color = mix(vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 0.0), value * 2.0);
    } else {
        // From green to red
        color = mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), (value - 0.5) * 2.0);
    }
    FragColor = vec4(color, 1.0);
}
)glsl";

Visualization::Visualization(Core& core, CoolantLoop& coolantLoop, std::atomic<bool>& running)
    : core(core),
      coolantLoop(coolantLoop),
      running(running),
      coreMutex(core.getMutex()),
      coolantMutex(coolantLoop.getMutex()),
      window(nullptr),
      shaderProgram(0),
      VAO_core(0),
      VBO_core(0),
      VAO_coolant(0),
      VBO_coolant(0) {
    // Constructor body
}

Visualization::~Visualization() {
    // Clean up resources if necessary
}

void Visualization::start() {
    renderLoop();
}

void Visualization::renderLoop() {
    std::cerr << "Visualization render loop started." << std::endl;

    glfwSetErrorCallback(glfwErrorCallback);

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return;
    } else {
        std::cerr << "GLFW initialized successfully." << std::endl;
    }

    // Set OpenGL version to 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // For macOS compatibility
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(800, 600, "Reactor Simulation Visualization", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return;
    } else {
        std::cerr << "GLFW window created successfully." << std::endl;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);
    std::cerr << "OpenGL context made current." << std::endl;

    // Initialize GLAD
    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return;
    } else {
        std::cerr << "GLAD initialized successfully." << std::endl;
    }

    // Set up viewport
    glViewport(0, 0, 800, 600);

    // Enable point size for gl_PointSize in vertex shader
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Set up shaders and other OpenGL settings
    if (!setupShaders()) {
        std::cerr << "Failed to set up shaders." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return;
    } else {
        std::cerr << "Shaders set up successfully." << std::endl;
    }

    setupBuffers();

    std::cerr << "Entering main render loop." << std::endl;

    // Main render loop
    while (running.load() && !glfwWindowShouldClose(window)) {
        // Handle events
        glfwPollEvents();

        // Clear the window
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the simulation components
        drawCore();
        drawCoolantLoop();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Sleep to limit frame rate (e.g., ~60 FPS)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    std::cerr << "Exiting render loop." << std::endl;

    // Clean up
    cleanup();
}

bool Visualization::setupShaders() {
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Check for compilation errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }

    // Link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return false;
    }

    // Delete shader objects after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}

void Visualization::setupBuffers() {
    // Generate VAO and VBO for core visualization
    glGenVertexArrays(1, &VAO_core);
    glGenBuffers(1, &VBO_core);

    // Generate VAO and VBO for coolant loop visualization
    glGenVertexArrays(1, &VAO_coolant);
    glGenBuffers(1, &VBO_coolant);
}

void Visualization::cleanup() {
    // Delete OpenGL resources
    glDeleteVertexArrays(1, &VAO_core);
    glDeleteBuffers(1, &VBO_core);

    glDeleteVertexArrays(1, &VAO_coolant);
    glDeleteBuffers(1, &VBO_coolant);

    glDeleteProgram(shaderProgram);

    // Destroy window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Visualization::drawCore() {
    // Acquire lock and copy necessary data
    int xSize, ySize, zSize;
    std::vector<CoreElement> elementsCopy;
    {
        std::lock_guard<std::mutex> lock(coreMutex);
        xSize = core.getXSize();
        ySize = core.getYSize();
        zSize = core.getZSize();
        elementsCopy = core.getElements(); // Copy elements
    } // Lock is released here

    int zSlice = zSize / 2; // Visualize the middle slice

    std::vector<float> vertices; // x, y, value

    double minTemp = std::numeric_limits<double>::max();
    double maxTemp = std::numeric_limits<double>::lowest();

    for (int x = 0; x < xSize; ++x) {
        for (int y = 0; y < ySize; ++y) {
            int idx = x + y * xSize + zSlice * xSize * ySize; // Calculate index without core.index()

            const CoreElement& element = elementsCopy[idx];

            double temp = element.getTemperature();
            minTemp = std::min(minTemp, temp);
            maxTemp = std::max(maxTemp, temp);

            // Adjust the normalization range as per your simulation's temperature range
            float normalizedValue = static_cast<float>((temp - 300.0) / (1000.0 - 300.0));
            normalizedValue = std::clamp(normalizedValue, 0.0f, 1.0f);

            // Normalize positions to range [-1, 1]
            float normX = (static_cast<float>(x) / (xSize - 1)) * 2.0f - 1.0f;
            float normY = (static_cast<float>(y) / (ySize - 1)) * 2.0f - 1.0f;

            vertices.push_back(normX);
            vertices.push_back(normY);
            vertices.push_back(normalizedValue);
        }
    }

    // Log temperature range
    static int frameCountCore = 0;
    if (frameCountCore % 60 == 0) { // Log every 60 frames (~1 second)
       // std::cout << "Core Temperature Range: Min = " << minTemp << " K, Max = " << maxTemp << " K" << std::endl;
    }
    frameCountCore++;

    // Update buffer data
    glBindVertexArray(VAO_core);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_core);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    // Set vertex attribute pointers
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Value attribute
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Use shader program
    glUseProgram(shaderProgram);

    // Set projection matrix
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

    // Draw points
    glPointSize(30.0f); // Increased point size
    glDrawArrays(GL_POINTS, 0, vertices.size() / 3);

    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error in drawCore(): " << error << std::endl;
    }
}

void Visualization::drawCoolantLoop() {
    // Variables to store copied data
    std::vector<double> temperatures;
    int chunkCount;

    // Acquire lock and copy necessary data
    {
        std::lock_guard<std::mutex> lock(coolantMutex);
        const auto& chunks = coolantLoop.getChunks();
        chunkCount = chunks.size();
        temperatures.reserve(chunkCount);

        for (const auto& chunk : chunks) {
            temperatures.push_back(chunk.getTemperature());
        }
    } // Lock is released here

    // Now use the copied temperatures for rendering
    std::vector<float> vertices; // x, y, value

    double minTemp = std::numeric_limits<double>::max();
    double maxTemp = std::numeric_limits<double>::lowest();

    int index = 0;
    for (const auto& temp : temperatures) {
        minTemp = std::min(minTemp, temp);
        maxTemp = std::max(maxTemp, temp);

        // Normalize temperature value
        float normalizedValue = static_cast<float>((temp - 300.0) / (600.0 - 300.0));
        normalizedValue = std::clamp(normalizedValue, 0.0f, 1.0f);

        // Arrange positions in a circular loop
        float angle = (static_cast<float>(index) / chunkCount) * 2.0f * 3.1415926f; // Angle in radians
        float radius = 0.8f; // Radius of the loop

        float normX = radius * cos(angle);
        float normY = radius * sin(angle);

        vertices.push_back(normX);
        vertices.push_back(normY);
        vertices.push_back(normalizedValue);

        index++;
    }

    // Log temperature range
    static int frameCountCoolant = 0;
    if (frameCountCoolant % 60 == 0) { // Log every 60 frames (~1 second)
        //std::cout << "Coolant Temperature Range: Min = " << minTemp << " K, Max = " << maxTemp << " K" << std::endl;
    }
    frameCountCoolant++;

    // Update buffer data
    glBindVertexArray(VAO_coolant);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_coolant);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    // Set vertex attribute pointers
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Value attribute
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Use shader program
    glUseProgram(shaderProgram);

    // Set projection matrix
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

    // Draw points
    glPointSize(10.0f); // Adjust point size as needed
    glDrawArrays(GL_POINTS, 0, vertices.size() / 3);

    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error in drawCoolantLoop(): " << error << std::endl;
    }
}