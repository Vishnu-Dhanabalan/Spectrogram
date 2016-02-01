#ifndef SPECTROGRAM_H_
#define SPECTROGRAM_H_
////////////////////////////////////////////////////
//  Project: Spectrogram in C++ and Python.
//  File: spectrogram.h
//  Version: 1.0
//
//  Author: Vishnu Dhanabalan.
//  Contact: vishnudhanabalan@outlook.com
////////////////////////////////////////////////////

/*
 Spectrogram.h -- Header file for spectrogram class declration.
 
 Info: 
 Contains class declration for spectrogram.
 
 Version history:
 v1.0  -- Created spectrogram.h
*/

#include <iostream>
#include <fftw3.h>
#include <vector>
#include <Python/python.h>

class spectrogram{
private:
    /********* Source path and other information ************/
    std::string currPath;
    std::string audioFilename;
    float *audioInputBuffer;
    unsigned long audioBufferLength;
    unsigned int sampleRate;

    /********* Vector variables ************/
    std::vector<std::vector<double>> audioFrames;

    /********* Windowing variables *********/
    unsigned int windowSize;
    unsigned int hopSize;
    std::string windowType;
    double windowAmplificationFactor;
    
    /********* FFTW variables *********/
    unsigned int fftSize;
    double *fftInput;
    fftw_complex *fftOutput;
    fftw_plan stftPlan;
    std::vector<std::vector<double>> spectrogramOutput;
    std::string textFilename;
    
    /********* Private setters *********/
    void setCurrPath(std::string _cp);
    void setAudioFilename(std::string _afn);
    void readAudioIn();
    void setWindowSize(unsigned int _ws);
    void setHopSize(unsigned int _hs);
    void setWindowType(std::string _wt);
    void setupFFT(unsigned int _ffts);
    
    /********* Python embedding variables *********/
    PyObject *pName, *pModule, *pDict, *pFunc, *pArgs;
    PyObject *pModulePath, *pValSampleRate, PyObject, *pValWindowSize, *pValAudioLength;
    void pythonSetup();
    
public:
    /********* Constructor & Destructor *********/
    spectrogram();
    ~spectrogram();
    
    /********* Public setters *********/
    void setupSource(std::string _cp, std::string _afn);
    void setupSpectrogram(unsigned int _ws, unsigned int _hs, std::string _wt, unsigned int _ffts);
    
    /********* Input data structure modifiers *********/
    void prepareFrames();
    void windowFrames();
    
    /********* FFT methods *********/
    void doSTFT();
    
    /********** Printing text file methods ***********/
    void print();
    
    /********** Plot methods ***********/
    void plot();
};

#endif /* SPECTROGRAM_H_ */
