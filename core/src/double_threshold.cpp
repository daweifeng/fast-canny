#include "double_threshold.h"
#include <immintrin.h>
#include <iostream>


void DoubleThreshold(double *input, double *output, int width, int height,
                     double lowThreshold, double highThreshold) {

    __m256d lowTh, highTh, pixelValue, strongEdge, weakEdge, zero, highVal, lowVal;

    lowTh = _mm256_set1_pd(lowThreshold);
    highTh = _mm256_set1_pd(highThreshold);
    zero = _mm256_setzero_pd();
    highVal = _mm256_set1_pd(highThreshold);
    lowVal = _mm256_set1_pd(lowThreshold);

    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j += 4) {
            // Load four double-precision values
            pixelValue = _mm256_loadu_pd(&input[i * width + j]);

            // Condition for strong edges (>= highThreshold)
            strongEdge = _mm256_cmp_pd(pixelValue, highTh, _CMP_GE_OQ);

            // Condition for weak edges (lowThreshold <= value <= highThreshold)
            weakEdge = _mm256_and_pd(
                _mm256_cmp_pd(pixelValue, lowTh, _CMP_GE_OQ),
                _mm256_cmp_pd(pixelValue, highTh, _CMP_LE_OQ)
            );

            // Assign highThreshold to strong edges
            __m256d strongResult = _mm256_blendv_pd(zero, highVal, strongEdge);

            // Assign lowThreshold to weak edges
            __m256d weakResult = _mm256_blendv_pd(zero, lowVal, weakEdge);

            // Combine results (priority to strong edges)
            pixelValue = _mm256_add_pd(strongResult, weakResult);

            // Store the result
            _mm256_storeu_pd(&output[i * width + j], pixelValue);
        }
    }
}

// void DoubleThresholdSlow(double *input, double *output, int height, int width, double lowThreshold, double highThreshold){
//     for(int i=1; i<height-1; i++){
//         for(int j=1; j<width-1; j++){
//             if(input[i][j] >= highThreshold){
//                 output[i][j] = highThreshold;
//             }
//             else if(lowThreshold <= input[i][j] && input[i][j] <= highThreshold){
//                 output[i][j] = lowThreshold;
//             }
//             else{
//                 output[i][j] = 0;
//             }
//         }
//     }
//     return;
// }