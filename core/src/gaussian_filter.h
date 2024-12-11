#pragma once

void GaussianFilter(const double *input, double *output, int kernalSize,
                    int width, int height, double sigma);

void GaussianFilterSlow(const double *input, double *output, int kernalSize,
                        int width, int height, double sigma);

void GenerateGaussianKernel(double *kernel, int width, int height,
                            double sigma);
