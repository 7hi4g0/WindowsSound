#include "wave.h"

// Some experimentation with octaves and fading
// Liked the result and I'm keeping the code, but it's not useful in current form

float Modify(WaveFn Wave) {
    int octaves = 5;

    float IntermediateSampleValue = 0;
    int Modifier = 1;
    float TotalIntensity = 0;

    for (int octave = 1; octave <= octaves; octave++) {
        IntermediateSampleValue += Wave(TimeIndex, ToneHz * Modifier) * (1.0f / Modifier);
        Modifier *= 2;
        TotalIntensity += 1.0f/Modifier;
    }

    return (IntermediateSampleValue / TotalIntensity) * (1 - (1.0f/(1.0f + expf(4.0f * (1.25f - TimeIndex)))));
}