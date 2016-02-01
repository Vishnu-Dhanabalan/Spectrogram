#include <iostream>
#include <cmath>
#include <Python/python.h>
#include <sndfile.h>

#include "Spectrogram.h"

//======================================================================
//      DEFAULT CONSTRUCTOR
//======================================================================
spectrogram::spectrogram():currPath(""), audioFilename(""), audioInputBuffer(nullptr), audioBufferLength(0), sampleRate(0), windowSize(0), hopSize(0), windowType(""), fftInput(nullptr), fftOutput(nullptr), stftPlan(nullptr){
    std::cout << "Default constructor has been created." << std::endl;
}

//======================================================================
//      DEFAULT DESTRUCTOR
//======================================================================
/*
 NOTE: All the FFTW malloc memory blocks and it's plan will be destroyed here.
       DO NOT free up anywhere else or modify this code.
*/
spectrogram::~spectrogram(){
    std::cout << "Destructor called." << std::endl;
    
    Py_Finalize();
    fftw_destroy_plan(stftPlan);
    fftw_free(fftInput);
    fftw_free(fftOutput);
}

//======================================================================
//      PUBLIC METHODS
//======================================================================
void spectrogram::setupSource(std::string _cp, std::string _afn){
    setCurrPath(_cp);
    setAudioFilename(_afn);
    readAudioIn();
}

/*
    setupSpectrogram(unsigned int _ws, unsigned int _hs, std::string _wt) sets up the 
    spectrogram for its values for window size, hop size and window type. It also sets
    up the FFTW variables like fft input, output and its plan. (current plan: R2C 1D).
*/
void spectrogram::setupSpectrogram(unsigned int _ws, unsigned int _hs, std::string _wt, unsigned int _ffts){
    std::cout << "Setting up parameters for spectrogram." << std::endl;
    
this->setWindowSize(_ws);
    this->setHopSize(_hs);
    this->setWindowType(_wt);
    
    this->setupFFT(_ffts);
}

/*
 Frames the audioInputBuffer into frames of vectors based on the windowSize
 hopSize.
*/
void spectrogram::prepareFrames(){
    unsigned int maxBlockStart = (int) this->audioBufferLength - windowSize;
    unsigned int lastBlockStart = (int) (maxBlockStart - (maxBlockStart % hopSize));
    unsigned int numBlocks = (lastBlockStart)/hopSize + 1;
    
    this->audioFrames.resize(numBlocks);
    // Frame audio samples based on windowSize and hopSize.
    for(unsigned int i = 0; i < numBlocks; i++){
        this->audioFrames[i].resize(this->windowSize);
        for(unsigned int j = 0; j < this->windowSize; j++){
            this->audioFrames[i][j] = this->audioInputBuffer[i * this->hopSize + j];
        }
    }
    
    unsigned int numZeroPads = this->fftSize - this->windowSize;
    
    if(numZeroPads > 0){
        for(unsigned int i = 0; i < numBlocks; i++){
            this->audioFrames[i].resize(this->windowSize + numZeroPads);
            for(unsigned int j = this->windowSize; j < this->windowSize + numZeroPads; j++){
                this->audioFrames[i][j] = 0;
            }
        }
    }
    else{
        // Setting the window size to fft size if numZeroPad is negative.
        this->fftSize = this->windowSize;
    }
    
    // Frame spectrogram output based on fftSize. Note that it's length is half of input because of FFT output symmetry.
    this->spectrogramOutput.resize(numBlocks);
    for(unsigned int i = 0; i < numBlocks; i++){
        this->spectrogramOutput[i].resize((this->fftSize)/2 + 1);
        for(unsigned int j = 0; j < this->fftSize/2 + 1; j++){
            this->spectrogramOutput[i][j] = 0;
        }
    }
}

