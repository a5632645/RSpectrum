#pragma once
#include "AudioFFT/AudioFFT.h"
#include <cstddef>
#include <numeric>
#include <span>
#include <vector>
#include <complex>
#include <cmath>
#include <numbers>

class RFFT2 {
public:
    void Init(int fft_size) {
        fft_size_ = fft_size;
        fft_.init(fft_size);
        window_.resize(fft_size);
        dwindow_.resize(fft_size);
        twindow_.resize(fft_size);
        sample_.resize(fft_size);
        real_.resize(NumDataBins());
        imag_.resize(NumDataBins());
        xh_data_.resize(NumDataBins());
        xdh_data_.resize(NumDataBins());
        xth_data_.resize(NumDataBins());
        temp_.resize(NumDataBins());

        for (int i = 0; i < fft_size; ++i) {
            auto t = static_cast<float>(i) / (fft_size - 1.0f);
            window_[i] = 0.5f * (1.0f - std::cos(std::numbers::pi_v<float> * 2.0f * t));
            dwindow_[i] = 0.5f * std::numbers::pi_v<float> * 2.0f * std::sin(std::numbers::pi_v<float> * 2.0f * t);
        }

        std::ranges::copy(window_, twindow_.begin());
        auto offset = 0.5f * (window_.size() - 1);
        for (size_t k = 0; k < window_.size(); ++k)
        {
            twindow_[k] *= (k - offset);
        }

        auto window_sum = std::accumulate(window_.cbegin(), window_.cend(), 0.0f);
        window_scale_ = 2.0f / window_sum;
        dwindow_scale_ = window_scale_ / (2 * std::numbers::pi_v<float>);
        float energy = 0.0f;
        for (auto s : window_) {
            energy += s * s;
        }
        float gain = 1.0f / std::sqrt(energy);
        gain_scale_ = gain / window_scale_;
    }

    void Transform(std::span<float> buffer) {
        for (int i = 0; i < fft_size_; ++i) {
            sample_[i] = buffer[i] * window_[i];
        }
        fft_.fft(sample_.data(), real_.data(), imag_.data());
        for (int i = 0; auto & cpx : xh_data_) {
            cpx = std::complex{ real_[i] ,imag_[i] } * window_scale_;
            ++i;
        }

        for (int i = 0; i < fft_size_; ++i) {
            sample_[i] = buffer[i] * dwindow_[i];
        }
        fft_.fft(sample_.data(), real_.data(), imag_.data());
        for (int i = 0; auto & cpx : xdh_data_) {
            cpx = std::complex{ real_[i] ,imag_[i] } * dwindow_scale_;
            ++i;
        }

        for (int i = 0; i < fft_size_; ++i) {
            sample_[i] = buffer[i] * twindow_[i];
        }
        fft_.fft(sample_.data(), real_.data(), imag_.data());
        for (int i = 0; auto & cpx : xth_data_) {
            cpx = std::complex{ real_[i] ,imag_[i] } * dwindow_scale_;
            ++i;
        }
    }

    float CorrectFreqBin(int i) const {
        auto up = xdh_data_[i].imag() * xh_data_[i].real() - xdh_data_[i].real() * xh_data_[i].imag();
        auto down = std::norm(xh_data_[i]) + 1e-18f;
        auto freq_c = -up / down;
        return i + freq_c;
    }

    float CoreectTime(int idx) const {
        auto X_h = xh_data_[idx];
        auto X_Th = xth_data_[idx];

        auto num = X_h.real() * X_Th.real() + X_h.imag() * X_Th.imag();
        auto magSquared = norm(X_h) + 1e-18f;

        return num * std::numbers::pi_v<float> * 2.0f / magSquared / fft_size_;
    }

    float CoreectGain(int i) const {
        return std::abs(xh_data_[i]) * gain_scale_;
    }

    int NumDataBins() const { return fft_size_ / 2 + 1; }
    int FFtSize() const { return fft_size_; }

private:
    float window_scale_{};
    float dwindow_scale_{};
    float gain_scale_{};
    int fft_size_;
    std::vector<float> window_;
    std::vector<float> dwindow_;
    std::vector<float> twindow_;
    std::vector<float> sample_;
    std::vector<float> real_;
    std::vector<float> imag_;
    std::vector<float> temp_;
    std::vector<std::complex<float>> xh_data_;
    std::vector<std::complex<float>> xdh_data_;
    std::vector<std::complex<float>> xth_data_;
    audiofft::AudioFFT fft_;
};
