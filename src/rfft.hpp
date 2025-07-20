#pragma once
#include "AudioFFT/AudioFFT.h"
#include <vector>
#include <complex>
#include <span>

class RFFT {
public:
    void Init(int fft_size);
    void Transform(std::span<float> block);
    float CorrectFreqBin(int i) const;
    /*
     * @return [-0.5, 0.5] -0.5 is frame begin, 0.5 is frame end
     */
    float CoreectTime(int i) const;
    float CoreectGain(int i) const;
    int NumDataBins() const;
    int FFtSize() const { return fft_size_; }
private:
    int fft_size_;
    std::vector<float> window_;
    std::vector<float> sample_;
    std::vector<float> real_;
    std::vector<float> imag_;
    std::vector<std::complex<float>> complex_;
    std::vector<std::complex<float>> t_complex_;
    std::vector<std::complex<float>> f_complex_;
    audiofft::AudioFFT fft_;
};
