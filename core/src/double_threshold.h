#ifndef DOUBLE_THRESHOLD_H
#define DOUBLE_THRESHOLD_H

void DoubleThresholdSlow(double *input, double *output, int width, int height,
                         double low_thres = 50, double high_thres = 100);
void DoubleThreshold(double *input, double *output, int width, int height,
                     double low_thres = 50, double high_thres = 100);

#endif // DOUBLE_THRESHOLD_H
