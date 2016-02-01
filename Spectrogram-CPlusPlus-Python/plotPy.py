#######################################################
#   Project: Spectrogram in C++ and Python.
#   File: plotPy.py
#   Version: 1.0
#
#   Author: Vishnu Dhanabalan.
#   Contact: vishnudhanabalan@outlook.com
#######################################################

# plotPy.py -- Contains plotting implementation for spectrogram project.
# Uses numpy and matplotlib for reading text file as ndarrays and plotting respectively.


import os
import numpy as np
from matplotlib import pyplot as plt

def plotSpectrogram(filePath = " ", sampleRate=44100, windowSize=1024, audioLength=100000):
    
    # Loads the spectral data from the path provided in filePath.
    spectralData = np.loadtxt(filePath, dtype=float)
    timebins, frequencyBins = np.shape(spectralData)
    scale = np.linspace(0, 1, frequencyBins)
    scale *= (frequencyBins - 1) / max(scale)
    scale = np.unique(np.round(scale))
    
    # Computing new frequency bins based on sample rate.
    allFreqBins = np.abs(np.fft.fftfreq(frequencyBins * 2, 1. / sampleRate)[:frequencyBins + 1])
    newFreqBins = []
    for i in range(0, len(scale)):
        if i == len(scale) - 1:
            newFreqBins += [np.mean(allFreqBins[scale[i]:])]
        else:
            newFreqBins += [np.mean(allFreqBins[scale[i]:scale[i + 1]])]

    plt.figure(figsize=(15, 7.5))
    plt.imshow(np.transpose(spectralData), origin="lower", aspect="auto", cmap="jet")
    plt.colorbar()
    
    # Labelling the X and Y axis.
    plt.xlabel("time in seconds")
    plt.ylabel("frequency in Hertz")
    plt.xlim([0, timebins - 1])
    plt.ylim([0, frequencyBins])

    # linearly locates time bins in X axis.
    xLocation = np.float32(np.linspace(0, timebins - 1, 5))
    plt.xticks(xLocation,["%.02f" % l for l in ((xLocation * audioLength / timebins) + (0.5 * windowSize)) / sampleRate])
    # linearly locates freq bins in Y axis.
    yLocation = np.int16(np.round(np.linspace(0, frequencyBins - 1, 10)))
    plt.yticks(yLocation, ["%.02f" % newFreqBins[i] for i in yLocation])
    
    plt.show()

