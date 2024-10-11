#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

#define MAX_FREQ 3.4
#define BASE_FREQ 2.4

static inline unsigned long long rdtsc(void) {
  unsigned hi, lo;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

int main(int argc, char *argv[]) {
  std::string imagePath = (std::string)argv[1];
  std::cout << "image path: " << imagePath << "\n";
  cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
  if (image.empty()) {
    std::cerr << "Error: Could not load image." << "\n";
    return -1;
  }

  unsigned long long st;
  unsigned long long et;
  unsigned long long sum = 0;

  st = rdtsc();

  cv::Mat blurredImage;
  cv::Mat edges;
  double lowerThreshold = 100;
  double upperThreahold = 200;
  cv::GaussianBlur(image, blurredImage, cv::Size(3, 3),
                   0.5); // Kernel size 3x3, sigma = 0.5
  cv::Canny(blurredImage, edges, lowerThreshold, upperThreahold);

  et = rdtsc();

  sum += (et - st);

  std::cout << "RDTSC Cycles Taken for Canny: " << sum * MAX_FREQ / BASE_FREQ
            << "\n";

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
