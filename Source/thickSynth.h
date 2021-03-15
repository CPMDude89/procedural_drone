/*
  ==============================================================================

    thickSynth.h
    Created: 1 Mar 2021 9:27:02am
    Author:  Christopher Duvall

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "osc.h"

/**
 The heart of this class is a vector of oscillators, with alternating wave types.
 Provides the low frequencies and the main substance of the drone.
 
 A vector of LFO's to control gain creates interesting beating frequencies with amplitude modulation
 Amount of elements in both vectors change linearly over time. (Start at 3, iterate up to 10, step down to 3... etc etc)
 Amplitude modulation changes randomly over time. Each sounding oscillator with have a different modulation, but balanced gain.
 Filter cutoff and resonance controlled by LFO's.
 
 oscCount is amount of elements in vectors at any given time.
*/

class ThickSynth : Oscillator
{

public:
    // -------- SETTERS -------- //
    void setAllSampleRate(double SR) // sample rate
    {
        lfo1.setSampleRate(SR);
        lfo2.setSampleRate(SR);
        counterMax = (int)SR;
    }
    
    void setLFOFrequencies() // frequencies (fixed)
    {
        lfo1.setFreq(lfoFreq1);
        lfo2.setFreq(lfoFreq2);
    }

    void setCutoff(float CO) // filter cutoff
    {
        cutoff = CO;
    }
    
    void setResMod(float RM) // filter resonance
    {
        resMod = RM;
    }
    
    void setVectorVol() // dynamically scaling oscillator vector gain
    {
        vectorVol = 0.9 / (float)oscCount;
    }
    
    // -------- GETTERS -------- //
    float getCutoff() // filter cutoff
    {
        return cutoff;
    }
    
    float getResMod() // filter resonance
    {
        return resMod;
    }
    
    // -------- METHODS -------- //
    
    // initialize vectors
    // oscVector is sounding vector
    // gainVector creates procedurally generated beating amplitude modilation
    void initVector(double _SR)
    {
        // init sounding oscillator vector
        for (int i = 0; i < oscCount; i++)
        {
            oscVector.push_back(Oscillator());
            oscVector[i].setSampleRate(_SR);
            oscVector[i].setPulseWidth(vectorPW);
            oscVector[i].setFreq(vectorFreq * (i + 1));
        }
        
        // init gain oscillator
        for (int i = 0; i < oscCount; i++)
        {
            gainVector.push_back(Oscillator());
            gainVector[i].setSampleRate(_SR);
            float test = randommm.nextFloat() * (i + randommm.nextFloat());
            gainVector[i].setFreq( test );
            std::cout << i << "'s gain is: " << test << "\n";
        }
    }
    
    // Amplitude modulation dynamically created here
    // Each LFO element in vector has a different frequency
    // counter1 keeps track of frequencies, counter2 keeps track of number of elements in vector
    float vectorGain(int iterator)
    {
        // iterate counters
        counter1++; // frequencies
        counter2++; // amount of elements in vectors
        
        // keep track of incrementing or decrementing amounts of elements in vectors
        if (oscCount == 11)
            up = false; // up == false means going down
        if (oscCount == 3 && up == false)
            up = true; // up == true means going up (INITIAL SETTING)
        
        // randomly give one gain vector element a different frequency
        if (counter1 == counterMax * 30 )
        {
            int next = randommm.nextInt(oscCount); // pick a random element
            
            // create random gain frequency
            float nextGain = randommm.nextInt(gainMax) * (randommm.nextFloat() + 0.1);
            
            gainVector[next].setFreq(nextGain); // implement changes
            counter1 = 0; // reset counter
            gainMax += 2; // increase frequency maximum, increase potential entropy
        }
        
        // linearly create vector elements (both vectors)
        if (counter2 == counterMax * 70 && up == true) // going up
        {
            addElement(counterMax);
            counter2 = 0;
        }
        else if (counter2 == counterMax * 70 && up == false) // going down
        {
            removeElement();
            counter2 = 0;
        }
        
        float gain = gainVector[iterator].sineWave(); // keep vector elements' gain in check
        
        return gain;
    }
    
    // increase the amount of vector elements in both vectors
    void addElement(int SR)
    {
        oscCount++; // iterate osc count
        
        setVectorVol(); // balance volume across all oscillators
        
        // push new elements into vectors
        oscVector.push_back(Oscillator());
        gainVector.push_back(Oscillator());
        
        // set up new sounding oscillator in vector
        oscVector[oscCount - 1].setSampleRate((float)SR);
        oscVector[oscCount - 1].setFreq(vectorFreq * oscCount);
        oscVector[oscCount - 1].setPulseWidth(vectorPW);
        
        // set up new gain LFO in vector
        gainVector[oscCount - 1].setSampleRate(SR);
        gainVector[oscCount - 1].setFreq(randommm.nextFloat() * ((oscCount - 1) + randommm.nextFloat()));
    }
    
    // decrement vector elemtns
    void removeElement()
    {
        oscCount--; // regulate top-level vector element variable
        setVectorVol(); // regulate oscillator gain
        oscVector.pop_back();
        gainVector.pop_back();
    }

    // -------- PROCESS -------- //
    // steps through filter LFO'ed params
    // steps through oscVector frequency modulations
    // outputs sample to top level program
    float process(double _SR)
    {   
        setCutoff(lfo1.sineWave() * 1200.0 + 1850.0 ); // filter cutoff
        setResMod(lfo2.sineWave() + 1.0 * 5.0 ); // filter resonance
        
        float raw = 0.0f; // starter sample

        // regulate oscVector (sounding oscillator vector)
        for (int j = 0; j < oscCount; j++)
        {
            // frequency modulation amount
            float mod = (lfo2.sineWave() + randommm.nextFloat() + 1.1) ;
            
            // three different kinds of wave types in this vector
            // although frequencies are randomized, they are generated in clean ratios to each other
            if (j % 3 == 0)
            {
                oscVector[j].setFreq(vectorFreq * (j + 0.4 ) * mod);
                raw += oscVector[j].squareWave() * vectorVol * 0.5;
            }
            else if (j % 3 == 1)
            {
                oscVector[j].setFreq(vectorFreq * (j + 1.2) * mod);
                raw += oscVector[j].sineWave() * vectorVol * 1.2;
            }
            else
            {
                oscVector[j].setFreq(vectorFreq * (j + 1.8) * mod);
                raw += oscVector[j].triWave() * vectorVol;
            }
            
            raw *= vectorGain((j)); // volume regulation
        }
        
        return raw;
    }
    
private:
    // init lfos
    Oscillator lfo1;
    Oscillator lfo2;
    
    //init sounding oscillator and gain LFO vectors
    std::vector<Oscillator> oscVector;
    std::vector<Oscillator> gainVector;
    
    // init sounding oscillator variables
    int oscCount = 3; // top-level oscillator regulation amount
    float vectorVol = 0.9 / (float)oscCount; // sounding oscillator gain regulator
    float vectorFreq = 45.0f; // starting vector frequency
    float vectorPW = 0.4f; // square wave pulse width
    
    // init LFO variables
    float lfoFreq1 = .0612f; // mostly for filter cutoff modulation
    float lfoFreq2 = 0.005f; // filter resonance modulation and oscVector frequency modulation
    int counterMax; // integer version of sample rate, used for timing
    int counter1 = 0; // frequencies
    int counter2 = 0; // amount of elements in vectors
    int gainMax = 2; // initial frequency maximum
    
    bool up = true; // incrementing or decrementing vector elements
    
    juce::Random randommm; // juce random object
    
    //init filter variables
    float cutoff;
    float resMod;
};
