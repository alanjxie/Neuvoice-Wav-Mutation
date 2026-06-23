#define _USE_MATH_DEFINES
#include "kiss_fft.h"
#include <vector>
#include <cmath>
#include "helper.h"
#include "AudioFile.h"
#include <iostream>
int main() {

    //Load input file
    AudioFile<float> audio_file1;
    audio_file1.load("/Users/alanxie/projects/am_zephyr_sample_v1.wav");
    int inputFileSize = audio_file1.samples[0].size();
    audio_file1.printSummary();

    std::vector<float> input_wav_file = audio_file1.samples[0];

    //Load mapping file and normalize size to input file
    AudioFile<float> audio_file2;
    audio_file2.load("/Users/alanxie/projects/bf_alice_sample_v1.wav");
    int mappingFileSize = audio_file2.samples[0].size();
    audio_file2.printSummary();

    std::vector<float> mapping_wav_file;
    for (int i = 0; i < inputFileSize; ++i) {
        if (i < mappingFileSize) {
        mapping_wav_file.push_back(audio_file2.samples[0][i]);
        } else {
        mapping_wav_file.push_back(0.0f); 
    }
    }

    int fft_size = 1024;
    int step_size = 256;

    // 1. Allocate the Forward FFT plan (inverse flag = 0)
    kiss_fft_cfg forward_cfg = kiss_fft_alloc(fft_size, 0, nullptr, nullptr);

    // 2. Allocate the Inverse FFT plan (inverse flag = 1)
    kiss_fft_cfg inverse_cfg = kiss_fft_alloc(fft_size, 1, nullptr, nullptr);

    //initializing the 2d arrays
    std::vector<std::vector<kiss_fft_cpx>> twod_fftmatrix_inputfile;
    std::vector<std::vector<kiss_fft_cpx>> twod_fftmatrix_mappingfile;
    std::vector<std::vector<kiss_fft_cpx>> mutated_spectrogram;
    HelperClass helper;

    helper.stft_template(twod_fftmatrix_inputfile, forward_cfg, input_wav_file);
    helper.stft_template(twod_fftmatrix_mappingfile, forward_cfg, mapping_wav_file);

    //debug statements
    std::cout << "Input file vector size: " << input_wav_file.size() << std::endl;
    std::cout << "Input matrix rows: " << twod_fftmatrix_inputfile.size() << std::endl;
    if (!twod_fftmatrix_inputfile.empty()) {
        std::cout << "Input matrix cols: " << twod_fftmatrix_inputfile[0].size() << std::endl;
    }
    std::cout << "Total STFT frames analyzed: " << twod_fftmatrix_inputfile.size() << std::endl;

    mutated_spectrogram.resize(twod_fftmatrix_inputfile.size());


    for (int i = 0; i < twod_fftmatrix_inputfile.size(); ++i) {
        mutated_spectrogram[i].resize(twod_fftmatrix_inputfile[i].size());
    }
    std::vector<double> prev_input_phase(fft_size, 0.0);
    std::vector<double> prev_output_phase(fft_size, 0.0);

    //Math for mapping and mutating
    for (int i = 0; i < twod_fftmatrix_inputfile.size(); ++i) {
        for (int j = 0; j < twod_fftmatrix_inputfile[i].size(); ++j) {
            double src_magnitude = std::sqrt(twod_fftmatrix_inputfile[i][j].r * twod_fftmatrix_inputfile[i][j].r + twod_fftmatrix_inputfile[i][j].i * twod_fftmatrix_inputfile[i][j].i);
            double mapped_phase = std::atan2(twod_fftmatrix_mappingfile[i][j].i, twod_fftmatrix_mappingfile[i][j].r);

            double input_phase = std::atan2(
            twod_fftmatrix_inputfile[i][j].i,
            twod_fftmatrix_inputfile[i][j].r
        );

        // Expected phase advance for bin j over one hop
        double expected_advance = (2.0 * M_PI * j * step_size) / fft_size;

        // True deviation from expected
        double delta = (input_phase - prev_input_phase[j]) - expected_advance;

        // Wrap delta to [-pi, pi]
        delta = delta - 2.0 * M_PI * std::round(delta / (2.0 * M_PI));

        // Accumulate output phase coherently
        prev_output_phase[j] += expected_advance + delta;
        prev_input_phase[j]   = input_phase;

        mutated_spectrogram[i][j].r = src_magnitude * std::cos(prev_output_phase[j]);
        mutated_spectrogram[i][j].i = src_magnitude * std::sin(prev_output_phase[j]);
        }
    }


    //final output array
    std::vector<float> output_audio(inputFileSize);

    //buffer
    std::vector<kiss_fft_cpx> audio_buffer(fft_size);

    //initializing process of taking inverse STFT
    for (int i = 0; i < mutated_spectrogram.size(); ++i) {
        kiss_fft(inverse_cfg, mutated_spectrogram[i].data(), audio_buffer.data());
        int index_offset = i * step_size;
        
        for (int j = 0; j < fft_size; ++j) {
            float new_audio = audio_buffer[j].r / static_cast<float>(fft_size);
            // 3. Accumulate safely
            if ((j + index_offset) < output_audio.size()) {
                output_audio[j + index_offset] += new_audio;
                }
        }
    };
    std::cout << "--- INVERSE STFT DIAGNOSTICS ---" << std::endl;
    std::cout << "Mutated spectrogram frames: " << mutated_spectrogram.size() << std::endl;
    std::cout << "Output audio size: " << output_audio.size() << std::endl;
    if (output_audio.size() > 1000) {
    std::cout << "Sample 1000 value before scaling: " << output_audio[1000] << std::endl;
    }


    AudioFile<float> output_file;
    AudioFile<float>::AudioBuffer buffer;
    buffer.resize(1);
    buffer[0].resize(output_audio.size());

    for (int j = 0; j < buffer[0].size(); ++j) {
        float scaled_sample = output_audio[j] * fft_size * 0.375f;
        if (scaled_sample > 1.0f)  scaled_sample = 1.0f;
        if (scaled_sample < -1.0f) scaled_sample = -1.0f;

        buffer[0][j] = scaled_sample;
    }


    output_file.setAudioBuffer(buffer);
    output_file.save ("/Users/alanxie/projects/newFile.wav");
    return 0;
}