/*
 Performs windowing on each frame of input based on selection of 
 windowType
*/
void spectrogram::windowFrames(){
    // Iterators.
    unsigned int i, j;
    
    double *window = new double[this->windowSize];
    
    if(windowType == "rectangular" || windowType == "Rectangular"){
        for(int i = 0; i < this->windowSize; i++)
            window[i] = 1.000;
    }
    else if(windowType == "hamming" || windowType == "Hamming"){
        for(int i = 0; i < this->windowSize; i++)
            window[i] =  ( 0.54 - 0.46 * cos (2.0 * M_PI * (double)i/(double)(this->windowSize-1)) );
    }
    else if(windowType == "hann" || windowType == "Hann"){
        for(int i = 0; i < this->windowSize; i++)
            window[i] = (double) (0.5 * (1 - cos(2 * M_PI * (double)i / (double)(this->windowSize-1))));
    }
    else if(this->windowType == "blackmann" || this->windowType == "Blackmann"){
        for(int i = 0; i < this->windowSize; i++)
            window[i] = (double)0.42 - 0.5 * cos(2 *M_PI * i / (this->windowSize-1)) + 0.08 * cos(4 * M_1_PI * i / (this->windowSize-1));
    }
    
    for(i = 0; i < this->audioFrames.size(); i++)
        for(j = 0; j < this->windowSize; j++)
            this->audioFrames[i][j] *= window[j];
    
    for(i = 0; i < 1024; i++)
        this->windowAmplificationFactor += (window[i] * window[i]) ;
        
    free(window);
}

/*
 Peforms STFT on each frame. Also converts amplitude in dB. Saves output
 in the same format, i.e, frames of vectors.
*/
void spectrogram::doSTFT(){
    // Perform Short Time Fourier Transform.
    
    // Copy window by window data from inputFrames to fftInput, followed by executing
    // the stftPlan and copying output into stftOutput vector.
    for(int i = 0; i < this->audioFrames.size(); i++){
        for(int j = 0; j < this->fftSize; j++){
            fftInput[j] = this->audioFrames[i][j];
        }
        fftw_execute(this->stftPlan);
        for(int k = 0; k < this->fftSize/2 + 1; k++){
            this->spectrogramOutput[i][k] = (this->fftOutput[k][0]*this->fftOutput[k][0] + this->fftOutput[k][1]*this->fftOutput[k][1]) / (this->windowAmplificationFactor * this->sampleRate);
        }
    }
    
    this->audioFrames.clear();
    
     // Correcting DC and nyquist points
    for(int i = 0; i < this->spectrogramOutput.size()-1; i++){
        for(int j = 1; j < this->fftSize/2 + 1; j++){
            this->spectrogramOutput[i][j] *= 2;
        }
    }

    for(int i = 0; i < this->spectrogramOutput.size(); i++){
        for(int j = 0; j < this->fftSize/2 + 1; j++){
           this->spectrogramOutput[i][j] =  10 * log10((this->spectrogramOutput[i][j]) +  2.220446049250313e-16);
        }
    }
}

/*
 Prints it in a text file to be able to loaded into Python module for plotting.
 Filename -- SpectrogramOutput.txt.
*/
void spectrogram::print(){
    std::string outTxtFilepath = this->currPath + "SpectrogramOutput.txt";
    const char *path = outTxtFilepath.c_str();
    FILE *out = fopen(path, "w");
    for(int i = 0; i < this->spectrogramOutput.size(); i++){
        for(int j = 0; j < this->fftSize/2; j++){
            fprintf(out, "%f ", this->spectrogramOutput[i][j]);
        }
        fprintf(out, "\n");
    }
    this->spectrogramOutput.clear();
    // Don't move this to destructor. Python plotting won't work if '*out' is not destroyed.
    fclose(out);
}

