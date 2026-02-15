
#include "src.h"
#include <math.h>
#include <stdlib.h>

// PREP

float* src_blackman(int num_taps) {
    float* window = malloc(num_taps * sizeof(float));
    for (int i = 0; i < num_taps; i++) {
        float alpha = 0.42;
        float beta = 0.5;
        window[i] = alpha - beta * cos(2.0f * M_PI * (float)i / (float)(num_taps - 1)) + (beta - alpha) * cos(4.0f * M_PI * (float)i / (float)(num_taps - 1));
    }
    return window;
}

float* src_fir_prototype(int num_taps) {
    float* proto = malloc(num_taps * sizeof(float));
    int m = num_taps - 1;
    for (int i = 0; i < num_taps; i++) {
        float x = 2.f * 0.125 * (i - m / 2.f);
        proto[i] = (x == 0.0f) ? 1.0f : sinf(M_PI * x) / (M_PI * x);
    }
    return proto;
}

float* src_generate_fir_coeffs(int num_taps) {
    float* proto = src_fir_prototype(num_taps);
    float* window = src_blackman(num_taps);
    for (int i = 0; i < num_taps; i++) {
        proto[i] *= window[i];
    }
    free(window);

    return proto;
}

void src_taps_to_pfb(float* coefficients, int num_taps, float** p_pfb) {
    int taps_per_phase = (num_taps + 3) / 4;
    int pfb_size = taps_per_phase * 4;
    float* pfb = calloc(pfb_size, sizeof(float));

    for (int phase = 0; phase < 4; phase++) {
        for (int tap = 0; tap < taps_per_phase; tap++) {
            int coeff_idx = tap * 4 + phase;
            if (coeff_idx < num_taps) {
                pfb[phase * taps_per_phase + taps_per_phase - 1 - tap] = coefficients[coeff_idx];
            }
        }
    }

    *p_pfb = pfb;
}

float* src_generate_fir_filter(float* coefficients, int num_taps) {
    float* pfb;

    src_taps_to_pfb(coefficients, num_taps, &pfb);

    return pfb;
}

// RUNTIME

float dot(float* coeffs, int len, float* samples, int end_index) {
    float dotprod = 0.0f;

    int start_i = 0;
    if (end_index < len - 1) {
        start_i = len - 1 - end_index;
    }

    for (int i = start_i; i < len; i++) {
        int sample_idx = end_index - len + 1 + i;
        dotprod += coeffs[i] * samples[sample_idx];
    }
    return dotprod;
}

void src_filt(int num_taps, float* pfb, float* input, int count, float* output) {
    const int taps_per_phase = (num_taps + 3) / 4;
    int out_idx = 0;

    for (int i = 0; i < count; i++) {
        for (int phase = 0; phase < 4; phase++) {
            float* current_coeffs = &pfb[phase * taps_per_phase];
            output[out_idx++] = dot(current_coeffs, taps_per_phase, input, i);
        }
    }
}