#include "wave.h"

void KarplusStrong(int16_t * Buffer, int SamplesToWrite, int SamplingRate, float ToneHz, int Volume) {
    int SampleWindowSize = SamplingRate / ToneHz;

    int16_t *SampleWindowBuffer = Buffer;
    int SampleIndex = 0;

    for (; SampleIndex < SampleWindowSize; SampleIndex++) {
        uint32_t value;

        value = rand();

        float SampleValue = ((float) value / RAND_MAX) * 2.0f - 1.0f;

        *Buffer++ = SampleValue * Volume;
        *Buffer++ = SampleValue * Volume;
    }

    for (; SampleIndex < SamplesToWrite; SampleIndex++) {
        int16_t FirstFeedbackValue = *SampleWindowBuffer;
        SampleWindowBuffer += 2;
        int16_t SecondFeedbackValue = *SampleWindowBuffer;

        int16_t SampleValue = (float)(FirstFeedbackValue + SecondFeedbackValue) * 0.994f / 2;
        *Buffer++ = SampleValue;
        *Buffer++ = SampleValue;
    }
}