#pragma once

void Gradient(const double *input, double *output, double *theta, int width,
              int height);

void GradientSlow(const double *input, double *output, double *theta, int width,
                  int height);
