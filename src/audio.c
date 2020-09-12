
#include <stdlib.h>

#include <Sound.h>

#include <audio.h>


void initSndPacket(SndPacket *sndPacket, short nbAudioBytesToRead)
{
    char *snd = malloc(sizeof(SoundHeader) + nbAudioBytesToRead);
    sndPacket->soundHeader = (SoundHeader *)snd;

    sndPacket->soundHeader->samplePtr = nil;
    sndPacket->soundHeader->length = nbAudioBytesToRead;
    sndPacket->soundHeader->sampleRate = rate11khz;
    sndPacket->soundHeader->loopStart = 0;
    sndPacket->soundHeader->loopEnd = 0;
    sndPacket->soundHeader->encode = stdSH;
    sndPacket->soundHeader->baseFrequency = kMiddleC;

    sndPacket->cmd.cmd = bufferCmd;
    sndPacket->cmd.param1 = 0;
    sndPacket->cmd.param2 = (long)snd;

    sndPacket->buf = (char *)&(sndPacket->soundHeader->sampleArea);
}


void releaseSndPacket(SndPacket *sndPacket)
{
    free(sndPacket->soundHeader);
}
