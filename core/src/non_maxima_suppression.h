#ifndef NON_MAX_SUPPRESSION_H
#define NON_MAX_SUPPRESSION_H

void NonMaxSuppressionSlow(double *input, double *output, double *theta,
                           int kernalSize, int width, int height);

void NonMaxSuppression(double *input, double *output, double *theta,
                       int kernalSize, int width, int height);
#endif // NON_MAX_SUPPRESSION_H
