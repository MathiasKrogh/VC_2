#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <chrono>

#include <glad/gl.h>
#define GLAD_GL_IMPLEMENTATION

#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <opencv2/opencv.hpp>

#include <common/Shader.hpp>
#include <common/Camera.hpp>
#include <common/Scene.hpp>
#include <common/Object.hpp>
#include <common/TextureShader.hpp>
#include <common/Quad.hpp>
#include <common/Texture.hpp>
#include <common/Filters.hpp>
#include <common/PixelationShader.hpp>

using namespace std;

// Enums for filter and processing mode selection
enum class FilterType { NONE, SINCITY, PIXELATION };
enum class ProcessingMode { GPU, CPU };

// Global state variables
FilterType currentFilter = FilterType::NONE;
ProcessingMode currentMode = ProcessingMode::GPU;
int pixelSize = 10;

// Track current shader to avoid unnecessary changes
Shader* currentShader = nullptr;

// FPS tracking variables
float fps = 0.0f;
int frameCount = 0;
std::chrono::steady_clock::time_point lastFPSTime;

// Runtime transform parameters
float translateX = 0.0f;
float translateY = 0.0f;
float rotateX = 0.0f;
float scaleFactor = 1.0f;

// Mouse interaction state
bool leftMouseButtonPressed = false;
bool rightMouseButtonPressed = false;

// Helper functions
bool initWindow(std::string windowName);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void printControls();

