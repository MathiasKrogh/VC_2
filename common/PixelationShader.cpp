#include "PixelationShader.hpp"

PixelationShader::PixelationShader(std::string vertexShaderName, std::string fragmentShaderName) 
    : TextureShader(vertexShaderName, fragmentShaderName), currentPixelSize(10.0f) {
    // Get uniform location after shader is compiled
    pixelSizeLocation = glGetUniformLocation(programID, "pixelSize");
    
    if (pixelSizeLocation == -1) {
        printf("Warning: Could not find 'pixelSize' uniform in pixelation shader\n");
    }
}

void PixelationShader::setPixelSize(float size) {
    currentPixelSize = size;
}

void PixelationShader::bind() {
    // Call parent bind first to activate the shader program
    TextureShader::bind();
    
    // Then set our custom uniform
    if (pixelSizeLocation != -1) {
        glUniform1f(pixelSizeLocation, currentPixelSize);
    }
}