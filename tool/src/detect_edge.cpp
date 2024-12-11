

#include "fast_canny.h"
#include "opencv2/opencv.hpp"
#include <filesystem>
#include <iostream>

#define GAUSSIAN_KERNEL_SIZE 3
#define GAUSSIAN_KERNEL_SIGMA 0.5
#define CANNY_GRADIENT_LOWER_THRESHOLD 100
#define CANNY_GRADIENT_UPPER_THRESHOLD 200

int main(int argc, char *argv[]) {

  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " fast/opencv "
              << "<coco_image_path>\n";

    return -1;
  }

  std::string mode = argv[1];

  if (mode != "fast" && mode != "opencv") {
    std::cerr << "Invalid mode: " << mode << "\n";
    return -1;
  }

  std::filesystem::path cocoImagePath = (std::string)argv[2];

  std::cout << "Coco image path: " << cocoImagePath << "\n";

  cv::Mat image = cv::imread(cocoImagePath.string(), cv::IMREAD_GRAYSCALE);

  if (image.empty()) {
    std::cerr << "Error: Unable to read image\n";
    return -1;
  }

  cv::Mat imageDouble;
  image.convertTo(imageDouble, CV_64F);

  if (mode == "fast") {

    auto edges = FastCanny(imageDouble, CANNY_GRADIENT_LOWER_THRESHOLD,
                           CANNY_GRADIENT_UPPER_THRESHOLD, GAUSSIAN_KERNEL_SIZE,
                           GAUSSIAN_KERNEL_SIGMA);

    auto edgesData = edges.get();

    bool success = cv::imwrite("edges_" + mode + ".png", *edgesData);

    if (!success) {
      std::cerr << "Error: Unable to write image\n";
      return -1;
    }

  } else if (mode == "opencv") {

    cv::Mat blurredImage;
    cv::GaussianBlur(image, blurredImage,
                     cv::Size(GAUSSIAN_KERNEL_SIZE, GAUSSIAN_KERNEL_SIZE),
                     GAUSSIAN_KERNEL_SIGMA, cv::BORDER_CONSTANT, 0);

    cv::Mat edges;
    cv::Canny(blurredImage, edges, CANNY_GRADIENT_LOWER_THRESHOLD,
              CANNY_GRADIENT_UPPER_THRESHOLD, 3);

    bool success = cv::imwrite("edges_" + mode + ".png", edges);

    if (!success) {
      std::cerr << "Error: Unable to write image\n";
      return -1;
    }

  } else {
    std::cerr << "Invalid mode: " << mode << "\n";
    return -1;
  }

  return 0;
}
