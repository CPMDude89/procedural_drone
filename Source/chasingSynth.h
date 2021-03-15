/*
 ==============================================================================
 
 chasingSynth.h
 Created: 2 Mar 2021 11:06:07am
 Author:  Christopher Duvall
 
 ==============================================================================
 */

#pragma once

#include "osc.h"
#include "effects.h"
#include <JuceHeader.h>
#include "PluginProcessor.h"

/**
 This synth creates a high frequency, procedurally generated sonic element.
 Named "ChasingSynth" because it is constantly targeting new frequencies and smoothly going up or down to them.
 Set up to chase frequencies from thick synth's filter cutoff.
 A very wide range of speeds create an unpredictable, playful quality.
 Calls in a bit-distortion effect, which is at maximum at very beginning of new chase, and will smoothly decrase as gets closer to target frequency
*/

class ChasingSynth : Oscillator
{
    
public:
    // -------- SETTERS -------- //
    void setAllSampleRates(double SR) // LFO sample rate
    {
        lfo1.setSampleRate(SR);
    }
    
    void setAllFrequencies() // frequencies
    {
        lfo1.setFreq(lfoFreq1);
        vectorFreq = targetFreq / 2;
        detune = random.nextFloat() + 1.0f;
    }
    
    void setTarget(float cutoff) // new frequency to chase
    {
        newTarget = cutoff / 2.0f;
    }
    
    void setPan() // regulate panning
    {
        if (panSwitch == false)
            panSwitch = true;
        else
            panSwitch = false;
    }
    
    // -------- GETTERS -------- //
    float getGain1() // left ear
    {
        return gain1;
    }
    
    float getGain2() // right ear
    {
        return gain2;
    }
    
    // -------- METHODS -------- //
    
    // setup vector
    void initVector (double SR)
    {
        for (int i = 0; i < oscCount; i++)
        {
            oscVector.push_back(Oscillator());
            oscVector[i].setSampleRate(SR);
            
            if (i == 0)
                oscVector[i].setFreq(vectorFreq * (i + 1));
            
            else
                oscVector[i].setFreq((oscVector[i - 1]).getFreq() * 1.1);
        }
    }
    
    // regulate panning
    // mod is lfo frequency
    void pan()
    {
        if (panSwitch == false)
        {
            gain1 = mod;
            gain2 = (1.0 - mod);
        }
        else
        {
            gain2 = mod;
            gain1 = (1.0 - mod);
        }
    }
    
    // resets LFO frequencies when synth reaches target frequency
    void resetFrequencies()
    {
        if (up == true) // if new target frequency is higher, start lower
            vectorFreq =  targetFreq / 2;
        else // if new target frequency is lower, start higher
            vectorFreq = targetFreq * 2;
        
        // create new LFO frequency
        lfo1.setFreq(random.nextFloat() * 1.1);
        detune = random.nextFloat() + 1.0f;
    }
    
    // find new frequency to chase
    void resetTarget(float prevTarget)
    {
        if (newTarget > targetFreq) // compare new target to old target
        {
            up = true; // going up
        }
        else
            up = false; // going down
        
        targetFreq = newTarget;
        resetFrequencies(); // reset LFO frequency
        setPan(); // keep bouncing back and forth
    }
    
    // go after target frequency
    void chase(float target)
    {
        // chase upwards
        if (oscVector[0].getFreq() < target && up == true)
        {
            mod = lfo1.sineWave(); // mod keeps everything together
            effect.adjustDistortion(mod); // such as distortion effect
            oscVector[0].setFreq( vectorFreq * (0 + 1) * (mod + 1.0f));
        }
        
        // CAUGHT!
        else if (oscVector[0].getFreq() >= target && up == true)
        {
            lfo1.resetPhase(); // set LFO back to 0
            resetTarget(target); // find new frequency to chase
        }
        
        // chase downwards (same set up as above)
        else if (oscVector[0].getFreq() > target && up == false)
        {
            mod = lfo1.sineWave();
            effect.adjustDistortion(mod);
            oscVector[0].setFreq( vectorFreq * (0 + 1) / (mod + 1.0f));
        }
        
        // CAUGHT! (same set up as above)
        else if (oscVector[0].getFreq() <= target && up == false)
        {
            lfo1.resetPhase();
            resetTarget(target);
        }
    }
    
    // -------- PROCESS -------- //
    // top - level control
    // outputs signal to controlling program
    // iterates over sounding vector for frequency modulation
    float process()
    {
        pan(); // regulate pan
        
        float sample = 0.0f;
        float processedSample = 0.0f;
        
        chase(targetFreq); // CHASE!
        
        for (int i = 0; i < oscCount; i++)
        {
            if (i % 2 == 0)
            {
                sample += oscVector[i].triWave(); // triangle wave foundation
            }
                
            else
            {
                // base frequency off lowest frequency
                oscVector[i].setFreq((oscVector[i - 1]).getFreq() * detune);
                sample += oscVector[i].triWave();
            }
            
            sample *= vectorVol; // regulate vector gain
        }
        
        // bring in distortion effect
        processedSample = effect.tanDistortion(sample);
        
        return processedSample;
    }
    
private:
    
    // vector variables
    int oscCount = 2; // top level vector amount control
    float vectorVol = 0.9 / (float)oscCount; // dynamic vector gain
    
    // sounding oscillator variables
    float vectorFreq; // frequency
    float detune; // detune
    
    // LFO variables
    float lfoFreq1 = 0.05f; // frequency
    
    // panning variables
    float gain1 = 0.0f; // left
    float gain2 = 1.0f; // right
    bool panSwitch = false; // starting point, switches each reset
    
    // target variables
    float newTarget; // next frequency to chase
    float targetFreq = 700.0f; // starting frequency to chase
    bool up = true; // going up or down
    
    double mod; // handy unifying variable, just set to LFO to regulate panning, distortion,
                // and frequency modulation

    
    std::vector<Oscillator> oscVector; // sounding oscillator vector
    
    Oscillator lfo1; // movement LFO
    
    Effects effect;
    
    juce::Random random;
};
