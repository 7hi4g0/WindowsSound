#ifndef WAVE_H

#include <assert.h>
#include <stdint.h>

#define PI 3.14159265f


int SamplesPerSecond = 48000;
int Channels = 2;
int BytesPerSample = 2;
int BufferLength = 2;
int BytesPerSecond = Channels * BytesPerSample * SamplesPerSecond;
int BufferSize = BufferLength * BytesPerSecond;

struct SoundContext;

typedef float (* WaveFn)(float TimeIndex, float Tone);


void FillBuffer(int16_t * Buffer, int SamplesToWrite, WaveFn Wave, float ToneHz, int Volume);

#define WAVE_H
#endif