#include <filesystem>
#include <iostream>
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

  for (const auto &p : std::filesystem::directory_iterator(imageMeta.path)) {

    cv::Mat image = cv::imread(p.path(), cv::IMREAD_GRAYSCALE);

    if (image.empty()) {
      throw std::runtime_error("Could not load image: " + p.path().string());
    }

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
  }

  // TODO: Find out the correct number of instructions
  double totalInstructions = imageMeta.width * imageMeta.height;
  double cycles = sum * MAX_FREQ / BASE_FREQ;

  std::cout << "RDTSC Cycles Taken for Canny: " << cycles << "\n";
  std::cout << "Instructions Per Cycle" << totalInstructions / cycles << "\n";
}

int main(int argc, char *argv[]) {
  std::filesystem::path cocoImagePath = (std::string)argv[1];
  std::cout << "Coco image path: " << cocoImagePath << "\n";

  CocoImageMeta smallImage = {cocoImagePath / "500x500", 500, 500};
  CocoImageMeta mediumImage = {cocoImagePath / "800x800", 800, 800};
  CocoImageMeta largeImage = {cocoImagePath / "1024x1024", 1024, 1024};

  try {
    std::cout << "================================================" << "\n";
    std::cout << "Testing images in " << smallImage.path << "\n";
    TestImages(smallImage);
    std::cout << "================================================" << "\n";

    std::cout << "================================================" << "\n";
    std::cout << "Testing images in " << mediumImage.path << "\n";
    TestImages(mediumImage);
    std::cout << "================================================" << "\n";

    std::cout << "================================================" << "\n";
    std::cout << "Testing images in " << largeImage.path << "\n";
    TestImages(largeImage);
    std::cout << "================================================" << "\n";
  } catch (const std::runtime_error &err) {
    std::cerr << "[ERROR] " << err.what() << "\n";

    return -1;
  }

  return 0;
}
