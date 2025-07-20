#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include <cmath>
#include <objidlbase.h>

constexpr int kFps = 30;
//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    auto& apvts = *processorRef.value_tree_;
    gradient_.addColour(0.0, juce::Colours::black);
    gradient_.addColour(0.5, juce::Colours::purple);
    gradient_.addColour(1.0, juce::Colours::yellow);

    algrithm_.addItem("RFFT", 1);
    algrithm_.addItem("RTTF2", 2);
    addAndMakeVisible(algrithm_);
    algrithm_attachment_ = std::make_unique<juce::ComboBoxParameterAttachment>(*apvts.getParameter("ALGRITHM"), algrithm_);

    setSize(600, 300);
    setResizable(true, true);
    startTimerHz(kFps);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
    algrithm_attachment_ = nullptr;
    stopTimer();
}

//==============================================================================
static float FreqToPitch(float req) {
    return 69.0f + 12.0f * std::log2(req / 440.0f);
}

void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g) {
    auto b = getLocalBounds().toFloat();
    b.removeFromBottom(algrithm_.getHeight());
    g.drawImage(image_, b);
}

constexpr int kTime = 1; // seconds
constexpr int kTimeResolution = 8;
void AudioPluginAudioProcessorEditor::resized() {
    auto b = getLocalBounds();
    auto bottom = b.removeFromBottom(20);
    algrithm_.setBounds(bottom.removeFromLeft(100));
    image_ = juce::Image{juce::Image::RGB, kTime * kFps * kTimeResolution, b.getHeight(), true};
}

void AudioPluginAudioProcessorEditor::timerCallback() {
    image_.moveImageSection(0, 0, kTimeResolution / 4, 0, image_.getWidth() - kTimeResolution / 4, image_.getHeight());
    image_.clear({image_.getWidth() - kTimeResolution / 4, 0, kTimeResolution / 4, image_.getHeight()});

    static const float upper = FreqToPitch(20000.0f);
    static const float bottom = FreqToPitch(20.0f);
    constexpr float db_upper = 0.0f;
    constexpr float db_bottom = -60.0f;
    float fs = static_cast<float>(processorRef.getSampleRate());
    int xindex_begin = image_.getWidth() - kTimeResolution;
    int algrithm = algrithm_.getSelectedItemIndex();

    if (algrithm == 1) {
        auto& rfft = processorRef.rffter_.rfft2_;
        int nbins = rfft.NumDataBins();
        int fft_size = rfft.FFtSize();
        for (int i = 0; i < nbins; ++i) {
            float gain = rfft.CoreectGain(i);
            float freq = rfft.CorrectFreqBin(i) * fs / fft_size;
            if (freq < 20.0f || freq > 20000.0f) continue;
            float pitch = FreqToPitch(freq);
            int yindex = (int)juce::jmap<float>(pitch, bottom, upper, image_.getHeight() - 1.0f, 0.0f);
            float time = rfft.CoreectTime(i) * 8.0f;
            int xindex = (int)juce::jmap<float>(time, -0.5f, 0.5f, 0.0f, kTimeResolution - 1.0f);
            float db = 20.0f * std::log10(gain + 1e-10f);
            db = std::clamp(db, db_bottom, db_upper);
            float db_norm = (db - db_bottom) / (db_upper - db_bottom);
            auto color = gradient_.getColourAtPosition(db_norm);
            auto color2 = image_.getPixelAt(xindex_begin + xindex, yindex);
            if (color.getLightness() > color2.getLightness()) {
                image_.setPixelAt(xindex_begin + xindex, yindex, color);
            }
        }
    }
    else if (algrithm == 0) {
        auto& rfft = processorRef.rffter_.rfft_;
        int nbins = rfft.NumDataBins();
        int fft_size = rfft.FFtSize();
        for (int i = 0; i < nbins; ++i) {
            float gain = rfft.CoreectGain(i);
            float freq = rfft.CorrectFreqBin(i) * fs / fft_size;
            if (freq < 20.0f || freq > 20000.0f) continue;
            float pitch = FreqToPitch(freq);
            int yindex = (int)juce::jmap<float>(pitch, bottom, upper, image_.getHeight() - 1.0f, 0.0f);
            float time = rfft.CoreectTime(i);
            int xindex = (int)juce::jmap<float>(time, -0.5f, 0.5f, 0.0f, kTimeResolution - 1.0f);
            float db = 20.0f * std::log10(gain + 1e-10f);
            db = std::clamp(db, db_bottom, db_upper);
            float db_norm = (db - db_bottom) / (db_upper - db_bottom);
            auto color = gradient_.getColourAtPosition(db_norm);
            auto color2 = image_.getPixelAt(xindex_begin + xindex, yindex);
            if (color.getLightness() > color2.getLightness()) {
                image_.setPixelAt(xindex_begin + xindex, yindex, color);
            }
        }
    }

    repaint();
}
