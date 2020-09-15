#!/bin/sh

WIDTH=640
HEIGHT=480
FPS=10

ffmpeg -y -i $1 -filter:v fps=fps=$FPS -filter:v scale=$WIDTH:$HEIGHT video_resampled.mp4
ffmpeg -y -i $1 -vn -ar 11000 -ac 1 -acodec pcm_u8 audio.wav
python3 convertVideo.py video_resampled.mp4 audio.wav

# Generate an image for the mini vMac emulator
genisoimage -hfs -hfs-unlock -probe -V Video -o video.img video.dat
