#pragma once

#include <cstdlib>
#include <ctime>
#include <iostream>

#include <data/FluidIndex.hpp>
#include <data/FluidMemory.hpp>
#include <data/TensorTypes.hpp>

#include <data/FluidDataSet.hpp>
#include <algorithms/public/UMAP.hpp>
#include <algorithms/public/Grid.hpp>
#include <algorithms/public/KMeans.hpp>

#include "Extractor.hpp"
#include "Sound.hpp"


using namespace cinder;
using namespace std;
using namespace fluid;
using cinder::audio::BufferRef;
using cinder::audio::SourceFileRef;

class Corpus{
    public:
        Corpus();

        void addSound(Sound snd);
        void draw();
        void project();
        void findBounds();
        int getRandom(int cluster);
        void makeWaves();
        bool empty();
        void makeEnvelope(size_t sampleRate, size_t grainSamples);
        void slice(audio::BufferRef b, size_t sampleRate);
    
        int mEngine{-1};
        Channel32f mChannel;
        gl::Texture2dRef mTexture;
        audio::Buffer envelope;
        map<pair<int, int>,int> mPositionsMap;
        int mMinX, mMaxX, mMinY, mMaxY;
    
        float SEGMENT_DUR = 0.2;
        float ENV_DUR{0.1};
        int ENV_SIZE;
        float GRAIN_SIZE;
        std::vector<Sound> mSounds;
    
private:
    
    int lastIndex{0};
    algorithm::UMAP mUmap;
    algorithm::Grid mGrid;
    FluidDataSet<std::string, double, 1> mDataset;
    FluidDataSet<std::string, double, 1> mProjectionDS;
    FluidDataSet<std::string, double, 1> mGridDS;
    RealMatrix mPositions;
    
    
    double mMaxVal, mMinVal;
    extractor mExtractor;
    
    
    
    
    
    
    
};

Corpus::Corpus(){
    mDataset = FluidDataSet<std::string, double, 1>(20);
    mMaxVal = 0;
    mMinVal = 0;
}

bool Corpus::empty(){
    return lastIndex == 0;
}

void Corpus::slice(audio::BufferRef src, size_t sampleRate){
    size_t numSamples = src->getNumFrames();
    size_t offset = 0;
    size_t grainSamples = SEGMENT_DUR * sampleRate;
    GRAIN_SIZE = grainSamples;
    makeEnvelope(sampleRate, grainSamples);
    
    mMaxVal = 0;
    mMinVal = 0;
    
    while ((offset + grainSamples) < numSamples){
        audio::Buffer b = audio::Buffer(grainSamples, 1);
        b.copyOffset(*src.get(), grainSamples, 0, offset);
        offset += grainSamples;
        audio::BufferRef dest = std::make_shared<audio::Buffer>(b);
        Sound snd(dest, sampleRate);
        mExtractor.extract(snd);
        addSound(snd);
        mMaxVal += snd.maxLoudness;
        mMinVal += snd.minLoudness;
    }
    mMaxVal /= mSounds.size();
    mMinVal /= mSounds.size();
    
    double range = mMaxVal - mMinVal;
    mMaxVal += (range * 0.75);
    mMinVal -= (range * 0.75);
}


void Corpus::addSound(Sound snd){
    mDataset.add(std::to_string(lastIndex++), snd.mfccStats);
    mSounds.push_back(snd);
}

void Corpus::draw(){
    if (mEngine == 0){
        gl::draw( mTexture);
    }
    else if (mEngine == 1){
        for (auto& snd:mSounds) snd.draw();
    }
}

void Corpus::makeWaves(){
    int width = app::getWindowWidth() / (mMaxX + 1);
    int height = app::getWindowHeight() / (mMaxY + 1);
    for(int i = 0; i < mSounds.size(); i++){
        int x = mPositions(i, 0) * width;
        int y = mPositions(i, 1) * height;
        mSounds[i].makeWave(x, y, width, height, mMinVal, mMaxVal);
    }
}

void Corpus::findBounds(){
    mMaxX = INT_MIN;
    mMinX = INT_MAX;
    mMaxY = INT_MIN;
    mMinY = INT_MAX;

    for (int i = 0; i < mPositions.rows(); i++){
        if(mPositions(i, 0) < mMinX) mMinX = mPositions(i, 0);
        if(mPositions(i, 0) > mMaxX) mMaxX = mPositions(i, 0);
        if(mPositions(i, 1) < mMinY) mMinY = mPositions(i, 1);
        if(mPositions(i, 1) > mMaxY) mMaxY = mPositions(i, 1);
    }
}

void Corpus::project(){
    mProjectionDS = mUmap.train(mDataset);
    mGridDS = mGrid.process(mProjectionDS);
    mPositions = RealMatrix(mGridDS.getData());
    findBounds();
    for (int i = 0; i < mPositions.rows(); i++){
        pair<int,int> pos = make_pair<int,int>(mPositions(i, 0),mPositions(i, 1));
        mPositionsMap.insert(pair<pair<int,int>,int>(pos,i));
    }
}

void Corpus:: makeEnvelope(size_t sampleRate, size_t grainSamples){
    ENV_SIZE = static_cast<size_t>(ENV_DUR * sampleRate);
    envelope = audio::Buffer(ENV_SIZE, 1);
    float incr = M_PI / float(ENV_SIZE);
    float angle = 0;
    float* data = envelope.getData();
    for(int i = 0; i < ENV_SIZE; i ++){
        data[i] = sin(angle) * sin(angle);
        angle += incr;
    }
}
