#include <math.h>

#include "wave.h"

// Some experimentation with octaves and fading
// Liked the result and I'm keeping the code, but it's not useful in current form

float Modify(WaveFn Wave, float TimeIndex, float ToneHz) {
    int octaves = 3;

    float Modifier = 2.0f;
    float TotalIntensity = 1.329771f;
    
    float IntermediateSampleValue = Wave(TimeIndex, ToneHz);

    for (int octave = 1; octave <= octaves; octave++) {
        IntermediateSampleValue += Wave(TimeIndex, ToneHz * Modifier) * (1.0f / Modifier);
        // IntermediateSampleValue += Wave(TimeIndex, ToneHz / Modifier) * (1.0f / Modifier);
        // TotalIntensity += 2.0f/Modifier;
        Modifier *= 2;
    }

    return (IntermediateSampleValue / TotalIntensity) * (1 - (1.0f/(1.0f + expf(4.0f * (1.25f - TimeIndex)))));
}