/* ------------------------------------------------------------------------- */
/* main                                                                      */
/* ------------------------------------------------------------------------- */
int main(void) {
    // --- Step 1: Open camera (OpenCV part) -----------
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Error: Could not open camera. Exiting." << endl;
        return -1;
    }
    cout << "Camera opened successfully." << endl;

    // Optional: Set camera resolution for testing different resolutions
    // cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    // cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

    // --- Step 2: Initialize OpenGL context (GLFW & GLAD) ---
    if (!initWindow("Real-time Video Processing - Assignment 2")) return -1;

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        fprintf(stderr, "Failed to initialize OpenGL context (GLAD)\n");
        cap.release();
        return -1;
    }
    cout << "Loaded OpenGL " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << "\n";

    // Basic OpenGL setup
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glClearColor(0.1f, 0.1f, 0.2f, 0.0f); // A dark blue background
    glEnable(GL_DEPTH_TEST);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // --- Step 3: Prepare Scene, Shaders, and Objects ---------------------
    
    // Get one frame from the camera to determine its size.
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
        cerr << "Error: couldn't capture an initial frame from camera. Exiting.\n";
        cap.release();
        glfwTerminate();
        return -1;
    }
    
    cout << "Captured initial frame: " << frame.cols << "x" << frame.rows << endl;

    // Create multiple shaders for different GPU filters
    TextureShader* passthroughShader = new TextureShader("videoTextureShader.vert", "videoTextureShader.frag");
    TextureShader* sinCityShader = new TextureShader("videoTextureShader.vert", "sinCity.frag");
    PixelationShader* pixelationShader = new PixelationShader("videoTextureShader.vert", "pixelation.frag");
    
    // Create scene and camera
    Scene* myScene = new Scene();
    Camera* renderingCamera = new Camera();
    renderingCamera->setPosition(glm::vec3(0, 0, -2.5)); // Move camera back to see the quad

    // Calculate aspect ratio and create a quad with the correct dimensions.
    float videoAspectRatio = (float)frame.cols / (float)frame.rows;
    Quad* myQuad = new Quad(videoAspectRatio);
    myQuad->setShader(passthroughShader); // Set initial shader (don't change after this!)
    myScene->addObject(myQuad);
    
    // Create OpenGL texture for video frames
    Texture* videoTexture = nullptr;
    cv::flip(frame, frame, 0); // Flip for OpenGL coordinate system
    videoTexture = new Texture(frame.data, frame.cols, frame.rows, true);
    
    cout << "Created video texture" << endl;  
    
    // Set texture for all shaders
    passthroughShader->setTexture(videoTexture);
    sinCityShader->setTexture(videoTexture);
    pixelationShader->setTexture(videoTexture);
    pixelationShader->setPixelSize((float)pixelSize);
    cout << "Shaders configured successfully" << endl;

    // Initialize FPS tracking
    lastFPSTime = std::chrono::steady_clock::now();
    
    // Buffer for CPU-processed frames
    cv::Mat processedFrame;

    // Print control instructions
    printControls();
    
    cout << "Entering main render loop..." << endl;

    // --- Step 4: Main Render Loop ---------------------
    while (!glfwWindowShouldClose(window)) {  
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Check for ESC key press
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // --- Capture and process new frame ---
        cap >> frame;
        if (!frame.empty() && videoTexture != nullptr) {
            cv::flip(frame, frame, 0); // Flip for OpenGL coordinate system
            
            // Apply CPU filters if in CPU mode
            if (currentMode == ProcessingMode::CPU) {
                switch (currentFilter) {
                    case FilterType::SINCITY:
                        Filters::applySinCity(frame, processedFrame);
                        frame = processedFrame;
                        break;
                    case FilterType::PIXELATION:
                        Filters::applyPixelation(frame, processedFrame, pixelSize);
                        frame = processedFrame;
                        break;
                    case FilterType::NONE:
                    default:
                        // No processing needed
                        break;
                }
            }
            // Update GPU texture with (potentially processed) frame
            videoTexture->update(frame.data, frame.cols, frame.rows, true);
        } else {
            cout << "[LOOP] Frame empty or texture null!" << endl;
        }

        // --- Select and manually bind the appropriate shader ---
        
        // We need to manually set which shader the quad will use
        // by directly modifying its shader pointer (hacky but necessary due to setShader deleting)
        if (currentMode == ProcessingMode::GPU) {
            // GPU mode: use appropriate shader for filtering
            switch (currentFilter) {
                case FilterType::SINCITY:
                    currentShader = sinCityShader;
                    break;
                case FilterType::PIXELATION:
                    pixelationShader->setPixelSize((float)pixelSize);
                    currentShader = pixelationShader;
                    break;
                case FilterType::NONE:
                default:
                    currentShader = passthroughShader;
                    break;
            }
        } else {
            // CPU mode: always use passthrough shader (filtering done on CPU)
            currentShader = passthroughShader;
        }

        // --- Render with the selected shader ---
        try {
            // Manually bind the shader we want to use
            currentShader->bind();

            // Apply runtime transformations
            myQuad->setTranslate(glm::vec3(translateX, translateY, 0.0f));
            myQuad->setRotate(rotateX);       // single axis rotation
            myQuad->setScale(scaleFactor);
            
            // Update matrices
            glm::mat4 ModelMatrix = myQuad->getTransform();
            glm::mat4 MVP = renderingCamera->getViewProjectionMatrix() * ModelMatrix;
            currentShader->updateMVP(MVP);
            
            // Render the quad directly (bypassing its internal shader)
            myQuad->directRender();
            
        } catch (const std::exception& e) {
            break;
        }

        // --- FPS calculation and display ---
        frameCount++;
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastFPSTime).count();
        
        if (elapsed >= 1000) { // Update FPS every second
            fps = frameCount / (elapsed / 1000.0f);
            frameCount = 0;
            lastFPSTime = currentTime;
            
            // Print current status
            string mode = (currentMode == ProcessingMode::GPU) ? "GPU" : "CPU";
            string filter = "None";
            if (currentFilter == FilterType::SINCITY) filter = "Sin City";
            else if (currentFilter == FilterType::PIXELATION) filter = "Pixelation (size: " + to_string(pixelSize) + ")";
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup -----------------------------------------------------------
    cout << "Closing application..." << endl;
    cap.release();
    delete myScene;
    delete renderingCamera;
    delete passthroughShader;
    delete sinCityShader;
    delete pixelationShader;
    delete videoTexture;

    glDeleteVertexArrays(1, &VertexArrayID);
    glfwTerminate();
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Helper: initWindow (GLFW)                                                 */
/* ------------------------------------------------------------------------- */
bool initWindow(std::string windowName){
    if (!glfwInit()){
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1024, 768, windowName.c_str(), NULL, NULL);
    if (window == NULL){
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    return true;
}

/* ------------------------------------------------------------------------- */
/* Keyboard callback for interactive control                                 */
/* ------------------------------------------------------------------------- */
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;
    
    switch (key) {
        case GLFW_KEY_1:
            currentFilter = FilterType::NONE;
            cout << "\n>>> Filter: None" << endl;
            break;
        case GLFW_KEY_2:
            currentFilter = FilterType::SINCITY;
            cout << "\n>>> Filter: Sin City" << endl;
            break;
        case GLFW_KEY_3:
            currentFilter = FilterType::PIXELATION;
            cout << "\n>>> Filter: Pixelation" << endl;
            break;
        case GLFW_KEY_G:
            currentMode = ProcessingMode::GPU;
            cout << "\n>>> Mode: GPU Processing" << endl;
            break;
        case GLFW_KEY_C:
            currentMode = ProcessingMode::CPU;
            cout << "\n>>> Mode: CPU Processing" << endl;
            break;
        case GLFW_KEY_H:
            printControls();
            break;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            leftMouseButtonPressed = true;
        } else if (action == GLFW_RELEASE) {
            leftMouseButtonPressed = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            rightMouseButtonPressed = true;
        } else if (action == GLFW_RELEASE) {
            rightMouseButtonPressed = false;
        }
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    const float translateSpeed = 0.05f;  // How fast translation reacts
    const float rotateSpeed = 5.0f;      // How fast rotation reacts (degrees)
    const float scaleSpeed = 0.1f;       // How fast zoom reacts

    if (rightMouseButtonPressed) {
        // Rotate X-axis
        rotateX += static_cast<float>(yoffset) * rotateSpeed;
    } else if (leftMouseButtonPressed) {
        // Translate
        translateY += static_cast<float>(yoffset) * translateSpeed;
        translateX += static_cast<float>(xoffset) * translateSpeed;
    } else {
        // Zoom / scale
        scaleFactor += static_cast<float>(yoffset) * scaleSpeed;
        if (scaleFactor < 0.1f) scaleFactor = 0.1f; // Prevent negative or too small scale
    }
}

/* ------------------------------------------------------------------------- */
/* Print control instructions                                                 */
/* ------------------------------------------------------------------------- */
void printControls() {
    cout << "\n============================================" << endl;
    cout << "         REAL-TIME VIDEO PROCESSING         " << endl;
    cout << "============================================" << endl;
    cout << "FILTERS:" << endl;
    cout << "  1       - No filter (passthrough)" << endl;
    cout << "  2       - Sin City filter" << endl;
    cout << "  3       - Pixelation filter" << endl;
    cout << "\nPROCESSING MODE:" << endl;
    cout << "  G       - GPU processing (shaders)" << endl;
    cout << "  C       - CPU processing (OpenCV)" << endl;
    cout << "\nOTHER:" << endl;
    cout << "  H       - Show this help" << endl;
    cout << "  ESC     - Exit application" << endl;
    cout << "============================================\n" << endl;
}