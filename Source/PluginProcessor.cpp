/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
Drone_pieceAudioProcessor::Drone_pieceAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  juce::AudioChannelSet::mono(), true)
#endif
                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                  )
#endif
{
}

Drone_pieceAudioProcessor::~Drone_pieceAudioProcessor()
{
}

//==============================================================================
const juce::String Drone_pieceAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Drone_pieceAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool Drone_pieceAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool Drone_pieceAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double Drone_pieceAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Drone_pieceAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int Drone_pieceAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Drone_pieceAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Drone_pieceAudioProcessor::getProgramName (int index)
{
    return {};
}

void Drone_pieceAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Drone_pieceAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
    //===================================================================
    // ---- BEGIN CUSTOM CODE ---- //

    
    // initialize thick synth variables
    SR = sampleRate;
    ts.setAllSampleRate(SR);
    ts.setLFOFrequencies();
    ts.initVector(SR);
    
    // initialize chase synth variables
    cs.setAllSampleRates(SR);
    cs.setAllFrequencies();
    cs.initVector(SR);
    
    // init reverb
    juce::Reverb::Parameters reverbParams;
    reverbParams.dryLevel = 0.5f;
    reverbParams.wetLevel = 0.3f;
    reverbParams.roomSize = 0.7f;
    reverb.setParameters(reverbParams);
    reverb.reset();
    
    // init filter
    TS_filter.setCoefficients(juce::IIRCoefficients::makeLowPass(SR, 300.0, 1.0));
    TS_filter.reset();
    
    // ---- END CUSTOM CODE ---- //
    
}

void Drone_pieceAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Drone_pieceAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    
    return true;
#endif
}
#endif

void Drone_pieceAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear (i, 0, buffer.getNumSamples());
    
    //===================================================================
    // ---- BEGIN CUSTOM CODE ---- //
    
    // init variables
    int numSamples = buffer.getNumSamples();
    float * leftChannel = buffer.getWritePointer(0);
    float * rightChannel = buffer.getWritePointer(1);

    // ---- START DSP LOOP ---- //
    for (int i = 0; i < numSamples; i++)
    {
        // set up samples before processing
        TS_raw_sample = 0.0f;
        TS_sample = 0.0f;
        CS_sample = 0.0f;
        
        // process thick synth sample (pre filter)
        TS_raw_sample = ts.process(SR);
        
        //send thick synth cutoff over to chase synth to... chase...
        cs.setTarget(ts.getCutoff());
        
        // process chase synth sample
        CS_sample = cs.process();
        
        // apply gain
        CS_sample *= CS_gain;

        // apply filter to thick synth
        TS_filter.setCoefficients(juce::IIRCoefficients::makeLowPass(SR, ts.getCutoff(), ts.getResMod()));
        TS_sample = TS_filter.processSingleSampleRaw(TS_raw_sample);

        //apply gain
        TS_sample *= TS_gain;
        
        // add samples to output channels and pan chase synth
        leftChannel[i] = TS_sample + (CS_sample * cs.getGain1());
        rightChannel[i] = TS_sample + (CS_sample * cs.getGain2());
    }
    // ---- END DSP LOOP ---- //
    
    // apply reverb
    reverb.processStereo(leftChannel, rightChannel, numSamples);
    
    // ---- END CUSTOM CODE ---- //
}

//==============================================================================



bool Drone_pieceAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Drone_pieceAudioProcessor::createEditor()
{
    return new Drone_pieceAudioProcessorEditor (*this);
}

//==============================================================================
void Drone_pieceAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Drone_pieceAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Drone_pieceAudioProcessor();
}
