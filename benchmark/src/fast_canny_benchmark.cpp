#include "fast_canny.h"
#include <filesystem>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#define MAX_FREQ 3.4
#define BASE_FREQ 2.4
#define NUM_IMAGES_PER_SIZE 10
#define GAUSSIAN_KERNEL_SIZE 3
#define GAUSSIAN_KERNEL_SIGMA 0.5
#define CANNY_GRADIENT_LOWER_THRESHOLD 100
#define CANNY_GRADIENT_UPPER_THRESHOLD 200

static inline unsigned long long rdtsc(void) {
  unsigned hi, lo;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

struct CocoImageMeta {
  std::filesystem::path path;
  int width;
  int height;
};

void TestImages(const CocoImageMeta &imageMeta) {
  unsigned long long st;
  unsigned long long et;
  unsigned long long cvCannyTime = 0;
  unsigned long long fastCannyTime = 0;
  unsigned long long runs = 0;
  int repeat = 1000;
  std::vector<cv::Mat> images;
  images.reserve(NUM_IMAGES_PER_SIZE);

  for (const auto &p : std::filesystem::directory_iterator(imageMeta.path)) {
    cv::Mat image = cv::imread(p.path(), cv::IMREAD_GRAYSCALE);

    if (image.empty()) {
      throw std::runtime_error("Could not load image: " + p.path().string());
    }

    images.push_back(image);
  }

  cv::Mat image;
  cv::Mat blurredImage;
  cv::Mat referenceEdges;
  cv::Mat imageDouble;
  image = images[0];
  image.convertTo(imageDouble, CV_64F);

  st = rdtsc();
  cv::GaussianBlur(image, blurredImage,
                   cv::Size(GAUSSIAN_KERNEL_SIZE, GAUSSIAN_KERNEL_SIZE),
                   GAUSSIAN_KERNEL_SIGMA, cv::BORDER_CONSTANT, 0);
  cv::Canny(blurredImage, referenceEdges, CANNY_GRADIENT_LOWER_THRESHOLD,
            CANNY_GRADIENT_UPPER_THRESHOLD);

  et = rdtsc();
  cvCannyTime += (et - st);

  st = rdtsc();
  auto edges = FastCanny(imageDouble, CANNY_GRADIENT_LOWER_THRESHOLD,
                         CANNY_GRADIENT_UPPER_THRESHOLD, GAUSSIAN_KERNEL_SIZE,
                         GAUSSIAN_KERNEL_SIGMA);
  et = rdtsc();
  fastCannyTime += (et - st);

  std::cout << "Canny time for OpenCV: " << cvCannyTime << "\n";
  std::cout << "Canny time for FastCanny: " << fastCannyTime << "\n";
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <coco_image_path>\n";

    return -1;
  }
  std::filesystem::path cocoImagePath = (std::string)argv[1];
  std::cout << "Coco image path: " << cocoImagePath << "\n";

  CocoImageMeta image32 = {cocoImagePath / "32x32", 32, 32};
  CocoImageMeta image64 = {cocoImagePath / "64x64", 64, 64};
  CocoImageMeta image128 = {cocoImagePath / "128x128", 128, 128};
  CocoImageMeta image256 = {cocoImagePath / "256x256", 256, 256};
  CocoImageMeta image512 = {cocoImagePath / "512x512", 512, 512};
  CocoImageMeta image1024 = {cocoImagePath / "1024x1024", 1024, 1024};

  try {
    cv::setNumThreads(0);

    std::cout << "================================================" << "\n";

    std::cout << "Testing images in " << image32.path << "\n";
    TestImages(image32);
    std::cout << "================================================" << "\n";

    std::cout << "Testing images in " << image64.path << "\n";
    TestImages(image64);
    std::cout << "================================================" << "\n";

    std::cout << "Testing images in " << image128.path << "\n";
    TestImages(image128);
    std::cout << "================================================" << "\n";

    std::cout << "Testing images in " << image256.path << "\n";
    TestImages(image256);
    std::cout << "================================================" << "\n";

    std::cout << "Testing images in " << image512.path << "\n";
    TestImages(image512);
    std::cout << "================================================" << "\n";

    std::cout << "Testing images in " << image1024.path << "\n";
    TestImages(image1024);
    std::cout << "================================================" << "\n";
  } catch (const std::runtime_error &err) {
    std::cerr << "[ERROR] " << err.what() << "\n";

    return -1;
  }

  return 0;
}