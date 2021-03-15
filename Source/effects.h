/*
  ==============================================================================

    effects.h
    Created: 2 Mar 2021 4:34:36pm
    Author:  Christopher Duvall

  ==============================================================================
*/

#pragma once

/**
 Contains distortion effect very similar to week 3 tutorial.
 Ties distortion intensity to an incoming variable for a dynamic effect
*/


class Effects
{
    
public:
    // -------- SETTERS -------- //
    void adjustDistortion(float thresh) // ties distortoin effect to incoming variable
    {
        distThreshold = thresh;
    }
    
    // -------- PROCESS -------- //
    // bit crush distortion, based on week 3 tutorial
    float distortion(float inSample)
    {
        float outSample = inSample;
        
        if (outSample > distThreshold)
            outSample = distReturn;
        else if (outSample < -distThreshold)
            outSample = -distReturn;
        
        return outSample;
    }
    
    // tan distortion, based on week 3 tutorial
    // actually where signal comes in and goes out
    float tanDistortion(float inSample)
    {
        float outSample = inSample * tanGain;
        
        return distortion(tanhf(outSample));
    }
    
private:
    float distThreshold; // distortion intensity control
    float distReturn = 0.8f; // bit distortion control
    float tanGain = 2.0; // tan distortion control
};
