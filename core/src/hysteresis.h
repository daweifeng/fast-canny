#pragma once

void HysteresisSlow(double *input, double *output, int width, int height,
                    double lowThreshold, double highThreshold);
void Hysteresis(double *input, double *output, int width, int height,
                double lowThreshold, double highThreshold);
void HysteresisIteration(double *input, double *output, int width, int height,
                         double lowThreshold, double highThreshold);

void HysteresisQueue(double *input, double *output, int width, int height,
                     double lowThreshold, double highThreshold);
