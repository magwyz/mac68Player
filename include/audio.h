
#ifndef __AUDIO__
#define __AUDIO__

#include <Sound.h>


typedef struct SndPacket
{
    SndCommand cmd;
    SoundHeader *soundHeader;
    char *buf;
} SndPacket;


void initSndPacket(SndPacket *sndPacket, short nbAudioBytesToRead);
void releaseSndPacket(SndPacket *sndPacket);


#endif
