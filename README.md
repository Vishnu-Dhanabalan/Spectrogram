*** Create Spectrogram of audio samples using C++ and Python ***

*** COMPATIBILITY WARNING *** 
I created this project using XCode. If you do not have XCode, please go to the project folder
to find source files. You must include library files for FFTW, libsndfile, Python framework on 
your own.

*** STEPS TO CREATE SPECTROGRAM ***
Step 1:
In the main file, change the path and file name. Only .wav files are supported with libsndfile. 

Step 2:
Define spectrogram properties like windowSize, windowType, hopSize and fftSize depending upon your purpose.

Step 3:
In the spectrogram.cpp file, go to pythonSetup() definition (It is a private method and you can find it in the end).
Read the comment about the Python file path. 

Step 4:
Hit run.

*** Possible future changes ***
Python script reads the output text file which is created by C++ side, to get spectrogram output. I will try to 
pass it directly into Python script which possibly will boost up some speed. 
