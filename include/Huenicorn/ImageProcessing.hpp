#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

#include <Huenicorn/Color.hpp>


namespace Huenicorn
{
  using Colors = std::vector<Color>;

  /**
   * @brief Provides image manipulation functions
   * 
   */
  namespace ImageProcessing
  {
    /**
     * @brief Outputs a resampled bitmap of an input bitmap
     * 
     * @param image Input bitmap
     * @param targetWidth Target width of the output bitmap
     */
    void rescale(cv::Mat& image, int targetWidth);


    /**
     * @brief Outputs a rectangular portion of the source Image
     * 
     * @param sourceImage Input image
     * @param a Top-left coordinates
     * @param b Bottom-right coordinates
     * @return cv::Mat Crop of the image
     */
    cv::Mat getSubImage(const cv::Mat& sourceImage, const glm::ivec2& a, const glm::ivec2& b);


    /**
     * @brief Get the Dominant Colors object
     * 
     * @param image Input image
     * @param k Amount of dominant colors to output
     * @return Colors List of dominant colors
     */
    Colors getDominantColors(cv::Mat& image, unsigned k = 1);


    namespace Algorithms
    {
      Colors kMeans(const cv::Mat& image, unsigned k = 1);
      Colors mean(const cv::Mat& image, unsigned);
    }
  };
}
