#include <filesystem>
#include <fstream>
#include <unordered_map>

#include <Huenicorn/ImageProcessing.hpp>
#include <Huenicorn/Logger.hpp>

int main(int argc, char* argv[])
{
  std::unordered_map<Huenicorn::Interpolation::Type, std::filesystem::path> testOutputs = {
    {Huenicorn::Interpolation::Type::Nearest, "Nearest"},
    {Huenicorn::Interpolation::Type::Cubic, "Cubic"},
    {Huenicorn::Interpolation::Type::Area, "Area"}
  };

  if(argc < 2){
    Huenicorn::Logger::error("Please provide path to images");
    return 0;
  }

  std::filesystem::path samplesDirPath(argv[1]);

  std::filesystem::path samplesInputDirPath = samplesDirPath / "Inputs";
  std::filesystem::path samplesOutputDirPath = samplesDirPath / "Outputs";

  if(!std::filesystem::exists(samplesDirPath) || !std::filesystem::is_directory(samplesDirPath)){
    Huenicorn::Logger::error("Invalid folder " + std::string(samplesInputDirPath));
    return 0;
  }

  for(const auto& entry : std::filesystem::directory_iterator(samplesInputDirPath)){
    cv::Mat sample = cv::imread(entry.path());
    
    std::filesystem::path filename = entry.path().filename();
    Huenicorn::Logger::log(filename);
    Huenicorn::Logger::log("");

    for(const auto& [type, interpolationOutDir] : testOutputs){
      std::filesystem::path dir = samplesOutputDirPath / interpolationOutDir;
      std::filesystem::create_directories(dir);
      std::filesystem::path fullpath = dir / filename;

      cv::Mat sampleClone = sample.clone();
      Huenicorn::ImageProcessing::rescale(sampleClone, 43, type);

      Huenicorn::Logger::log(fullpath);

      cv::imwrite(fullpath, sampleClone);
    }
  }

  return 0;
}