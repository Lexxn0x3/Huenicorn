#include <FreenSync/ImageProcessing.hpp>

#include <algorithm>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace std;

namespace ImageProcessing{
  void rescale(cv::Mat& img, int targetWidth)
  {
    int sourceHeight = img.rows;
    int sourceWidth = img.cols;

    if(sourceWidth < targetWidth){
      return;
    }

    float scaleRatio = static_cast<float>(targetWidth) / sourceWidth;

    int targetHeight = sourceHeight * scaleRatio;
    cv::resize(img, img, cv::Size(targetWidth, targetHeight), 0, 0, cv::InterpolationFlags::INTER_LINEAR);
  }


  cv::Mat getSubImage(const cv::Mat& sourceImage, const glm::ivec2& a, const glm::ivec2& b)
  {
    cv::Range cols(std::max(0, a.x), std::min(b.x, sourceImage.cols));
    cv::Range rows(std::max(0, a.y), std::min(b.y, sourceImage.rows));

    return sourceImage(rows, cols);
  }


  Colors getDominantColors(cv::Mat& img, unsigned k)
  {
    Colors dominantColors;
    dominantColors.reserve(k);
    cv::Mat data = img.reshape(1, img.total());
    data.convertTo(data, CV_32F);

    cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 10, 1.0);

    vector<int> labels;
    cv::Mat centers;
    int flags = cv::KmeansFlags::KMEANS_PP_CENTERS;
    //int flags = cv::KmeansFlags::KMEANS_RANDOM_CENTERS;
    double compactness = cv::kmeans(data, k, labels, criteria, 10, flags, centers);

    centers.convertTo(centers, CV_8U);

    for(int i = 0; i < centers.rows; i++){
      const uint8_t* Mi = centers.ptr<uint8_t>(i);
      dominantColors.emplace_back(Mi[2], Mi[1], Mi[0]);
    }

    return dominantColors;
  }
}
