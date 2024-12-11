
#include "opencv2/opencv.hpp"

std::shared_ptr<cv::Mat> FastCanny(const cv::Mat &input, int lowerThreshold,
                                   int upperThreshold, int kernelSize,
                                   double sigma);
