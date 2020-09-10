#ifndef UNICODE
#define UNICODE
#endif


#include <math.h>

#include <windows.h>

#include "wave.h"
#include "fade.cpp"
#include "effects.cpp"

#define X_AUDIO
// #define DIRECT_SOUND

#ifdef DIRECT_SOUND
#include "dsound.cpp"
#elif defined(X_AUDIO)
#include "xaudio.cpp"
#endif

BITMAPINFO BitmapInfo;
void *BitmapMemory;
// LONG BitmapWidth = SamplesPerSecond * 2;
// LONG BitmapHeight = 3000 * 2 + 1;
uint16_t BitmapWidth = 9600;
uint16_t BitmapHeight = 5001;

void InitImageBuffer();
void DrawGraph(HWND hWnd, HDC DeviceContext);

LRESULT CALLBACK MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

float SquareWave(float TimeIndex, float Tone);
float SineWave(float TimeIndex, float Tone);
float SawtoothWave(float TimeIndex, float Tone);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const WCHAR CLASS_NAME[] = L"Windows Sound Test";
    const WCHAR WINDOW_NAME[] = L"Windows Sound Systems Test";

    WNDCLASSEX window_class = {};
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
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

    InitImageBuffer();

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

    int Running = true;
    while (Running) {
        MSG msg = {};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                Running = false;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // HDC DeviceContext = GetDC(hWnd);

        // DrawGraph(hWnd, DeviceContext);

        // ReleaseDC(hWnd, DeviceContext);
    }

    return 0;
}

LRESULT CALLBACK MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(hwnd, &Paint);
            DrawGraph(hwnd, DeviceContext);
            EndPaint(hwnd, &Paint);
            return 0;
        }
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void InitImageBuffer() {
    BitmapInfo = {};
    BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    uint32_t BitmapSize = BitmapWidth * BitmapHeight * 4;

    BitmapMemory = VirtualAlloc(0, BitmapSize, MEM_COMMIT, PAGE_READWRITE);

    uint32_t *CurrentPixel = (uint32_t *) BitmapMemory;

    for (uint16_t Y = 0; Y < BitmapHeight; Y++) {
        for (uint16_t X = 0; X < BitmapWidth; X++) {
            *CurrentPixel++ = 0x00808080;
        }
    }
}

void DrawGraph(HWND hWnd, HDC DeviceContext) {
    RECT WindowRect;
    GetClientRect(hWnd, &WindowRect);
    int WindowWidth = WindowRect.right - WindowRect.left;
    int WindowHeight = WindowRect.bottom - WindowRect.top;

    StretchDIBits(DeviceContext,
                    0, 0, WindowWidth, WindowHeight,
                    0, 0, BitmapWidth, BitmapHeight,
                    BitmapMemory, &BitmapInfo,
                    DIB_RGB_COLORS, SRCCOPY);
}

void FillBuffer(int16_t * Buffer, uint32_t SamplesToWrite, WaveFn Wave, float ToneHz, int Volume) {

    // TODO: What to do when RunningSample wrap?
    // or, How to prevent it wrapping?
    for (uint32_t RunningSample = 0; RunningSample < SamplesToWrite; RunningSample++) {
        float TimeIndex = (float) RunningSample / SamplesPerSecond;

        int16_t SampleValue = (int16_t) (Modify(Wave, TimeIndex, ToneHz) * Volume);

        *Buffer++ = SampleValue;
        *Buffer++ = SampleValue;

        if (RunningSample < BitmapWidth) {
            uint16_t X = (uint16_t) RunningSample;
            uint16_t Y = ((SampleValue + 3000) / 2) + 1000;

            ((uint32_t *)BitmapMemory)[Y * BitmapWidth + X] = 0;
        }
    }
    // KarplusStrong(Buffer, SamplesToWrite, SamplesPerSecond, ToneHz, Volume);
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