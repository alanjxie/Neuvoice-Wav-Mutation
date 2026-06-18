#define _USE_MATH_DEFINES
#include "kiss_fft.h"
#include <vector>
#include <cmath>
#include "helper.h"
std::vector<float> main() {
    int fft_size = 1024;
    int step_size = 256;

    // 1. Allocate the Forward FFT plan (inverse flag = 0)
    kiss_fft_cfg forward_cfg = kiss_fft_alloc(fft_size, 0, nullptr, nullptr);

    // 2. Allocate the Inverse FFT plan (inverse flag = 1)
    kiss_fft_cfg inverse_cfg = kiss_fft_alloc(fft_size, 1, nullptr, nullptr);

    //initializing the 2d arrays
    //they're 2d because of the nature of STFT...since it slides we have to take multiple ffts at once
    std::vector<std::vector<kiss_fft_cpx>> twod_fftmatrix_inputfile;
    std::vector<std::vector<kiss_fft_cpx>> twod_fftmatrix_mappingfile;
    std::vector<std::vector<kiss_fft_cpx>> mutated_spectrogram;

    std::vector<float> input_wav_file(44100 * 3, 0.5f);
    //Instantiates a helper, undefined currently
    HelperClass helper;

    helper.stft_template(twod_fftmatrix_inputfile, forward_cfg, input_wav_file);
    helper.stft_template(twod_fftmatrix_mappingfile, forward_cfg, input_wav_file);

    //just going to assume the components are the same length, ill gemini it later whatever
    for (int i = 0; i < twod_fftmatrix_inputfile.size(); ++i) {
        for (int j = 0; j < twod_fftmatrix_inputfile[i].size(); ++j) {
            double src_magnitude = std::sqrt(twod_fftmatrix_inputfile[i][j].r * twod_fftmatrix_inputfile[i][j].r + twod_fftmatrix_inputfile[i][j].i * twod_fftmatrix_inputfile[i][j].i);
            double mapped_phase = std::atan2(twod_fftmatrix_mappingfile[i][j].r, twod_fftmatrix_mappingfile[i][j].i);
            mutated_spectrogram[i][j].r = src_magnitude * cos(mapped_phase);
            mutated_spectrogram[i][j].i = src_magnitude * sin(mapped_phase);
        }
    }

    //final output array
    std::vector<float> output_audio(input_wav_file.size());
    //buffer
    std::vector<kiss_fft_cpx> audio_buffer(fft_size);

    //initializing process of taking inverse STFT
    for (int i = 0; i < mutated_spectrogram.size(); ++i) {
        kiss_fft(inverse_cfg, mutated_spectrogram[i].data(), audio_buffer.data());
        int index_offset = i * step_size;
        
        for (int j = 0; j < fft_size; ++j) {
            float new_audio = audio_buffer[j].r / static_cast<float>(fft_size);
            output_audio[j + index_offset] = new_audio;
        }
    };


    return output_audio;
}
