# This Python file uses the following encoding: utf-8

import sys

import numpy as np
import cv2
import wave

WIDTH = 320*2
HEIGHT = 240*2
FPS = 10


def getBinaryImage(frame):
    ret = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    ret = cv2.resize(ret, (WIDTH, HEIGHT), interpolation = cv2.INTER_AREA)
    ret = cv2.Canny(ret,50,200)
    ret = 255 - ret
    return ret

def encodeImage(frame):
    ret = bytearray()

    bitId = 0
    curByte = 0
    for id in np.ndindex(frame.shape):
        if frame[id] == 0:
            curByte |= 1 << (7 - bitId)

        bitId += 1
        if bitId >= 8:
            ret.append(curByte)
            bitId = 0
            curByte = 0

    return ret



if __name__ == "__main__":

    cap = cv2.VideoCapture(sys.argv[1])
    hasAudio = False

    if len(sys.argv) == 3:
        audio = wave.open(sys.argv[2], "rb")
        frameRate = audio.getframerate()
        hasAudio = True

    fd = open("video.dat", "wb")

    while cap.isOpened():
        ret, frame = cap.read()
        if frame is None:
            break

        frame = getBinaryImage(frame)
        frameData = encodeImage(frame)
        fd.write(frameData)

        if hasAudio:
            nbSamples = frameRate // FPS
            audioData = audio.readframes(nbSamples)
            fd.write(audioData)

        cv2.imshow('frame', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    fd.close()
    cap.release()
    cv2.destroyAllWindows()
