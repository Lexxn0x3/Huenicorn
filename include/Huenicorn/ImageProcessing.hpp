#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

#include <Huenicorn/Color.hpp>


namespace Huenicorn
{
  using Colors = std::vector<Color>;

  namespace ImageProcessing
  {
    void rescale(cv::Mat& image, int targetWidth);

    cv::Mat getSubImage(const cv::Mat& sourceImage, const glm::ivec2& a, const glm::ivec2& b);

    Colors getDominantColors(cv::Mat& image, unsigned k = 1);
  };
}
