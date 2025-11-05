#ifndef PIXELATION_SHADER_HPP
#define PIXELATION_SHADER_HPP

#include "TextureShader.hpp"
#include <string>

class PixelationShader : public TextureShader {
private:
    GLint pixelSizeLocation;
    float currentPixelSize;

public:
    PixelationShader(std::string vertexShaderName, std::string fragmentShaderName);
    
    void setPixelSize(float size);
    
    void bind() override;
};

#endif // PIXELATION_SHADER_HPP