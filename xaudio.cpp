
#include <xaudio2.h>

#include "wave.h"

struct SoundContext {
    IXAudio2 *XAudioContext;
    IXAudio2MasteringVoice *MasterVoice;
    IXAudio2SourceVoice *SourceVoice;
};

int InitXAudio(HWND hWnd, SoundContext *Context) {
    // TODO: Improve this
    assert(Context != NULL);

    // FFS!!!
    CoInitialize(NULL);

    IXAudio2 *XAudioContext;
    IXAudio2MasteringVoice *MasterVoice;
    IXAudio2SourceVoice *SourceVoice;

    if(FAILED(XAudio2Create(&XAudioContext, 0, XAUDIO2_DEFAULT_PROCESSOR))) {
        return -1;
    }

    HRESULT hr;
    if (FAILED(hr = XAudioContext->CreateMasteringVoice(&MasterVoice, Channels, SamplesPerSecond, 0, NULL, NULL))) {
        return -1;
    }

    WAVEFORMATEX WaveFormat = {};
    WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
    WaveFormat.nChannels = Channels;
    WaveFormat.nSamplesPerSec = SamplesPerSecond;
    WaveFormat.nAvgBytesPerSec = BytesPerSecond;
    WaveFormat.nBlockAlign = Channels * BytesPerSample;
    WaveFormat.wBitsPerSample = BytesPerSample * 8;

    if(FAILED(XAudioContext->CreateSourceVoice(&SourceVoice, &WaveFormat, 0))) {
        return -1;
    }

    *Context = {
        XAudioContext,
        MasterVoice,
        SourceVoice
    };

    return 0;
}

int FillXAudioBuffer(SoundContext Context, WaveFn Wave, float ToneHz, int Volume) {
    // Need to save a reference to this somewhere, so I can change or free it.
    int16_t *Buffer = (int16_t *)VirtualAlloc(NULL, BufferSize, MEM_COMMIT, PAGE_READWRITE);

    int SamplesToWrite = BufferSize / (BytesPerSample * Channels);

    FillBuffer(Buffer, SamplesToWrite, Wave, ToneHz, Volume);

    XAUDIO2_BUFFER XAudioBuffer = {};
    XAudioBuffer.Flags = XAUDIO2_END_OF_STREAM;
    XAudioBuffer.AudioBytes = BufferSize;
    XAudioBuffer.pAudioData = (BYTE *) Buffer;
    XAudioBuffer.PlayBegin = 0;
    XAudioBuffer.PlayLength = 0;
    XAudioBuffer.LoopBegin = 0;
    XAudioBuffer.LoopLength = 0;
    XAudioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    XAudioBuffer.pContext = NULL;

    if(FAILED(Context.SourceVoice->SubmitSourceBuffer(&XAudioBuffer))) {
        return -1;
    }

    return 0;
}

void XAudioPlay(SoundContext Context) {
    Context.SourceVoice->Start(0);
}