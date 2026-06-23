#ifndef HELPER_H
#define HELPER_H
#include <vector>
#include "kiss_fft.h"
struct HelperClass {
    void stft_template(std::vector<std::vector<kiss_fft_cpx>>&output_array, kiss_fft_cfg cfg, const std::vector<float> input_wav_file);
};

#endif