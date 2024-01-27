#include <Huenicorn/ImageProcessing.hpp>


namespace Huenicorn
{
  namespace ImageProcessing
  {
    void rescale(cv::Mat& image, int targetWidth)
    {
      int sourceHeight = image.rows;
      int sourceWidth = image.cols;

      if(sourceWidth < targetWidth){
        return;
      }

      float scaleRatio = static_cast<float>(targetWidth) / sourceWidth;

      int targetHeight = sourceHeight * scaleRatio;
      cv::resize(image, image, cv::Size(targetWidth, targetHeight), 0, 0, cv::InterpolationFlags::INTER_LINEAR);
    }


    cv::Mat getSubImage(const cv::Mat& sourceImage, const glm::ivec2& a, const glm::ivec2& b)
    {
      cv::Range cols(std::max(0, a.x), std::min(b.x, sourceImage.cols));
      cv::Range rows(std::max(0, a.y), std::min(b.y, sourceImage.rows));

      return sourceImage(rows, cols);
    }


    Colors getDominantColors(cv::Mat& image, unsigned k)
    {
      if(image.cols < 1 || image.rows < 1){
        return {Color(0, 0, 0)};
      }
    
      return Algorithms::mean(image, k);
    }


    namespace Algorithms
    {
      Colors kMeans(const cv::Mat& image, unsigned k)
      {
        Colors dominantColors;
        dominantColors.reserve(k);
        cv::Mat data = image.reshape(1, image.total());
        data.convertTo(data, CV_32F);

        cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 10, 1.0);

        std::vector<int> labels;
        cv::Mat centers;
        int flags = cv::KmeansFlags::KMEANS_PP_CENTERS;
        //int flags = cv::KmeansFlags::KMEANS_RANDOM_CENTERS;
        cv::kmeans(data, k, labels, criteria, 10, flags, centers);

        centers.convertTo(centers, CV_8U);

        for(int i = 0; i < centers.rows; i++){
          const uint8_t* Mi = centers.ptr<uint8_t>(i);
          dominantColors.emplace_back(Mi[2], Mi[1], Mi[0]);
        }

        return dominantColors;
      }


      Colors mean(const cv::Mat& image, unsigned /*k*/)
      {
        cv::Mat data = image.reshape(3, image.total());
        auto mean = cv::mean(data);

        return Colors{
          {
            static_cast<uint8_t>(mean[2]),
            static_cast<uint8_t>(mean[1]),
            static_cast<uint8_t>(mean[0])
          }
        };
      }
    }
  }
}
