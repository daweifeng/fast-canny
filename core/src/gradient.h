#pragma once
#include <immintrin.h>

void Gradient(double *input, double *output, double *theta, int width, int height);

void GradientSlow(double *input, double *output, double *theta, int width, int height);