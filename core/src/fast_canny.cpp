#include "fast_canny.h"
#include "double_threshold.h"
#include "gaussian_filter.h"
#include "gradient.h"
#include "hysteresis.h"
#include "non_maxima_suppression.h"
#include "opencv2/core/mat.hpp"
#include <iostream>
#include <memory>

std::shared_ptr<cv::Mat> FastCanny(const cv::Mat &input, int lowerThreshold,
                                   int upperThreshold, int kernelSize,
                                   double sigma) {

  for (int i = 0; i < input.rows * input.cols; i++) {
    if (input.at<double>(i) < 0 || input.at<double>(i) > 255) {
      throw std::runtime_error("FastCanny failed: input image must have pixel "
                               "values in the range [0, 255]");
    }
  }

  double *blurredImage = new double[input.rows * input.cols];
  // GaussianFilter(input.ptr<double>(), blurredImage, kernelSize, input.cols,
  //                input.rows, sigma);

  // Apply Sobel filter

  double *gradientOutput = new double[input.rows * input.cols];
  double *thetaOutput = new double[input.rows * input.cols];


  // GradientSlow(blurredImage, gradientOutput, thetaOutput, input.cols,
  //              input.rows);

  // Apply non-maximum suppression

  double *nonMaxSuppressionOutput = new double[input.rows * input.cols];
  NonMaxSuppression(gradientOutput, nonMaxSuppressionOutput, thetaOutput, 3,
                    input.cols, input.rows);

  // Apply hysteresis thresholding

  double *doubleThresholdOutput = new double[input.rows * input.cols];
  // DoubleThreshold(nonMaxSuppressionOutput, doubleThresholdOutput, input.cols,
  //                 input.rows, lowerThreshold, upperThreshold);

  double *hysteresisOutput = new double[input.rows * input.cols];
  // Hysteresis(doubleThresholdOutput, hysteresisOutput, input.cols, input.rows,
  //            lowerThreshold, upperThreshold);

  // Copy the output to the output image

  std::shared_ptr<cv::Mat> output = std::make_shared<cv::Mat>(
      input.rows, input.cols, CV_64F, nonMaxSuppressionOutput);

  delete[] blurredImage;
  delete[] gradientOutput;
  delete[] thetaOutput;
  delete[] nonMaxSuppressionOutput;
  delete[] doubleThresholdOutput;

  return output;
};
