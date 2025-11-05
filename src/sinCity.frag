#version 330 core
in vec2 UV;
out vec4 FragColor;
uniform sampler2D myTextureSampler;

void main() {
    vec4 color = texture(myTextureSampler, UV);
    
    // Convert to grayscale using luminance formula
    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    
    // Detect red areas (for selective colorization)
    // If the pixel has strong red component and low other components, keep it colored
    float redStrength = color.r - max(color.g, color.b);
    bool isRed = redStrength > 0.2 && color.r > 0.3;
    
    // Enhance contrast for dramatic black and white
    float contrast = 1.5;
    gray = (gray - 0.5) * contrast + 0.5;
    gray = clamp(gray, 0.0, 1.0);
    
    // Apply threshold for stark black/white effect
    float threshold = 0.5;
    gray = step(threshold, gray);
    
    if (isRed) {
        // Keep red areas colored but enhance them
        FragColor = vec4(color.r * 1.2, color.g * 0.3, color.b * 0.3, 1.0);
    } else {
        // Make everything else high-contrast black and white
        FragColor = vec4(gray, gray, gray, 1.0);
    }
}