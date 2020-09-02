#ifndef UNICODE
#define UNICODE
#endif

#include <stdint.h>
#include <assert.h>
#include <windows.h>
#include <dsound.h>
#include <math.h>

#define PI 3.14159265f


int SamplesPerSecond = 48000;
int Channels = 2;
int BytesPerSample = 2;
int BufferLength = 2;
int BytesPerSecond = Channels * BytesPerSample * SamplesPerSecond;
int BufferSize = BufferLength * BytesPerSecond;

LPDIRECTSOUND DSContext;
DSBUFFERDESC PrimaryBufferDesc;
LPDIRECTSOUNDBUFFER PrimaryBuffer;
WAVEFORMATEX WaveFormat;
DSBUFFERDESC SecondaryBufferDesc;
LPDIRECTSOUNDBUFFER SecondaryBuffer;

typedef float (* WaveFn)(float TimeIndex, float Tone);

LRESULT CALLBACK MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitDSound(HWND hWnd);
void FillBuffer(WaveFn Wave);

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

    InitDSound(hWnd);
    FillBuffer(SineWave);
    ShowWindow(hWnd, nCmdShow);

    SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

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

void InitDSound(HWND hWnd) {
    if(FAILED(DirectSoundCreate(NULL, &DSContext, NULL))) {
        PostQuitMessage(-1);
        return;
    }

    // TODO: Do I need it?
    if (FAILED(DSContext->SetCooperativeLevel(hWnd, DSSCL_PRIORITY))) {
        PostQuitMessage(-1);
        return;
    }

    PrimaryBufferDesc = {};
    PrimaryBufferDesc.dwSize = sizeof(DSBUFFERDESC);
    PrimaryBufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
    PrimaryBufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;

    if(FAILED(DSContext->CreateSoundBuffer(&PrimaryBufferDesc, &PrimaryBuffer, NULL))) {
        // TODO: Make it more robust and alert or log the error
        // This quits the application but doesn't really work.
        // If anything tries to use the values set here it will segfault
        PostQuitMessage(-1);
        return;
    }

    WaveFormat = {};
    WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
    WaveFormat.nChannels = Channels;
    WaveFormat.nSamplesPerSec = SamplesPerSecond;
    WaveFormat.nAvgBytesPerSec = BytesPerSecond;
    WaveFormat.nBlockAlign = Channels * BytesPerSample;
    WaveFormat.wBitsPerSample = BytesPerSample * 8;

    // TODO: Do I need it?
    if (FAILED(PrimaryBuffer->SetFormat(&WaveFormat))) {
        PostQuitMessage(-1);
        return;
    }

    SecondaryBufferDesc = {};
    SecondaryBufferDesc.dwSize = sizeof(DSBUFFERDESC);
    SecondaryBufferDesc.dwFlags = 0;
    SecondaryBufferDesc.dwBufferBytes = BufferSize;
    SecondaryBufferDesc.lpwfxFormat = &WaveFormat;
    SecondaryBufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;

    if(FAILED(DSContext->CreateSoundBuffer(&SecondaryBufferDesc, &SecondaryBuffer, NULL))) {
        PostQuitMessage(-1);
        return;
    }
}

void FillBuffer(WaveFn Wave) {
    LPVOID BufferBlock1;
    DWORD BufferBlock1Size;
    LPVOID BufferBlock2;
    DWORD BufferBlock2Size;

    if (FAILED(SecondaryBuffer->Lock(0, BufferSize, &BufferBlock1, &BufferBlock1Size, &BufferBlock2, &BufferBlock2Size, 0))) {
        PostQuitMessage(-1);
        return;
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
    
    if (FAILED(SecondaryBuffer->Unlock(BufferBlock1, BufferBlock1Size,BufferBlock2, BufferBlock2Size))) {
        PostQuitMessage(-1);
        return;
    }
}

float SquareWave(float TimeIndex, float Tone) {
    return fmodf(TimeIndex / (Tone * 2), 2) < 1.0f ? 1.0f : -1.0f;
}

float SineWave(float TimeIndex, float Tone) {
    return sinf(TimeIndex * 2 * PI * Tone);
}