#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>

/**
 * Transformation class - Handles CPU-based geometric transformations
 * for real-time video processing using OpenCV with interpolation.
 */
class Transformation {
public:
    /**
     * Apply translation transformation
     * @param input Input image
     * @param output Output transformed image
     * @param tx Translation in x direction (in pixels)
     * @param ty Translation in y direction (in pixels)
     */
    static void applyTranslation(const cv::Mat& input, cv::Mat& output, 
                                 float tx, float ty);
    
    /**
     * Apply rotation transformation around image center
     * @param input Input image
     * @param output Output transformed image
     * @param angleDegrees Rotation angle in degrees (counter-clockwise)
     */
    static void applyRotation(const cv::Mat& input, cv::Mat& output, 
                             float angleDegrees);
    
    /**
     * Apply scaling transformation around image center
     * @param input Input image
     * @param output Output transformed image
     * @param scale Scale factor (1.0 = original size, >1.0 = larger, <1.0 = smaller)
     */
    static void applyScaling(const cv::Mat& input, cv::Mat& output, 
                            float scale);
    
    /**
     * Apply combined affine transformation (translation, rotation, scaling)
     * @param input Input image
     * @param output Output transformed image
     * @param tx Translation in x direction
     * @param ty Translation in y direction
     * @param angleDegrees Rotation angle in degrees
     * @param scale Scale factor
     */
    static void applyCombinedTransform(const cv::Mat& input, cv::Mat& output,
                                       float tx, float ty, 
                                       float angleDegrees, float scale);
    
    /**
     * Build a combined affine transformation matrix
     * @param tx Translation in x
     * @param ty Translation in y
     * @param angleDegrees Rotation angle in degrees
     * @param scale Scale factor
     * @param centerX Center of rotation/scaling X (typically image width / 2)
     * @param centerY Center of rotation/scaling Y (typically image height / 2)
     * @return 2x3 affine transformation matrix
     */
    static cv::Mat buildTransformMatrix(float tx, float ty, 
                                        float angleDegrees, float scale,
                                        float centerX, float centerY);
};

#endif // TRANSFORMATION_HPP
