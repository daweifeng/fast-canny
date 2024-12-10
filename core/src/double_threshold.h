#ifndef DOUBLE_THRESHOLD_H
#define DOUBLE_THRESHOLD_H

void double_threshold_slow(double *input, double *output, int width, int height,
                           double low_thres = 50, double high_thres = 100);
void double_threshold(double *input, double *output, int width, int height,
                      double low_thres = 50, double high_thres = 100);

#endif // DOUBLE_THRESHOLD_H