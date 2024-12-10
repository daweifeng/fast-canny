#ifndef NON_MAX_SUPPRESSION_H
#define NON_MAX_SUPPRESSION_H

void non_max_suppression_slow(double *input, double *output, double *theta,
                              int kernalSize, int width, int height,
                              double sigma);

void non_max_suppression(double *input, double *output, double *theta,
                         int kernalSize, int width, int height, double sigma);
#endif // NON_MAX_SUPPRESSION_H