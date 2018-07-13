## Create Spectrogram of audio samples using C++ and Python

This spectrogram is designed using C++ for computing STFT and power and using Python 
for plotting the data. 

I created this project using XCode. If you do not have XCode, please go to the project folder
to find source files. You must include library files for FFTW, libsndfile, Python framework on 
your own.

Step 1:
In the main file, change the path and file name. Only .wav files are supported with libsndfile. 

Step 2:
Define spectrogram properties like windowSize, windowType, hopSize and fftSize depending upon your purpose.

Step 3:
In the spectrogram.cpp file, go to pythonSetup() definition (It is a private method and you can find it in the end).
Read the comment about the Python file path. 

Step 4:
Hit run.

### Libraries used

C++:
libsndfile - For reading wav files.
FFTW - For computing STFT.

Python:
matplotlib - For plotting.
numpy - For reading spectrogram data from text file as ndarray.

### Version history

v1.0 - Created Spectrogram with C++ and Python. This project uses C++ for computing STFT and 
uses Python script for Plotting. Using C-Python API, C++ code calls Python script for 
plotting.

### In future...

Improving Python and C++ integration in such a way that Python module does not need to read the spectrogram output from the disk, instead convert the C++ vector into Python NumPy ndarrays.
