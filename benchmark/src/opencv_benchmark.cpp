#include <filesystem>
#include <iostream>
#include <list>
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include <string>

#define MAX_FREQ 3.4
#define BASE_FREQ 2.4
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
  unsigned long long sum = 0;
  unsigned long long runs = 0;
  int repeat = 100;
  std::list<cv::Mat> imageList;

  for (const auto &p : std::filesystem::directory_iterator(imageMeta.path)) {
    cv::Mat image = cv::imread(p.path(), cv::IMREAD_GRAYSCALE);

    if (image.empty()) {
      throw std::runtime_error("Could not load image: " + p.path().string());
    }

    imageList.push_back(image);
  }

  for (int i = 0; i != repeat; ++i) {
    for (const auto &image : imageList) {
      st = rdtsc();

      cv::Mat blurredImage;
      cv::Mat edges;
      cv::GaussianBlur(image, blurredImage,
                       cv::Size(GAUSSIAN_KERNEL_SIZE, GAUSSIAN_KERNEL_SIZE),
                       GAUSSIAN_KERNEL_SIGMA);
      cv::Canny(blurredImage, edges, CANNY_GRADIENT_LOWER_THRESHOLD,
                CANNY_GRADIENT_UPPER_THRESHOLD);

      et = rdtsc();
      sum += (et - st);
      runs++;
    }
  }

  unsigned long long gaussianFilterKernelFLOPS = 9 + 8; // per pixel
  unsigned long long intensityGradientsKernelFLOPS =
      (9 + 8) * 2 + 4 + 3;                                   // per pixel
  unsigned long long gradientMagnitudeThresholdingFLOPS = 2; // per pixel
  unsigned long long doubleThresholdFLOPS = 2;               // per pixel
  unsigned long long trackEdgeFLOPS = 9;

  unsigned long long totalFLOPS =
      runs * imageMeta.width * imageMeta.height *
      (gaussianFilterKernelFLOPS + intensityGradientsKernelFLOPS +
       gradientMagnitudeThresholdingFLOPS + doubleThresholdFLOPS +
       trackEdgeFLOPS);

  std::cout << "Total runs: " << runs << "\n";
  std::cout << "RDTSC Cycles Taken for Canny: " << sum << "\n";
  std::cout << "Total FLOPS: " << totalFLOPS << "\n";
  std::cout << "FLOPS Per Cycle: " << totalFLOPS / (sum * MAX_FREQ / BASE_FREQ)
            << "\n";
}

int main(int argc, char *argv[]) {
  std::filesystem::path cocoImagePath = (std::string)argv[1];
  std::cout << "Coco image path: " << cocoImagePath << "\n";

  CocoImageMeta image32 = {cocoImagePath / "32x32", 32, 32};
  CocoImageMeta image64 = {cocoImagePath / "64x64", 64, 64};
  CocoImageMeta image128 = {cocoImagePath / "128x128", 128, 128};
  CocoImageMeta image256 = {cocoImagePath / "256x256", 256, 256};
  CocoImageMeta image512 = {cocoImagePath / "512x512", 512, 512};
  CocoImageMeta image1024 = {cocoImagePath / "1024x1024", 1024, 1024};
  CocoImageMeta image2048 = {cocoImagePath / "2048x2048", 2048, 2048};

  try {

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

    std::cout << "Testing images in " << image2048.path << "\n";
    TestImages(image2048);
    std::cout << "================================================" << "\n";
  } catch (const std::runtime_error &err) {
    std::cerr << "[ERROR] " << err.what() << "\n";

    return -1;
  }

  return 0;
}
