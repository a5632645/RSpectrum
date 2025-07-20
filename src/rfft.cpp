#include "rfft.hpp"
#include <numbers>

static float Arg(std::complex<float> cpx) {
    constexpr float pi = std::numbers::pi_v<float>;
    float e = std::arg(cpx);
    e /= (2.0f * pi);
    if (e < 0.0f) e += 1.0f;
    return e;
}

void RFFT::Init(int fft_size) {
    fft_size_ = fft_size;
    fft_.init(fft_size);

    auto n = fft_size;
    window_.resize(n);
    real_.resize(n);
    imag_.resize(n);
    sample_.resize(n);
    t_complex_.resize(n);
    f_complex_.resize(n);
    complex_.resize(n);

    for (int i = 0; i < n; ++i) {
        auto t = static_cast<float>(i) / (n - 1.0);
        window_[i] = 0.5 * (1.0 - std::cos(std::numbers::pi * 2 * t));
    }

    float energy = 0.0f;
    for (auto s : window_) {
        energy += s * s;
    }
    float window_scale = 1.0f / std::sqrt(energy);
    for (auto& s : window_) {
        s *= window_scale;
    }
}

void RFFT::Transform(std::span<float> block) {
    for (int i = 0; i < fft_size_; ++i) {
        sample_[i] = block[i] * window_[i];
    }
    fft_.fft(sample_.data(), real_.data(), imag_.data());
    for (int i = 0; auto& cpx : complex_) {
        cpx = std::complex{ real_[i],imag_[i] };
        ++i;
    }

    // shift sample
    for (int i = 1; i < fft_size_; ++i) {
        sample_[i] = block[i - 1] * window_[i];
    }
    sample_[0] = 0.0f;
    fft_.fft(sample_.data(), real_.data(), imag_.data());
    for (int i = 0; auto& cpx : f_complex_) {
        cpx = std::complex{ real_[i],imag_[i] };
        ++i;
    }

    // shift spectrum
    for (int i = 1; i < NumDataBins(); ++i) {
        t_complex_[i] = complex_[i - 1];
    }
    t_complex_[0] = 0.0f;

    // cross spectrum
    for (int i = 0; i < NumDataBins(); ++i) {
        f_complex_[i] = complex_[i] * std::conj(f_complex_[i]);
        t_complex_[i] = complex_[i] * std::conj(t_complex_[i]);
    }
}

float RFFT::CorrectFreqBin(int i) const {
    // return std::arg(f_complex_[i]) * fft_size_ / 2.0f / std::numbers::pi_v<float>;
    return Arg(f_complex_[i]) * fft_size_;
}

float RFFT::CoreectTime(int i) const {
    // return 0.5f - std::arg(t_complex_[i]) / 2.0f / std::numbers::pi_v<float>;
    return 0.5f - Arg(t_complex_[i]);
}

float RFFT::CoreectGain(int i) const {
    return std::abs(complex_[i]);
}

int RFFT::NumDataBins() const {
    return fft_size_ / 2 + 1;
}
