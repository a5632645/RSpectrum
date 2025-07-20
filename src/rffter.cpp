#include "rffter.hpp"

void RFFTer::Init() {
    rfft_.Init(kFFTSize);
    rfft2_.Init(kFFTSize);
}

void RFFTer::Process(std::span<float> block) {
    std::copy(block.begin(), block.end(), buffer_.begin() + numInput_);
    numInput_ += static_cast<int>(block.size());
    while (numInput_ >= kFFTSize) {
        if (algrithm_ == 0) {
            rfft_.Transform(std::span{buffer_.data(), kFFTSize});
        }
        else if (algrithm_ == 1) {
            rfft2_.Transform(std::span{buffer_.data(), kFFTSize});
        }
        numInput_ -= kHopSize;
        for (int i = 0; i < numInput_; i++) {
            buffer_[i] = buffer_[i + kHopSize];
        }
    }
}
