#ifndef FILTERS_HPP
#define FILTERS_HPP

#include <opencv2/opencv.hpp>

/**
 * Filters class - Contains CPU implementations of various image filters
 * for real-time video processing and performance comparison with GPU versions.
 */
class Filters {
public:
    /**
     * Apply Sin City style filter - high contrast black and white with selective red color
     * @param input Input image (should be BGR format from OpenCV)
     * @param output Output filtered image
     */
    static void applySinCity(const cv::Mat& input, cv::Mat& output);
    
    /**
     * Apply pixelation filter - creates blocky pixel art effect
     * @param input Input image
     * @param output Output filtered image
     * @param pixelSize Size of each pixel block (default: 10)
     */
    static void applyPixelation(const cv::Mat& input, cv::Mat& output, int pixelSize = 10);
    
    /**
     * Apply affine transformation (translation, rotation, scaling)
     * @param input Input image
     * @param output Output transformed image
     * @param transformMatrix 2x3 affine transformation matrix
     */
    static void applyAffineTransform(const cv::Mat& input, cv::Mat& output, 
                                      const cv::Mat& transformMatrix);
};

#endif // FILTERS_HPP