#version 330 core
in vec2 UV;
out vec4 FragColor;

uniform sampler2D myTextureSampler;
uniform float pixelSize; // Size of the "pixels" - default could be 10.0

void main() {
    // Get the texture dimensions
    vec2 imageSize = vec2(textureSize(myTextureSampler, 0));
    
    // Calculate the size of one pixel block in UV coordinates (0.0 to 1.0 range)
    vec2 pixelBlock = vec2(pixelSize) / imageSize;
    
    // Snap UV coordinates to the pixel block grid
    // This makes all pixels in a block sample from the same location
    vec2 snappedUV = floor(UV / pixelBlock) * pixelBlock;
    
    // Add half a block offset to sample from the center of the block
    snappedUV += pixelBlock * 0.5;
    
    // Sample the texture at the snapped coordinates
    FragColor = texture(myTextureSampler, snappedUV);
}