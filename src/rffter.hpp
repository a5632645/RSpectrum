#pragma once
#include "rfft.hpp"
#include "rfft2.hpp"
#include <span>
#include <array>

class RFFTer {
public:
    static constexpr int kFFTSize = 4096;
    static constexpr int kOverlay = 4;
    static constexpr int kHopSize = kFFTSize / kOverlay;
    static constexpr int kNumBins = kFFTSize / 2 + 1;

    void Init();
    void Process(std::span<float> block);
    
    RFFT rfft_;
    RFFT2 rfft2_;
    int algrithm_ = 0;
private:
    std::array<float, 32768> buffer_{};
    int numInput_{};
    int writeEnd_{};
    int writeAddBegin_{};
};
