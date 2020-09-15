#ifndef WAVE_H

#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#define PI 3.14159265f

#ifdef _DEBUG

#include <stdio.h>

void output(const char *fmt, ...) {
    static char _stringBuffer[256];

    va_list args;
    va_start(args, fmt);

    vsprintf_s(_stringBuffer, fmt, args);

    va_end(args);

    OutputDebugStringA(_stringBuffer);
}

#endif

uint32_t SamplesPerSecond = 48000;
uint8_t Channels = 2;
uint8_t BytesPerSample = 2;
int BufferLength = 2;
int BytesPerSecond = Channels * BytesPerSample * SamplesPerSecond;
int BufferSize = BufferLength * BytesPerSecond;

struct SoundContext;

typedef float (* WaveFn)(float TimeIndex, float Tone);


void FillBuffer(int16_t * Buffer, uint32_t SamplesToWrite, WaveFn Wave, float ToneHz, int Volume);


#define WAVE_H
#endif