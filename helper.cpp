#define _USE_MATH_DEFINES
#include "kiss_fft.h"
#include <vector>
#include <cmath>
#include "helper.h"

int fft_size = 1024;
int step_size = 256;

class HelperClass {
    void stft_template (std::vector<std::vector<kiss_fft_cpx>>&output_array, kiss_fft_cfg cfg, std::vector<float> input_wav_file) {
        //Defining input and output buffers
        std::vector<kiss_fft_cpx> time_input(1024);
        std::vector<kiss_fft_cpx> freq_output(1024);

        //master loop, this constructs the stft manually
        for (int step = 0; step + fft_size <= input_wav_file.size(); step += step_size) {

            //populate the input array
            for (int j = 0; j < 1024 + step_size; ++j) {
                double raw_sample = input_wav_file[j];
                time_input[j].r = raw_sample;
                time_input[j].i = 0.0;
            }

            //window the signal to smooth it manually
            for (int n = 0; n < 1024; ++n) {
                double hann = 0.5 * (1.0 - std::cos((2.0 * M_PI * n) / (1024 - 1)));
                time_input[n].r = time_input[n].r * hann;
            }   
            kiss_fft(cfg, time_input.data(), freq_output.data());
            output_array.push_back(freq_output);

        }   
    }
};
