#ifndef UNICODE
#define UNICODE
#endif


#include <math.h>

#include <windows.h>

#include "wave.h"

#define X_AUDIO
#undef DIRECT_SOUND

#ifdef DIRECT_SOUND
#include "dsound.cpp"
#elif defined(X_AUDIO)
#include "xaudio.cpp"
#endif

LRESULT CALLBACK MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

float SquareWave(float TimeIndex, float Tone);
float SineWave(float TimeIndex, float Tone);
float SawtoothWave(float TimeIndex, float Tone);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLIne, int nCmdShow) {
    const WCHAR CLASS_NAME[] = L"Windows Sound Test";
    const WCHAR WINDOW_NAME[] = L"Windows Sound Systems Test";

    WNDCLASSEX window_class = {};
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.lpfnWndProc = MyWindowProc;
    window_class.hInstance = hInstance;
    window_class.lpszClassName = CLASS_NAME;
    
    RegisterClassEx(&window_class);

    HWND hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,
        CLASS_NAME,
        WINDOW_NAME,
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
    // TODO: This is a bit ugly right now. Can be improved.
#ifdef DIRECT_SOUND
    if (InitDSound(hWnd, &Context) == 0 && FillDSBuffer(Context, SineWave, 64, 3000) == 0) {
        ShowWindow(hWnd, nCmdShow);

        DSPlay(Context);
#elif defined(X_AUDIO)
    if (InitXAudio(hWnd, &Context) == 0 && FillXAudioBuffer(Context, SineWave, 64, 3000) == 0) {
        ShowWindow(hWnd, nCmdShow);

        XAudioPlay(Context);
#endif
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

void FillBuffer(int16_t * Buffer, int SamplesToWrite, WaveFn Wave, float ToneHz, int Volume) {

    // TODO: What to do when RunningSample wrap?
    // or, How to prevent it wrapping?
    for (uint32_t RunningSample = 0; RunningSample < SamplesToWrite; RunningSample++) {
        float TimeIndex = (float) RunningSample / SamplesPerSecond;

        int16_t SampleValue = Wave(TimeIndex, ToneHz) * Volume;

        *Buffer++ = SampleValue;
        *Buffer++ = SampleValue;
    }
}

float SquareWave(float TimeIndex, float Tone) {
    return fmodf(TimeIndex * Tone * 2, 2) < 1.0f ? 1.0f : -1.0f;
}

float SineWave(float TimeIndex, float Tone) {
    return sinf(TimeIndex * 2 * PI * Tone);
}

float SawtoothWave(float TimeIndex, float Tone) {
    return fmodf(TimeIndex * Tone + 1.0f, 2) - 1.0f;
}