/*
 Plots the spectrogram by invoking Python module. Please look at
 pythonSetup() method about setting current path constraints.
*/
void spectrogram::plot(){
    pythonSetup();
    PyTuple_SetItem(this->pArgs, 0, this->pModulePath);
    PyTuple_SetItem(this->pArgs, 1, this->pValSampleRate);
    PyTuple_SetItem(this->pArgs, 2, this->pValWindowSize);
    PyTuple_SetItem(this->pArgs, 3, this->pValAudioLength);
    
    // Calling the method.
    PyObject_CallObject(pFunc, pArgs);
}

//======================================================================
//      PRIVATE METHODS
//======================================================================
void spectrogram::setCurrPath(std::string _cp){
    this->currPath = _cp;
}

void spectrogram::setAudioFilename(std::string _afn){
    this->audioFilename = _afn;
}

void spectrogram::setWindowSize(unsigned int _ws){
    this->windowSize = _ws;
    this->fftSize = this->windowSize;
}

void spectrogram::setHopSize(unsigned int _hs){
    this->hopSize = _hs;
}

void spectrogram::setWindowType(std::string _wt){
    this->windowType = _wt;
}

void spectrogram::setupFFT(unsigned int _ffts){
    this->fftSize = _ffts;
    this->fftInput = (double*)fftw_malloc(sizeof(double)*this->fftSize);
    this->fftOutput = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*this->fftSize);
    this->stftPlan = fftw_plan_dft_r2c_1d(this->fftSize, this->fftInput, this->fftOutput, FFTW_ESTIMATE);
}

void spectrogram::readAudioIn(){
    SNDFILE *sf;
    SF_INFO info;
    memset(&info, 0, sizeof(info));
    
    long long numSamplesPerChannel, numTotalSamples;
    unsigned int numChannels;
    float readSamples;

    // Reading the .wav file.
    std::string fullPath = this->currPath + this->audioFilename;    // Current path/audiofilename.wav
    const char* path = fullPath.c_str();
    sf = sf_open(path, SFM_READ, &info);
    if(sf == NULL){
        std::cout << "Failed to open the file.\n";
        exit(-1);
    }
    
    this->sampleRate = info.samplerate;
    
    numSamplesPerChannel = info.frames;
    numChannels = info.channels;
    numTotalSamples = numSamplesPerChannel * numChannels;     // Support only mono. Don't wanna make the time vector twice the actual length.
    this->audioBufferLength = numTotalSamples;
    this->audioInputBuffer = new float[numTotalSamples];
    readSamples = sf_read_float(sf, this->audioInputBuffer, numTotalSamples);
    
    sf_close(sf);
}

void spectrogram::pythonSetup(){
    Py_Initialize();
    PyRun_SimpleString("import sys");
    
//    std::string sysAppend = "sys.path.append(\'"+this->currPath+"')";
    /** The above commented, 'sysAppend' variable should be passed into PyRun_SimpleString as an argument
        but I've got no clue why it's not working. Please change this path to this->currPath. If you can't
        make it work, please hard code your path (where your python file is available) like below. 
     **/
    PyRun_SimpleString("sys.path.append(\'/Users/Vishnu/Google Drive/programming tutorials/c++ practice/XCode projects/Spec_CPP_Python_Hybrid/Spec_CPP_Python_Hybrid/\')");

    this->pName = PyString_FromString("plotPy");
    
    if(!this->pName){
        std::cout << "Python script is not found. Please check the name and try again.\n";
    }
    
    this->pModule = PyImport_Import(this->pName);
    if(this->pModule == nullptr){
        PyErr_Print();
    }
    
    this->pDict = PyModule_GetDict(this->pModule);
    this->pFunc = PyDict_GetItemString(this->pDict, "plotSpectrogram");
    this->pArgs = PyTuple_New(4);
    
    const char *modulePath = (this->currPath+"SpectrogramOutput.txt").c_str();
    this->pModulePath = PyString_FromString(modulePath);
    this->pValSampleRate = PyInt_FromLong(this->sampleRate);
    this->pValWindowSize = PyInt_FromLong(this->windowSize);
    this->pValAudioLength = PyInt_FromLong(this->audioBufferLength);
}

// End of class member definitions.