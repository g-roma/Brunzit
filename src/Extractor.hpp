#pragma once

#include <data/FluidIndex.hpp>
#include <data/FluidMemory.hpp>
#include <data/TensorTypes.hpp>

#include <algorithms/public/Loudness.hpp>
#include <algorithms/public/MultiStats.hpp>
#include <algorithms/public/STFT.hpp>
#include <algorithms/public/SpectralShape.hpp>
#include <algorithms/public/MelBands.hpp>
#include <algorithms/public/DCT.hpp>

#include "Sound.hpp"

using namespace fluid;
using namespace fluid::algorithm;

class extractor{
    public:

        void extract(Sound& snd);
        RealVector computeStats(fluid::RealMatrixView matrix);
        void normalizeVector(fluid::RealVector& vec);
};

fluid::RealVector extractor::computeStats(fluid::RealMatrixView matrix)
{
    fluid::algorithm::MultiStats stats;
    fluid::index dim = matrix.cols();
    fluid::RealMatrix tmp(dim, 7);
    fluid::RealVector result(dim * 2);
    stats.process(matrix.transpose(), tmp);
    for (int j = 0; j < dim; j++)
    {
        result(j*2) = tmp(j, 0);
        result(j*2 + 1) = tmp(j, 1);
    }
    return result;
}

void extractor::extract(Sound& snd){
    using fluid::index;
    FluidTensor<float, 1> tensor (snd.mBuffer->getChannel(0), snd.mBuffer->getNumFrames());
    RealVector in(tensor);
    
    const fluid::index nBins = 513;
    const fluid::index fftSize = 2 * (nBins - 1);
    const fluid::index hopSize = 512;
    const fluid::index windowSize = 1024;
    const fluid::index halfWindow = windowSize / 2;
    const fluid::index nBands = 40;
    const fluid::index nCoefs = 10;
    
    
    STFT          stft{windowSize, fftSize, hopSize};
    SpectralShape shape(FluidDefaultAllocator());
    Loudness      loudness{windowSize};
    MelBands      bands{nBands, fftSize};
    DCT           dct{nBands, nCoefs};
    loudness.init(windowSize, snd.mSampleRate);
    bands.init(20, 5000, nBands, nBins, snd.mSampleRate, windowSize);
    dct.init(nBands, nCoefs);
    RealVector padded(in.size() + windowSize + hopSize);
    size_t nFrames = floor((padded.size() - windowSize) / hopSize);
    snd.loudnessVec = RealVector(nFrames);
    snd.mfccStats = RealVector(nFrames);
    RealMatrix mfccMat(nFrames, nCoefs);
    std::fill(padded.begin(), padded.end(), 0);
    padded(fluid::Slice(halfWindow, in.size())) <<= in;
    for (int i = 0; i < nFrames; i++)
    {
        ComplexVector  frame(nBins);
        RealVector     magnitude(nBins);
        RealVector     mels(nBands);
        RealVector     mfccs(nCoefs);
        RealVector     loudnessDesc(2);
        RealVectorView window = padded(fluid::Slice(i * hopSize, windowSize));
        stft.processFrame(window, frame);
        stft.magnitude(frame, magnitude);
        bands.processFrame(magnitude, mels, false, false, true, FluidDefaultAllocator());
        dct.processFrame(mels, mfccs);
        mfccMat.row(i) <<= mfccs;
        loudness.processFrame(window, loudnessDesc, false, false);
        snd.loudnessVec(i) = loudnessDesc(0);
    }
    
    snd.mfccStats = computeStats(mfccMat);
    snd.minLoudness = *std::min_element(
                    snd.loudnessVec.begin(),
                    snd.loudnessVec.end()
    );
    snd.maxLoudness = *std::max_element(
                    snd.loudnessVec.begin(),
                    snd.loudnessVec.end()
    );
}
