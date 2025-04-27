#pragma once
#include <random>

#include "cinder/Rand.h"
#include "cinder/audio/Context.h"
#include "cinder/audio/GenNode.h"
#include "cinder/audio/GainNode.h"

#include "Corpus.hpp"

using namespace cinder;
using namespace std;

class Synth {
    public:
        Synth(Corpus* c):mCorpus(c){};
        void start();
        void stop();
        void setVolume(float v);
        virtual void update(vec2 pos)=0;
    protected:
        audio::VoiceRef mVoice;
        Corpus* mCorpus;
};

void Synth::start(){
    mVoice->setVolume(0.);
    mVoice->start();
};

void Synth::stop(){
    mVoice->setVolume(0);
    mVoice->pause();
};
void Synth::setVolume(float f){
    mVoice->setVolume(f);
};

// Additive

class AdditiveSynth: public Synth{
    public:
        AdditiveSynth(Corpus* c);
        void update(vec2 pos) override;
    private:
        float mPhase = 0.0f;
        std::atomic<float> mFreq{440};
};

AdditiveSynth::AdditiveSynth(Corpus* c):Synth(c){
   mPhase = M_PI * (float) rand() / (RAND_MAX);
    mVoice = audio::Voice::create( [this] ( audio::Buffer *buffer, size_t sampleRate ) {
       float* channel0 = buffer->getChannel(0);
       float phaseIncr = ( this->mFreq / (float)sampleRate ) * 2 * (float) M_PI;
       for( size_t i = 0; i < buffer->getNumFrames(); i++ )    {
           this->mPhase = fmodf( this->mPhase + phaseIncr, 2 * M_PI );
           channel0[i] = std::sin( this->mPhase );
       }
    });
    mVoice->setVolume(0.0);//TODO
    mVoice->start();
}

void AdditiveSynth::update(vec2 pos){
    float currentColour = mCorpus->mChannel.getValue(pos);
    mFreq = 20 + 1000 * currentColour;
}


// Granular

class GranularSynth:public Synth{
    public:
        GranularSynth(Corpus* c);
        void update(vec2 pos) override;

    private:
        audio::BufferRef mCurrentBuffer;
        std::atomic<int> mBufIndex;
        static const int nOverlap = 8;
        std::atomic<std::array<float*,nOverlap>> mBuffers;
        std::atomic<std::array<int,nOverlap>> mIndices;
        std::atomic<std::array<int,nOverlap>> mOffsets;
        std::atomic<int> mSampleCount = 0;
        std::atomic<int> lastTrig = 0;
        std::atomic<int> mTrigRate;
        std::atomic<bool> mRandomize{true};
};

GranularSynth::GranularSynth(Corpus* c):Synth(c){
    mTrigRate = mCorpus->ENV_SIZE / nOverlap;
    auto idx = mIndices.load();
    for( size_t j = 0; j < nOverlap; j++ ) idx[j] = 0;
    mIndices.store(idx);
    
    auto off = mOffsets.load();
    for( size_t j = 0; j < nOverlap; j++ ) off[j] = 0;
    mOffsets.store(off);

    
    mVoice = audio::Voice::create( [this] ( audio::Buffer* buffer, size_t sampleRate ) {
        if(mCurrentBuffer == nullptr) return;
        float *channel0 = buffer->getChannel(0);
        auto idx = mIndices.load();
        auto bufs = mBuffers.load();
        auto off = mOffsets.load();

        for( size_t i = 0; i < buffer->getNumFrames(); i++ ) {
            float val = 0;
            // compute sample value
            for( size_t j = 0; j < nOverlap; j++ ) {
                if (bufs[j] != nullptr) {
                    float* data = bufs[j];
                    val += mCorpus->envelope[idx[j]] * data[off[j] + idx[j]];
                    idx[j]++;
                    if ( idx[j] >= mCorpus->ENV_SIZE)  idx[j] = 0;
                }
            }
            channel0[i] = val;
            // trigger new grains
            if (mSampleCount++ >= mTrigRate){
                lastTrig++;
                if (lastTrig >= nOverlap) lastTrig = 0;
                bufs[lastTrig] = mCurrentBuffer->getData();
                idx[lastTrig] = 0;
                if(mRandomize) off[lastTrig] = int(
                        (mCorpus->GRAIN_SIZE - mCorpus->ENV_SIZE)*((float) rand() / (RAND_MAX))
                );
                mSampleCount = 0;
            }
        }
        mIndices.store(idx);
        mOffsets.store(off);
        mBuffers.store(bufs);
    } );
    start();
}

void GranularSynth::update(vec2 pos) {
    if (!mCorpus->empty()){
        int x = int(mCorpus->mMaxX * pos[0] / app::getWindowWidth());
        int y = int(mCorpus->mMaxY * pos[1] / app::getWindowHeight());
        std::pair<int,int> key = make_pair(x, y);
        int snd = mCorpus->mPositionsMap[key];
        mCurrentBuffer = mCorpus->mSounds[snd].mBuffer;
        auto bufs = mBuffers.load();
        if(bufs[0] == nullptr) bufs[0] = mCurrentBuffer->getData();
        mBuffers.store(bufs);
    }
}
    
