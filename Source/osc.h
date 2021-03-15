//
//  osc.h
//  Week4_Challenges
//
//  Created by Christopher Duvall on 2/24/21.
//
#pragma once

#ifndef osc_h
#define osc_h
#define TP juce::MathConstants<float>::twoPi


/**
This oscillator class is based on examples and tutorials from earlier in class.
The biggest difference is embedded in each Oscillator object, four different oscillator types exsist.
This is expressly for vector use, so that a vector of Oscillators from osc.h can have a range of timbres.
Another feature of this Oscillator class is a ability to reset the phase, which is helpful for smoothly going from 0 - 1.

 IMPORTANT: Always initialize sample rate BEFORE initializing frequency. Otherwise phase delta might not be right.
*/

class Oscillator
{
public:
    // -------- CONSTRUCTOR -------- //
    Oscillator() {};
    
    
    // -------- SETTERS -------- //
    void setSampleRate(float SR) // sample rate
    {
        sampleRate = SR;
    }
    
    void setFreq(float freq) // frequency and phase delta
    {
        frequency = freq;
        phaseDelta = freq / sampleRate;
    }
    
    void setPulseWidth(float pw) // pulse width for square waves
    {
        pulseWidth = pw;
    }
    
    void resetPhase() // set phase back to 0, for regulating parameters
    {
        phase = 0;
    }
    
    // -------- GETTERS -------- //
    float getSampleRate()
    {
        return sampleRate;
    }
    
    float getFreq()
    {
        return frequency;
    }
    
    // -------- METHODS -------- //
    float process() // phasor
    {
        phase += phaseDelta;
        
        if (phase > 1.0f)
            phase -= 1.0f;
        
        return phase;
    }
    
    float sineWave() // sine wave
    {
        //float sineVal = sin( process() * 2 * 3.141592653589793 );
        float sineVal = sin( process() * TP );
        return sineVal;
    }
    
    float squareWave() // square wave
    {
        float squareVal = 0.5f;
        if (process() > pulseWidth)
            squareVal = -0.5f;
        
        return squareVal;
    }
    
    float triWave() // triangle wave
    {
        float triVal = fabs(process() - 0.5) - 0.5;
        return triVal * 3;
    }
    
private:
    float phase = 0.0f;
    float frequency;
    float sampleRate;
    float phaseDelta;
    float pulseWidth;
};


#endif /* osc_h */
