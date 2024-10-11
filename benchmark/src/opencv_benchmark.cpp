#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

int main(int argc, char *argv[]) {
  std::string imagePath = (std::string)argv[1];
  std::cout << "image path: " << imagePath << "\n";
  cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
  if (image.empty()) {
    std::cerr << "Error: Could not load image." << "\n";
    return -1;
  }
  cv::Mat blurredImage;
  cv::GaussianBlur(image, blurredImage, cv::Size(3, 3),
                   0); // Kernel size 5x5, sigma = 1.4

  cv::Mat edges;
  double lowerThreshold = 100;
  double upperThreahold = 200;
  cv::Canny(blurredImage, edges, lowerThreshold, upperThreahold);

  std::string outputFilePath = "output_edges.jpg";
  bool success = cv::imwrite(outputFilePath, edges);

  if (success) {
    std::cout << "Edge-detected image saved successfully as " << outputFilePath
              << "\n";
  } else {
    std::cerr << "Error: Could not save the image.\n";
    return -1;
  }

  return 0;
}
