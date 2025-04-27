#pragma once

#include <cstdlib>
#include <ctime>
#include <iostream>
#include "cinder/audio/audio.h"
#include <data/TensorTypes.hpp>
#include "cinder/Triangulate.h"

using namespace cinder;
using namespace fluid;

class Sound{
public:
    Sound(audio::SourceFileRef);
    Sound(audio::BufferRef, size_t sampleRate);
    audio::BufferRef mBuffer;
    RealVector  mfccStats;
    RealVector loudnessVec;
    double minLoudness, maxLoudness;
    size_t mSampleRate;
    void makeWave(int x, int y, int width, int heigth,
                  double min, double max);
    void draw();
    gl::BatchRef mWave;
    vec2 mPos;
    vec2 mDims;
};


Sound::Sound(audio::SourceFileRef src){
    mBuffer = src->loadBuffer();
    mSampleRate = src->getSampleRate();
}

Sound::Sound(audio::BufferRef src, size_t sampleRate):
    mBuffer(src), mSampleRate(sampleRate){}

void Sound::makeWave(int x, int y, int width, int height,
                     double min, double max){
    float step = float(loudnessVec.size()) / width;
    float xPos = x;
    float yPos = y + height;
    double rangeInv = 1 / (max - min);
    Path2d path;
    path.moveTo(vec2(xPos, yPos));
    for (int i = 1; i < width - 1; i++){
        double val = (loudnessVec(i * step) - min) * rangeInv;
        val = std::clamp(val, 0.0, 1.0);
        xPos = x + i;
        yPos =  (y + height) - (height * val);
        path.lineTo(vec2(xPos, yPos));
    }
    path.lineTo(vec2(xPos, y + height));
    path.lineTo(vec2(x, y + height));
    path.close();
    
    gl::VboMeshRef mesh = gl::VboMesh::create(
                Triangulator {path}.calcMesh()
    );
    mWave = gl::Batch::create(
                mesh, gl::getStockShader(gl::ShaderDef().color())
    );
    mPos = {x,y};
    mDims = {width, height};
}

void Sound::draw(){
    ci::gl::color(ci::Color(0.5, 0.5, 0.5));
    mWave->draw();
}
