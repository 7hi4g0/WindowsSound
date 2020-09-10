#ifndef WAVE_H

#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#define PI 3.14159265f


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