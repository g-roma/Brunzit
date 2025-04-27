#pragma once

#include <list>

using std::list;
using std::string;


class Flock {
public:
    Flock(int num, string icon, string color, Corpus* c);
    void update();
    void go(float m, int freq = 3);
    void turn(float m, int freq = 0);
    void up(int freq = 0);
    void down(int freq = 0);
    void left(int freq = 0);
    void right(int freq = 0);
    void stop(int freq = 0);
    void die(float p, int freq = 0);
    void volume(float v, int freq = 0);
    void wander(float p, int freq = 1);
    void seek(float x, float y, int freq = 0);
    void avoid(float threshold, float strength, string target, int freq = 3);
    void join(float threshold, float strength, string target, int freq = 3);
    void align(float threshold, float strength, string target, int freq = 3);
    void print();
    void draw();
    
private:
    vec2 seek(vec2 target, vec2 pos, vec2 vel);
    bool evalFreq(int freq);

    std::list<std::shared_ptr<Agent>> mAgents;
    float mMaxSpeed{4};
    float mMaxForce{0.05};
};

Flock::Flock(int num, string icon, string color, Corpus* c){
    int width = app::getWindowWidth();
    int height = app::getWindowHeight();
    for(int i = 0; i < num; i++){
        float x =  width * (float) rand() / (RAND_MAX);
        float y =  height * (float) rand() / (RAND_MAX);
        float dx =   (float) rand() / (RAND_MAX);
        float dy =  (float) rand() / (RAND_MAX);
        mAgents.push_back(
            std::make_shared<Agent>(
                vec2(x,y), vec2(dx, dy), icon, color, c
            )
        );
    }
    float vol = 0.05 / float(num);
    for( auto& a : mAgents){
        a->setVolume(vol);
    }
}

void Flock::draw(){
    for( auto& a : mAgents){
       a->draw();
    }
}

void Flock::print(){
    for( auto& a : mAgents){
        std::cout<<a->getVelocity()<<std::endl;
    }
}

bool Flock::evalFreq(int freq){
    float dice = (float) rand() / (RAND_MAX);
    switch (freq){
        case(0): return true; //once, will be removed
        case(1): return dice < 0.3; // sometimes
        case(2): return dice < 0.8; // often
        case(3): return true; // always
        default: return false;
    }
    return false;
}

void Flock::wander(float p, int freq){
    for( auto& a : mAgents) if(evalFreq(freq)) a->wander(p);
}

void Flock::go(float m, int freq){
    for( auto& a : mAgents) if(evalFreq(freq))a->go(m);
}

void Flock::turn(float m, int freq){
    for( auto& a : mAgents) if(evalFreq(freq))a->turn(m);
}

void Flock::up(int freq){
    for( auto& a : mAgents) if(evalFreq(freq))a->up();
}

void Flock::down(int freq){
    for( auto& a : mAgents) if(evalFreq(freq))a->down();
}

void Flock::left(int freq){
    for( auto& a : mAgents) if(evalFreq(freq))a->left();
}

void Flock::right(int freq){
    for( auto& a : mAgents) if(evalFreq(freq))a->right();
}

void Flock::stop(int freq){
    for( auto& a : mAgents) if(evalFreq(freq))a->stop();
}

void Flock::die(float p, int freq){
    for( auto& a : mAgents)
        if(evalFreq(freq) && ((float) rand() / (RAND_MAX)) < p)
            a->die();
}


void Flock::volume(float p, int freq){
    float vol = p / mAgents.size();

    for( auto& a : mAgents) if(evalFreq(freq)){
        a->setVolume(vol);
    };
}


void die(float p, int freq = 0);
void volume(float v, int freq = 0);


void Flock::seek(float x, float y, int freq){
    for( auto& a : mAgents) if(evalFreq(freq))a->seek(x, y);
}


void Flock::avoid(float threshold, float strength, string target, int freq){
    if(!evalFreq(freq))return;
    vec2 steer(0,0);
    int count = 0;
    for( auto& a1 :mAgents) {
        for( auto& a2 :mAgents) {
            vec2 diff = a1->getPosition() - a2->getPosition();
            float dist = length(diff);
            if (dist >0 && dist < threshold){
                steer += (normalize(diff) / dist);
                count++;
            }
        }
        if (count > 0) {
            steer /= (float)count;
        }
        if (length(steer)> 0) {
              steer = normalize(steer) * mMaxSpeed;
              steer -= a1->getVelocity();
            float mag = length(steer);
            if (mag > mMaxForce) steer *= (mMaxForce/mag);
        }
        a1->applyForce( steer * strength);
    }
}

void Flock::join(float threshold, float strength, string target, int freq){
    if(!evalFreq(freq))return;
    vec2 centroid(0,0);
    int count = 0;
    for( auto& a1 :mAgents) {
        for( auto& a2 :mAgents) {
            float dist = distance(a1->getPosition(), a2->getPosition());
            if (dist >0 && dist < threshold){
                centroid += a2->getPosition();
                count++;
            }
        }
        if (count > 0) {
            centroid /= (float)count;
            a1->applyForce(seek(centroid, a1->getPosition(), a1->getVelocity()) * strength);
        }
    }
}

void Flock::align(float threshold, float strength, string target, int freq){
    if(!evalFreq(freq))return;
    vec2 centroid(0,0);
    int count = 0;
    for( auto& a1 :mAgents) {
        for( auto& a2 :mAgents) {
            float dist = distance(a1->getPosition(), a2->getPosition());
            if (dist >0 && dist < threshold){
                centroid += a2->getVelocity();
                count++;
            }
        }
        if (count > 0) {
            centroid /= (float)count;
            centroid = normalize(centroid);
            vec2 steer = centroid - a1->getVelocity();
            float mag = length(steer);
            if (mag > mMaxForce) steer *= (mMaxForce/mag);
            a1->applyForce(steer * strength);
        }
    }
}

vec2 Flock::seek(vec2 target, vec2 pos, vec2 vel) {
    vec2 steer = normalize(target - pos) * mMaxSpeed;
    steer -= vel;
    float mag = length(steer);
    if (mag > mMaxForce) steer *= (mMaxForce/mag);
    return steer;
}
