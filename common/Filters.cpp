#include "Filters.hpp"
#include <algorithm>

void Filters::applySinCity(const cv::Mat& input, cv::Mat& output) {
    if (input.empty()) {
        return;
    }
    
    // Ensure output has the same size and type as input
    output = input.clone();
    
    // Process each pixel
    for (int y = 0; y < input.rows; y++) {
        for (int x = 0; x < input.cols; x++) {
            cv::Vec3b pixel = input.at<cv::Vec3b>(y, x);
            
            // OpenCV uses BGR format
            float b = pixel[0] / 255.0f;
            float g = pixel[1] / 255.0f;
            float r = pixel[2] / 255.0f;
            
            // Calculate grayscale using standard luminance formula
            float gray = 0.299f * r + 0.587f * g + 0.114f * b;
            
            // Detect red areas - check if red is dominant
            float redStrength = r - std::max(g, b);
            bool isRed = (redStrength > 0.2f) && (r > 0.3f);
            
            if (isRed) {
                // Keep red areas colored but enhance them
                // Reduce other channels to emphasize red
                output.at<cv::Vec3b>(y, x)[0] = cv::saturate_cast<uchar>(b * 0.3f * 255);  // B
                output.at<cv::Vec3b>(y, x)[1] = cv::saturate_cast<uchar>(g * 0.3f * 255);  // G
                output.at<cv::Vec3b>(y, x)[2] = cv::saturate_cast<uchar>(r * 1.2f * 255);  // R
            } else {
                // Apply high contrast black and white effect
                float contrast = 1.5f;
                gray = (gray - 0.5f) * contrast + 0.5f;
                gray = std::max(0.0f, std::min(1.0f, gray));
                
                // Apply threshold for stark black/white effect (like comic book)
                float threshold = 0.5f;
                gray = (gray >= threshold) ? 1.0f : 0.0f;
                
                uchar grayValue = cv::saturate_cast<uchar>(gray * 255);
                output.at<cv::Vec3b>(y, x) = cv::Vec3b(grayValue, grayValue, grayValue);
            }
        }
    }
}

void Filters::applyPixelation(const cv::Mat& input, cv::Mat& output, int pixelSize) {
    if (input.empty()) {
        return;
    }
    
    // Ensure pixel size is at least 1
    pixelSize = std::max(1, pixelSize);
    
    output = input.clone();
    
    // Process the image in blocks
    for (int y = 0; y < input.rows; y += pixelSize) {
        for (int x = 0; x < input.cols; x += pixelSize) {
            // Calculate the actual block size (handle edges where block might be cut off)
            int blockHeight = std::min(pixelSize, input.rows - y);
            int blockWidth = std::min(pixelSize, input.cols - x);
            
            // Calculate average color in the block
            cv::Scalar avgColor(0, 0, 0);
            int pixelCount = 0;
            
            for (int by = 0; by < blockHeight; by++) {
                for (int bx = 0; bx < blockWidth; bx++) {
                    cv::Vec3b pixel = input.at<cv::Vec3b>(y + by, x + bx);
                    avgColor[0] += pixel[0];  // B
                    avgColor[1] += pixel[1];  // G
                    avgColor[2] += pixel[2];  // R
                    pixelCount++;
                }
            }
            
            // Calculate average
            avgColor[0] /= pixelCount;
            avgColor[1] /= pixelCount;
            avgColor[2] /= pixelCount;
            
            // Fill the entire block with the average color
            cv::Vec3b fillColor(
                cv::saturate_cast<uchar>(avgColor[0]),
                cv::saturate_cast<uchar>(avgColor[1]),
                cv::saturate_cast<uchar>(avgColor[2])
            );
            
            for (int by = 0; by < blockHeight; by++) {
                for (int bx = 0; bx < blockWidth; bx++) {
                    output.at<cv::Vec3b>(y + by, x + bx) = fillColor;
                }
            }
        }
    }
}

void Filters::applyAffineTransform(const cv::Mat& input, cv::Mat& output, 
                                    const cv::Mat& transformMatrix) {
    if (input.empty() || transformMatrix.empty()) {
        output = input.clone();
        return;
    }
    
    // Apply affine transformation with linear interpolation
    cv::warpAffine(input, output, transformMatrix, input.size(), 
                   cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
}