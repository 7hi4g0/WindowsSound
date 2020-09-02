#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <dsound.h>


LPDIRECTSOUND DSContext;
DSBUFFERDESC PrimaryBufferDesc;
LPDIRECTSOUNDBUFFER PrimaryBuffer;
WAVEFORMATEX WaveFormat;
DSBUFFERDESC SecondaryBufferDesc;
LPDIRECTSOUNDBUFFER SecondaryBuffer;

LRESULT CALLBACK MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitDSound(HWND hWnd);

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
    ShowWindow(hWnd, nCmdShow);

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

    PrimaryBufferDesc = {};
    PrimaryBufferDesc.dwSize = sizeof(DSBUFFERDESC);
    PrimaryBufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
    PrimaryBufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;

    if(FAILED(DSContext->CreateSoundBuffer(&PrimaryBufferDesc, &PrimaryBuffer, NULL))) {
        PostQuitMessage(-1);
        return;
    }

    int SamplesPerSecond = 48000;
    int Channels = 2;
    int BytesPerSample = 2;
    int BufferLength = 2;
    int BytesPerSecond = Channels * BytesPerSample * SamplesPerSecond;
    int BufferSize = BufferLength * BytesPerSecond;

    WaveFormat = {};
    WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
    WaveFormat.nChannels = Channels;
    WaveFormat.nSamplesPerSec = SamplesPerSecond;
    WaveFormat.nAvgBytesPerSec = BytesPerSecond;
    WaveFormat.nBlockAlign = Channels * BytesPerSample;
    WaveFormat.wBitsPerSample = BytesPerSample * 8;

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

    if (FAILED(DSContext->SetCooperativeLevel(hWnd, DSSCL_PRIORITY))) {
        PostQuitMessage(-1);
        return;
    }
}

