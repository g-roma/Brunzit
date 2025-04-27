#pragma once
#include <random>
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "Corpus.hpp"
#include "Synth.hpp"

using namespace cinder;
using SynthRef = shared_ptr<Synth>;

class Agent {
    public:
        Agent(vec2 pos, vec2 vel = {0,0}, string icon="▶",
              string color = "white", Corpus* c = nullptr);
        void update();
        void draw();
        void wrap();
    
        void setVolume(float f);
        void applyForce(const vec2 &force);
        vec2 getPosition();
        vec2 getVelocity();
        vec2 getDirection();
    
        void go(float mult);
        void wander(float prob);
        void turn(float deg);
        void seek(float x, float y);
        void up();
        void down();
        void left();
        void right();
        void stop();
        void die();
        
    private:
        void computeHeading();
        vec2 mPosition, mVelocity, mAcceleration;
        vec2 mNormVelocity;
        float mRadius, mDecay, mMaxSpeed, mHeading;
        string mIcon;
        Color mColor;
        SynthRef mSynth;
        bool mDead{false};
};

Agent::Agent(vec2 position, vec2 velocity,
             string icon, string color,
             Corpus* c) {
    mPosition = position;
    mVelocity = velocity;
    mIcon = icon;
    color[0] = toupper(color[0]);
    mColor = svgNameToRgb(color.c_str());
    mAcceleration = vec2(0.0f);
    computeHeading();
    mMaxSpeed = 4.;
    mDecay = 0.99f;
    if(c->mEngine == 0)
        mSynth = SynthRef(new AdditiveSynth(c));
    else
        mSynth = SynthRef(new GranularSynth(c));
    if(mSynth) mSynth->update(mPosition);
}

void Agent::computeHeading(){
    if (mVelocity[0] == 0 && mVelocity[1] == 0){
        mNormVelocity = mVelocity;
        mHeading = 0;
    }
    else {
        mNormVelocity = glm::normalize(mVelocity);
        mHeading = atan2(mVelocity[1], mVelocity[0]);
    }
}

void Agent::wrap(){
    int width = app::getWindowWidth();
    int height = app::getWindowHeight();
    if (mPosition.x > width) mPosition.x -= width;
    else if (mPosition.x < 0) mPosition.x += width;
    if (mPosition.y > height) mPosition.y -=  height;
    else if (mPosition.y < 0) mPosition.y += height;
}

void Agent::update() {
    mVelocity += mAcceleration;
    float v = glm::length2(mVelocity);
    float mag = length(mVelocity);
    if (mag > mMaxSpeed) mVelocity *= (mMaxSpeed / mag);
    mPosition +=mVelocity;
    mAcceleration *= 0;
    computeHeading();
    if(mSynth) mSynth->update(mPosition);
}

void Agent::draw() {
    if(mDead) return;
    gl::pushModelMatrix();
    gl::translate(mPosition);
    gl::rotate(mHeading);
    gl::drawString(mIcon, {0,0}, mColor);
    gl::popModelMatrix();
}

void Agent::applyForce(const vec2 &force) {
    mAcceleration += force;
}

vec2 Agent::getPosition() {
    return mPosition;
}

vec2 Agent::getVelocity() {
    return mVelocity;
}

vec2 Agent::getDirection() {
    return mNormVelocity;
}

void Agent::go(float mul){
    update();
    mPosition += (mVelocity * mul);
    wrap();
}

void Agent::turn(float angle){
    float rad = 2 * M_PI * angle / 360.0;
    float x = mVelocity[0];
    float y = mVelocity[1];
    float c = std::cos(rad);
    float s = std::sin(rad);
    mVelocity[0] = x * c - y * s;
    mVelocity[1] = x * s + y * c;
    computeHeading();
}

void Agent::up(){
    mVelocity[0] = 0;
    mVelocity[1] = -1;
    computeHeading();
}

void Agent::down(){
    mVelocity[0] = 0;
    mVelocity[1] = 1;
    computeHeading();
}

void Agent::left(){
    mVelocity[0] = -1;
    mVelocity[1] = 0;
    computeHeading();
}

void Agent::right(){
    mVelocity[0] = 1;
    mVelocity[1] = 0;
    computeHeading();
}

void Agent::stop(){
    go(0);
}

void Agent::wander(float prob){
    float r = (float) rand() / (RAND_MAX);
    if (r < prob){
        float alpha = 180 * ((float) rand() / (RAND_MAX)) - 90 ; 
        turn(alpha);
        computeHeading();
    }
}

void Agent::seek(float x, float y){
    vec2 target(x,y);
    mVelocity = normalize(target - mPosition);
    computeHeading();
}

void Agent::setVolume(float f){
    mSynth->setVolume(f);
};

void Agent::die(){
    mDead = true;
    mSynth->stop();
};
