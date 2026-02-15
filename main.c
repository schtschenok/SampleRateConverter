// LLM-generated slop

#include "src.h"
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- HARDCODED SETTINGS --- */
#define INPUT_FILE_PATH "input.wav"
// Interpolation factor (Upsampling)
#define NUM_TAPS_PER_PHASE 64

int main(void) {
    // ============================================================
    // 1. SETUP & FILENAME GENERATION
    // ============================================================
    const char* in_filename = INPUT_FILE_PATH;
    char out_filename[256];
    const char* ext = strrchr(in_filename, '.');
    size_t base_len = ext ? (size_t)(ext - in_filename) : strlen(in_filename);
    strncpy(out_filename, in_filename, base_len);
    out_filename[base_len] = '\0';
    strcat(out_filename, "_resampled.wav");

    printf("Processing: %s -> %s\n", in_filename, out_filename);
    printf("Rate Change: %d", 4);

    // ============================================================
    // 2. PHASE I: FULL FILE LOADING
    // ============================================================
    SF_INFO sf_in_info = { 0 };
    SNDFILE* infile = sf_open(in_filename, SFM_READ, &sf_in_info);

    if (!infile) {
        printf("Error: Could not open input file '%s'\n", in_filename);
        puts(sf_strerror(NULL));
        return 1;
    }

    if (sf_in_info.channels != 1) {
        printf("Error: Input must be Mono (1 channel). Found %d.\n", sf_in_info.channels);
        sf_close(infile);
        return 1;
    }

    // Calculate total size and allocate full input buffer
    sf_count_t input_frame_count = sf_in_info.frames;
    float* full_input_buffer = (float*)malloc(input_frame_count * sizeof(float));
    
    if (!full_input_buffer) {
        printf("Error: Memory allocation failed for input buffer.\n");
        sf_close(infile);
        return 1;
    }

    // Read the entire file into memory
    sf_count_t read_count = sf_read_float(infile, full_input_buffer, input_frame_count);
    
    // Close input file immediately (Separation of concerns)
    sf_close(infile); 

    if (read_count != input_frame_count) {
        printf("Warning: Expected %ld frames, read %ld.\n", (long)input_frame_count, (long)read_count);
    }

    // ============================================================
    // 3. PHASE II: RESAMPLING (IN MEMORY)
    // ============================================================
    
    // A. Init Library
    int num_taps = NUM_TAPS_PER_PHASE * 4;

    float* coefficients = src_generate_fir_coeffs(num_taps);
    if (!coefficients) {
        printf("Error: Failed to generate FIR coefficients.\n");
        free(full_input_buffer);
        return 1;
    }

    float* pfb = src_generate_fir_filter(coefficients, num_taps);
    free(coefficients); // No longer needed

    if (!pfb) {
        printf("Error: Failed to generate FIR filter.\n");
        free(full_input_buffer);
        return 1;
    }

    // B. Allocate Full Output Buffer
    // Calculate theoretical output size
    sf_count_t max_output_frames = read_count * 4; // +16 for safety padding
    float* full_output_buffer = (float*)malloc(max_output_frames * sizeof(float));

    if (!full_output_buffer) {
        printf("Error: Memory allocation failed for output buffer.\n");
        free(full_input_buffer);
        free(pfb);
        return 1;
    }

    // C. Process Single Block
    // We pass the entire input buffer count cast to int (assuming file fits in int range as per src_filt signature)
    src_filt(num_taps, pfb, full_input_buffer, (int)read_count, full_output_buffer);

    // ============================================================
    // 4. PHASE III: FILE SAVING
    // ============================================================
    SF_INFO sf_out_info = sf_in_info;
    sf_out_info.samplerate = sf_in_info.samplerate * 4;
    sf_out_info.frames = max_output_frames; 
    
    SNDFILE* outfile = sf_open(out_filename, SFM_WRITE, &sf_out_info);
    if (!outfile) {
        printf("Error: Could not create output file '%s'\n", out_filename);
        puts(sf_strerror(NULL));
        // Cleanup happens below
    } else {
        sf_write_float(outfile, full_output_buffer, max_output_frames);
        sf_close(outfile);
        printf("Saved: %s\n", out_filename);
    }

    // ============================================================
    // 5. CLEANUP
    // ============================================================
    free(full_input_buffer);
    free(full_output_buffer);

    if (pfb) free(pfb);

    return 0;
}