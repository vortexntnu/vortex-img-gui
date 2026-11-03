#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <stdlib.h>   // calloc, free
#include <thread>     // std::thread
#include <mutex>      // std::mutex
#include <atomic>     // std::atomic
#include <utility>    // std::swap
#include <string.h>   // memcpy
#include "../lib/typedef.h"


// Tre faste bildebuffere som gjenbrukes. Kameraet skriver, GUI-et leser den nyeste.
struct frame_buffer {
    u8* Pixels[3];
    s32 Width, Height;
    s32 WriteIndex = 0;        // Kameraet skriver her (bare kamera-tråden bruker denne)
    s32 ReadyIndex = 1;        // Nyeste ferdige frame
    s32 ReadIndex  = 2;        // GUI-et viser denne (bare GUI-tråden bruker denne)
    bool NewFrame  = false;
    std::mutex Lock;
};

void FrameBufferInit(frame_buffer* FB, s32 Width, s32 Height) {
    FB->Width = Width;
    FB->Height = Height;
    for (s32 i = 0; i < 3; i++)
        FB->Pixels[i] = (u8*)calloc(Width * Height * 4, 1);   // RGBA = 4 bytes per piksel
}

void FrameBufferFree(frame_buffer* FB) {
    for (s32 i = 0; i < 3; i++)
        free(FB->Pixels[i]);
}

// Kalles av kamera-tråden når Pixels[WriteIndex] er ferdig skrevet
void PublishFrame(frame_buffer* FB) {
    std::lock_guard<std::mutex> L(FB->Lock);
    std::swap(FB->WriteIndex, FB->ReadyIndex);
    FB->NewFrame = true;
}

// Kalles av GUI-tråden. Returnerer true hvis *Out er en ny frame.
bool GetLatestFrame(frame_buffer* FB, u8** Out) {
    std::lock_guard<std::mutex> L(FB->Lock);
    bool IsNew = FB->NewFrame;
    if (IsNew) {
        std::swap(FB->ReadIndex, FB->ReadyIndex);
        FB->NewFrame = false;
    }
    *Out = FB->Pixels[FB->ReadIndex];
    return IsNew;
}

// Laster et bilde, skalerer det til Width x Height og returnerer RGBA-piksler.
// Returnerer nullptr hvis fila ikke finnes.
u8* LoadImageRGBA(const char* Path, s32 Width, s32 Height) {
    SDL_Surface* Loaded = SDL_LoadSurface(Path);
    if (Loaded == nullptr)
        return nullptr;

    SDL_Surface* Scaled = SDL_ScaleSurface(Loaded, Width, Height, SDL_SCALEMODE_LINEAR);
    SDL_Surface* Rgba   = SDL_ConvertSurface(Scaled, SDL_PIXELFORMAT_RGBA32);

    u8* Pixels = (u8*)malloc(Width * Height * 4);
    for (s32 y = 0; y < Height; y++)                       // Kopierer rad for rad
        memcpy(Pixels + y * Width * 4, (u8*)Rgba->pixels + y * Rgba->pitch, Width * 4);

    SDL_DestroySurface(Rgba);
    SDL_DestroySurface(Scaled);
    SDL_DestroySurface(Loaded);
    return Pixels;
}
void TestCameraThread(frame_buffer* FB, std::atomic<bool>* Running, std::atomic<bool>* NextImage) {
    // De tre bildene som skal byttes på
    u8* Images[3];
    Images[0] = LoadImageRGBA("asset/bilde1.png", FB->Width, FB->Height);
    Images[1] = LoadImageRGBA("asset/bilde2.png", FB->Width, FB->Height);
    Images[2] = LoadImageRGBA("asset/bilde3.png", FB->Width, FB->Height);

    s32  Current     = 0;
    bool ShowCurrent = true;                              // Viser første bilde med en gang

    while (*Running) {
        if (NextImage->exchange(false)) {                 // Var W trykket? Les og nullstill i ett steg
            // Gå til neste bilde, og hopp over bilder som ikke ble lastet
            for (s32 Tries = 0; Tries < 3; Tries++) {
                Current = (Current + 1) % 3;              // 0 → 1 → 2 → 0 → 1 → ...
                if (Images[Current] != nullptr)
                    break;
            }
            ShowCurrent = true;
        }
        if (ShowCurrent && Images[Current] != nullptr) {
            memcpy(FB->Pixels[FB->WriteIndex], Images[Current], FB->Width * FB->Height * 4);
            PublishFrame(FB);
        }
        ShowCurrent = false;                              // Ikke send samme bilde på nytt
        SDL_Delay(10);                                    // Liten pause så tråden ikke bruker 100 % CPU
    }

    for (s32 i = 0; i < 3; i++)
        free(Images[i]);                                  // free(nullptr) er trygt
}

