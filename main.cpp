#define _USE_MATH_DEFINES
#include "kiss_fft.h"
#include <vector>
#include <cmath>
#include "helper.h"
#include "AudioFile.h"
std::vector<float> main() {

    //Load input file
    AudioFile<float> audio_file;
    audio_file.load("/Users/alanxie/projects/am_zephyr_sample_v1");
    int inputFileSize = audio_file.samples[0].size();
    audio_file.printSummary();

    std::vector<float> input_wav_file = audio_file.samples[0];

    //Load mapping file and normalize size to input file
    audio_file.load("/Users/alanxie/projects/bf_alice_sample_v1");
    std::vector<float> mapping_wav_file;
    for (int i = 0; i < inputFileSize; ++i) {
        mapping_wav_file.push_back(audio_file.samples[0][i]);
    }

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
    //Instantiates a helper, undefined currently
    HelperClass helper;

    helper.stft_template(twod_fftmatrix_inputfile, forward_cfg, input_wav_file);
    helper.stft_template(twod_fftmatrix_mappingfile, forward_cfg, mapping_wav_file);

    //Math for mapping and mutating
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
    AudioFile<float>::AudioBuffer buffer;
    buffer.resize(1);
    buffer[0].resize(output_audio.size());

    for (int j = 0; j < buffer[0].size(); ++j) {
        float scaled_sample = output_audio[j] / 1.5f;
        if (scaled_sample > 1.0f)  scaled_sample = 1.0f;
        if (scaled_sample < -1.0f) scaled_sample = -1.0f;

        buffer[0][j] = scaled_sample;
    }
    audio_file.setAudioBuffer(buffer);
    audio_file.save ("/Users/alanxie/projects/newFile.wav");
    return output_audio;
}
