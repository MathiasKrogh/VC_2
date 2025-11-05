#include "Transformation.hpp"
#include <cmath>

void Transformation::applyTranslation(const cv::Mat& input, cv::Mat& output, 
                                      float tx, float ty) {
    if (input.empty()) {
        output = input.clone();
        return;
    }
    
    // Create translation matrix
    cv::Mat translationMat = (cv::Mat_<float>(2, 3) << 
        1, 0, tx,
        0, 1, ty);
    
    // Apply affine transformation with linear interpolation
    cv::warpAffine(input, output, translationMat, input.size(), 
                   cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
}

void Transformation::applyRotation(const cv::Mat& input, cv::Mat& output, 
                                   float angleDegrees) {
    if (input.empty()) {
        output = input.clone();
        return;
    }
    
    // Get rotation center (image center)
    cv::Point2f center(input.cols / 2.0f, input.rows / 2.0f);
    
    // Get rotation matrix (OpenCV uses counter-clockwise rotation for positive angles)
    cv::Mat rotationMat = cv::getRotationMatrix2D(center, angleDegrees, 1.0);
    
    // Apply affine transformation with linear interpolation
    cv::warpAffine(input, output, rotationMat, input.size(), 
                   cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
}

void Transformation::applyScaling(const cv::Mat& input, cv::Mat& output, 
                                  float scale) {
    if (input.empty()) {
        output = input.clone();
        return;
    }
    
    // Get image center for scaling around center
    float centerX = input.cols / 2.0f;
    float centerY = input.rows / 2.0f;
    
    // Build scaling matrix that scales around the center
    // Translation to origin, scale, translation back
    cv::Mat scaleMat = (cv::Mat_<float>(2, 3) << 
        scale, 0, centerX * (1 - scale),
        0, scale, centerY * (1 - scale));
    
    // Apply affine transformation with linear interpolation
    cv::warpAffine(input, output, scaleMat, input.size(), 
                   cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
}

void Transformation::applyCombinedTransform(const cv::Mat& input, cv::Mat& output,
                                            float tx, float ty, 
                                            float angleDegrees, float scale) {
    if (input.empty()) {
        output = input.clone();
        return;
    }
    
    float centerX = input.cols / 2.0f;
    float centerY = input.rows / 2.0f;
    
    // Build combined transformation matrix
    cv::Mat transformMat = buildTransformMatrix(tx, ty, angleDegrees, scale, 
                                                centerX, centerY);
    
    // Apply affine transformation with linear interpolation
    cv::warpAffine(input, output, transformMat, input.size(), 
                   cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
}

cv::Mat Transformation::buildTransformMatrix(float tx, float ty, 
                                             float angleDegrees, float scale,
                                             float centerX, float centerY) {
    // Convert angle to radians
    float angleRad = angleDegrees * CV_PI / 180.0f;
    float cosA = std::cos(angleRad);
    float sinA = std::sin(angleRad);
    
    // Combined transformation matrix:
    // 1. Translate to origin (move center to 0,0)
    // 2. Scale
    // 3. Rotate
    // 4. Translate back to original position
    // 5. Apply additional translation (tx, ty)
    
    // The combined 2x3 affine matrix is:
    // [scale*cos(a)  -scale*sin(a)  tx - centerX*scale*cos(a) + centerY*scale*sin(a) + centerX]
    // [scale*sin(a)   scale*cos(a)  ty - centerX*scale*sin(a) - centerY*scale*cos(a) + centerY]
    
    cv::Mat transformMat = (cv::Mat_<float>(2, 3) << 
        scale * cosA, -scale * sinA, 
        tx - centerX * scale * cosA + centerY * scale * sinA + centerX,
        
        scale * sinA, scale * cosA, 
        ty - centerX * scale * sinA - centerY * scale * cosA + centerY);
    
    return transformMat;
}