/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "osc.h"
#include "thickSynth.h"
#include "chasingSynth.h"
#include "effects.h"

//==============================================================================
/**
*/
class Drone_pieceAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Drone_pieceAudioProcessor();
    ~Drone_pieceAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    
    // ---- initialize class variables ---- //
    ThickSynth ts;
    ChasingSynth cs;
    
    // ---- initialize process variables ---- //
    float SR; // sample rate
    float TS_raw_sample; // thick synth pre-filter sample
    float TS_sample; // thick synth post-filter sample
    float CS_sample; // chase synth sample
    float TS_gain = 0.6; // thick synth gain
    float CS_gain = 0.09; // chase synth gain
    
    juce::Random random;
    
    juce::IIRFilter TS_filter;
    
    juce::Reverb reverb;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Drone_pieceAudioProcessor)
};
