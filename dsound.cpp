
#include <dsound.h>

#include "wave.h"

struct SoundContext {
    LPDIRECTSOUND DSContext;
    LPDIRECTSOUNDBUFFER PrimaryBuffer;
    LPDIRECTSOUNDBUFFER SecondaryBuffer;
};

int InitDSound(HWND hWnd, SoundContext *Context) {
    // TODO: Improve this
    assert(Context != NULL);

    LPDIRECTSOUND DSContext;
    LPDIRECTSOUNDBUFFER PrimaryBuffer;
    LPDIRECTSOUNDBUFFER SecondaryBuffer;

    if(FAILED(DirectSoundCreate(NULL, &DSContext, NULL))) {
        return -1;
    }

    // TODO: Do I need it?
    if (FAILED(DSContext->SetCooperativeLevel(hWnd, DSSCL_PRIORITY))) {
        return -1;
    }

    DSBUFFERDESC PrimaryBufferDesc = {};
    PrimaryBufferDesc.dwSize = sizeof(DSBUFFERDESC);
    PrimaryBufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
    PrimaryBufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;

    if(FAILED(DSContext->CreateSoundBuffer(&PrimaryBufferDesc, &PrimaryBuffer, NULL))) {
        return -1;
    }

    WAVEFORMATEX WaveFormat = {};
    WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
    WaveFormat.nChannels = Channels;
    WaveFormat.nSamplesPerSec = SamplesPerSecond;
    WaveFormat.nAvgBytesPerSec = BytesPerSecond;
    WaveFormat.nBlockAlign = Channels * BytesPerSample;
    WaveFormat.wBitsPerSample = BytesPerSample * 8;

    // TODO: Do I need it?
    if (FAILED(PrimaryBuffer->SetFormat(&WaveFormat))) {
        return -1;
    }

    DSBUFFERDESC SecondaryBufferDesc = {};
    SecondaryBufferDesc.dwSize = sizeof(DSBUFFERDESC);
    SecondaryBufferDesc.dwFlags = 0;
    SecondaryBufferDesc.dwBufferBytes = BufferSize;
    SecondaryBufferDesc.lpwfxFormat = &WaveFormat;
    SecondaryBufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;

    if(FAILED(DSContext->CreateSoundBuffer(&SecondaryBufferDesc, &SecondaryBuffer, NULL))) {
        return -1;
    }

    *Context = {
        DSContext,
        PrimaryBuffer,
        SecondaryBuffer
    };

    return 0;
}

int FillDSBuffer(SoundContext Context, WaveFn Wave, float ToneHz, int Volume) {
    LPVOID BufferBlock1;
    DWORD BufferBlock1Size;
    LPVOID BufferBlock2;
    DWORD BufferBlock2Size;

    if (FAILED(Context.SecondaryBuffer->Lock(0, BufferSize, &BufferBlock1, &BufferBlock1Size, &BufferBlock2, &BufferBlock2Size, 0))) {
        return -1;
    }

    assert(BufferBlock2 == NULL);
    assert(BufferBlock2Size == 0);
    assert(BufferBlock1 != NULL);
    assert(BufferBlock1Size == BufferSize);

    int SamplesToWrite = BufferSize / (BytesPerSample * Channels);

    FillBuffer((int16_t *) BufferBlock1, SamplesToWrite, Wave, ToneHz, Volume);
    
    if (FAILED(Context.SecondaryBuffer->Unlock(BufferBlock1, BufferBlock1Size,BufferBlock2, BufferBlock2Size))) {
        return -1;
    }

    return 0;
}

void DSPlay(SoundContext Context) {
    Context.SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
}