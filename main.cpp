#ifndef UNICODE
#define UNICODE
#endif

#include <assert.h>
#include <stdint.h>
#include <math.h>

#include <windows.h>
#include <dsound.h>

#define PI 3.14159265f


int SamplesPerSecond = 48000;
int Channels = 2;
int BytesPerSample = 2;
int BufferLength = 2;
int BytesPerSecond = Channels * BytesPerSample * SamplesPerSecond;
int BufferSize = BufferLength * BytesPerSecond;

struct SoundContext {
    LPDIRECTSOUND DSContext;
    LPDIRECTSOUNDBUFFER PrimaryBuffer;
    LPDIRECTSOUNDBUFFER SecondaryBuffer;
};

typedef float (* WaveFn)(float TimeIndex, float Tone);

LRESULT CALLBACK MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int InitDSound(HWND hWnd, SoundContext *Context);
int FillBuffer(SoundContext Context, WaveFn Wave);

float SquareWave(float TimeIndex, float Tone);
float SineWave(float TimeIndex, float Tone);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLIne, int nCmdShow) {
    const WCHAR CLASS_NAME[] = L"Windows Sound Test";

    WNDCLASSEX window_class = {};
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.lpfnWndProc = MyWindowProc;
    window_class.hInstance = hInstance;
    window_class.lpszClassName = CLASS_NAME;
    
    RegisterClassEx(&window_class);

    HWND hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,
        CLASS_NAME,
        L"Windows Sound Systems Test",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hWnd == NULL) {
        return 0;
    }

    SoundContext Context;
    if (InitDSound(hWnd, &Context) == 0 && FillBuffer(Context, SineWave) == 0) {
        ShowWindow(hWnd, nCmdShow);

        Context.SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
    } else {
        MessageBoxEx(
            hWnd,
            L"Error initialising Audio system",
            NULL,
            MB_ICONERROR | MB_OK,
            0
        );
        PostQuitMessage(-1);
    }

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

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

int FillBuffer(SoundContext Context, WaveFn Wave) {
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

    int ToneHz = 256;
    int Volume = 3000;
    int SamplesToWrite = BufferSize / (BytesPerSample * Channels);

    int SamplesPerPeriod = (SamplesPerSecond / ToneHz);

    int16_t *Buffer = (int16_t *) BufferBlock1;

    // TODO: What to do when RunningSample wrap?
    // or, How to prevent it wrapping?
    for (int RunningSample = 0; RunningSample < SamplesToWrite; RunningSample++) {
        float TimeIndex = (float) RunningSample / SamplesPerSecond;

        int SampleValue = Wave(TimeIndex, ToneHz) * Volume;

        *Buffer++ = SampleValue;
        *Buffer++ = SampleValue;
    }
    
    if (FAILED(Context.SecondaryBuffer->Unlock(BufferBlock1, BufferBlock1Size,BufferBlock2, BufferBlock2Size))) {
        return -1;
    }

    return 0;
}

float SquareWave(float TimeIndex, float Tone) {
    return fmodf(TimeIndex * Tone * 2, 2) < 1.0f ? 1.0f : -1.0f;
}

float SineWave(float TimeIndex, float Tone) {
    return sinf(TimeIndex * 2 * PI * Tone);
}