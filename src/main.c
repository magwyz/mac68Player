#include <Quickdraw.h>
#include <MacMemory.h>
#include <Sound.h>
#include <Events.h>
#include <Fonts.h>
#include <NumberFormatting.h>
#include <Resources.h>
#include <ToolUtils.h>
#include <OSUtils.h>

// For the Vsync interrupt
#include <Retrace.h>

// For the input file selection dialog
#include <StandardFile.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <audio.h>


// Consts

#define kQKeyMap 11
#define kFrameWidth 320*2
#define kFrameHeight 240*2
#define kFps 10
#define kSampleRate 11000
#define kNbAudioBytesToRead (kSampleRate / kFps)
#define bVBLInterval 6


// Global video vars

WindowPtr win;
Rect frameRect;
Rect dstRect;
BitMap frameBits;
enum dMode {READY, TO_DISPLAY} drawingMode = READY;


// Input method.

// Returns true if it keeps going.
bool read_input() {
    KeyMap theKeyMap;
    GetKeys(theKeyMap);
    if (BitTst(&theKeyMap, kQKeyMap)) {
        // Quit
        return false;
    }

    return true;
}


// VBL interrupt
// It is called at 60.15 / bVBLInterval Hz.

VBLTask vblTask = {0};

void vblInterrupt()
{
    if (drawingMode == TO_DISPLAY)
    {
        CopyBits(&frameBits, &win->portBits, &frameRect, &dstRect, srcCopy, nil);
        drawingMode = READY;
    }

    vblTask.vblCount = bVBLInterval;
}


// Main function

int main()
{
    OSErr err;
    int ret = 0;

    InitGraf(&qd.thePort);
    InitFonts();
    InitWindows();
    InitMenus();
    FlushEvents(everyEvent, 0);

    // Make a window
    Rect r = qd.screenBits.bounds;
    SetRect(&r, r.left + 5, r.top + 45, r.left + 5 + kFrameWidth, r.top + 45 + kFrameHeight);
    win = NewWindow(NULL, &r, "\pmac68Player", true, 0, (WindowPtr)-1, false, 0);
    if (win == nil)
    {
        printf("Error creating the window: %d\n", err);
        ret = 1;
        goto error;
    }
    SetPort(win);
    r = win->portRect;
    EraseRect(&r);
    SetRect(&dstRect, r.left, r.top, r.left + kFrameWidth, r.top + kFrameHeight);

    // Change cursor to a regular pointer to indicate we're done.
    InitCursor();

    // Go back to drawing on the window.
    SetPort(win);

    // Init the offscreen buffer
    GrafPtr framePtr = (GrafPtr)NewPtr(sizeof(GrafPort));
    if (framePtr == NULL)
    {
        printf("Error allocating the offscreen frame buffer: %d\n", err);
        ret = 1;
        goto error;
    }
    OpenPort(framePtr);

    SetRect(&frameRect, 0, 0, kFrameWidth, kFrameHeight);
    short frameRowBytes = ((frameRect.right - frameRect.left + 15) / 16) * 2;

    frameBits.rowBytes = frameRowBytes;
    frameBits.bounds = frameRect;
    frameBits.baseAddr = NewPtr((long)frameBits.rowBytes *
            (frameRect.bottom - frameRect.top));
    SetPortBits(&frameBits);
    ClipRect(&frameRect);
    EraseRect(&frameRect);

    long nbVideoBytesToRead = kFrameWidth * kFrameHeight / 8;

    // VBLTask initialization
    vblTask.vblAddr = (VBLProcPtr) vblInterrupt;
    vblTask.vblCount = bVBLInterval; /* Frequency of task, in ticks */
    vblTask.qType = vType;       /* qElement is a VBL task */
    vblTask.vblPhase = 0;
    err = VInstall((QElemPtr)&vblTask);
    if (err != noErr)
    {
        printf("Error during VBL setup: %d\n", err);
        vblTask.vblAddr = 0;
        ret = 1;
        goto error;
    }

    // Input video file opening
    StandardFileReply fileReply;
    StandardGetFile(NULL, -1, NULL, &fileReply);

    short fileRefNum = 0;
    err = FSpOpenDF(&fileReply.sfFile, fsRdPerm, &fileRefNum);
    if (err != noErr)
    {
        printf("Error opening the video file: %d\n", err);
        ret = 1;
        goto error;
    }

    // Init sound
    SndChannelPtr chan1 = NULL;
    SndPacket sndPacket[2] = {0};

    err = SndNewChannel(&chan1, sampledSynth, initMono | initNoInterp, nil);
    if (err != noErr)
    {
        printf("Error creating the sound channel: %d\n", err);
        ret = 1;
        goto error;
    }

    // Setup sound packets
    for (unsigned i = 0; i < 2; ++i)
        initSndPacket(sndPacket + i, kNbAudioBytesToRead);

    // Main loop
    int sndPacketId = 0;
    while(read_input()) {

        if (drawingMode == READY)
        {
            // Read the raw image
            long byteCount = nbVideoBytesToRead;
            FSRead(fileRefNum, &byteCount, frameBits.baseAddr);

            // Read the raw sound
            byteCount = kNbAudioBytesToRead;
            FSRead(fileRefNum, &byteCount, sndPacket[sndPacketId].buf);

            // Tell the VBL interrupt we are ready to display
            drawingMode = TO_DISPLAY;

            // Play the sound
            OSErr err = SndDoCommand(chan1, &sndPacket[sndPacketId].cmd, FALSE);
            if (err != noErr)
            {
                printf("Error playing the sound: %d\n", err);
                ret = 1;
                goto error;
            }

            sndPacketId++;
            if (sndPacketId > 1)
                sndPacketId = 0;
        }

    }

error:
    // Release everything

    FlushEvents(everyEvent, -1);

    for (unsigned i = 0; i < 2; ++i)
        releaseSndPacket(sndPacket + i);

    if (chan1 != NULL)
        SndDisposeChannel(chan1, TRUE);

    if (fileRefNum != 0)
        FSClose(fileRefNum);

    if (vblTask.vblAddr != 0)
        VRemove((QElemPtr)&vblTask);

    if (framePtr != NULL)
    {
        ClosePort(framePtr);
        DisposePtr((Ptr)framePtr);
    }

    // On error, wait user input before quiting
    // so it is possible to read the error message.
    if (ret == 1)
        getchar();

    return ret;
}
