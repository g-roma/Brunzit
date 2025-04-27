#pragma once

#include <string>
#include "cinder/Color.h"
#include "cinder/gl/gl.h"
#include "Actions.hpp"
#include "Flock.hpp"
#include  <map>
#include <filesystem>

using std::string;
using cinder::Color;
using namespace actions;

using ActionRef = shared_ptr<Action>;

struct Behaviour{
    string flockName;
    vector<ActionRef> actions;
};

class Runtime{
    public:
        std::vector<string>& getFlockNames();
        bool hasFlock(std::string name);
        bool addBehaviour(Behaviour b);
        bool runWorldActions(Behaviour b);
        void runFlockActions(Behaviour& b);
        bool addFlock(ActionRef a);
        bool makeMap(ActionRef a);
        bool changeBackground(ActionRef a);
        void update();
        void draw();
        Color bgColor{0,0,0};
        Corpus mCorpus;
    private:
        std::vector<string> mFlockNames;
        std::unordered_map<string, Flock> mFlocks;
        std::unordered_map<string, Behaviour> mBehaviours;
        std::unordered_map<string, string> mIcons{
            {"triangle","▶"},{"square","■" },
            {"circle","●"},{"trigram","☳" },
            {"arow", "→"},{"arrowhead","➤"},
            {"happy","☺"},{"sad","☹"},
            {"heart", "♥"} , {"nuclear", "☢"},
            {"snowman", "⛄"},{ "shade", "░"}, {"CIRCLE", "◯"}
        };
};

std::vector<string>& Runtime::getFlockNames(){return mFlockNames;};


void Runtime::update(){
    for(auto f:mFlockNames){
        if (mBehaviours.find(f) != mBehaviours.end()) {
            runFlockActions(mBehaviours.at(f));
        }
    }
}

void Runtime::draw(){
    gl::clear(bgColor);
    mCorpus.draw();
    for(auto& f:mFlocks) f.second.draw();
}

bool Runtime::hasFlock(std::string name){
    if (name =="world") return true;
    else return std::find(mFlockNames.begin(), mFlockNames.end(), name)
           != mFlockNames.end();
}

bool Runtime::addBehaviour(Behaviour b){
    if (b.flockName == "world") return runWorldActions(b);
    else if (hasFlock(b.flockName)){
        mBehaviours[b.flockName] = b;
        return true;
    }
    else return false;
}

void Runtime::runFlockActions(Behaviour& b){
    Flock f = mFlocks.at(b.flockName);

    auto iter = b.actions.begin();
    while (iter != b.actions.end()){
        auto a = *iter;
        if (a == nullptr) continue;
        switch(a->type){
            case  ActionType::go:
            {
                go* g = static_cast<go*>(a.get());
                f.go(g->mult, g->freq);
                break;
            }
                
            case ActionType::turn:
            {
                turn* g = static_cast<turn*>(a.get());
                f.turn(g->angle, g->freq);
                break;
            }
            case ActionType::up:
            {
                up* g = static_cast<up*>(a.get());
                f.up(g->freq);
                break;
            }
            case ActionType::down:
            {
                down* g = static_cast<down*>(a.get());
                f.down(g->freq);
                break;
            }
            case ActionType::left:
            {
                actions::left* g = static_cast<actions::left*>(a.get());
                f.left(g->freq);
                break;
            }
            case ActionType::right:
            {
                actions::right* g = static_cast<actions::right*>(a.get());
                f.right(g->freq);
                break;
            }

            case ActionType::stop:
            {
                stop* g = static_cast<stop*>(a.get());
                f.stop(g->freq);
                break;
            }
            case ActionType::wander:
            {
                wander* g = static_cast<wander*>(a.get());
                f.wander(g->prob, g->freq);
                break;
            }
            case ActionType::seek:
            {
                seek* g = static_cast<seek*>(a.get());
                f.seek(g->x, g->y, g->freq);
                break;
            }
            case ActionType::avoid:
            {
                avoid* g = static_cast<avoid*>(a.get());
                f.avoid(g->threshold, g->strength, g->target, g->freq);
                break;
            }
            case ActionType::join:
            {
                join* g = static_cast<join*>(a.get());
                f.join(g->threshold, g->strength, g->target, g->freq);
                break;
            }
            case ActionType::align:
            {
                actions::align* g = static_cast<actions::align*>(a.get());
                f.align(g->threshold, g->strength, g->target, g->freq);
                break;
            }

            case ActionType::die:
            {
                actions::die* g = static_cast<actions::die*>(a.get());
                f.die(g->prob, g->freq);
                break;
            }

            case ActionType::volume:
            {
                actions::volume* g = static_cast<actions::volume*>(a.get());
                f.volume(g->vol, g->freq);
                break;
            }
            default: break;
        }
        if (a->freq == 0){// do once
            iter = b.actions.erase(iter);
        } else iter++;
    }
}

bool Runtime::runWorldActions(Behaviour b){
    bool result;
    for (auto&& a:b.actions){
        if(a->type == ActionType::make){result = addFlock(a);}
        else if (a->type == ActionType::map){result = makeMap(a);}
        else if (a->type == ActionType::background){result = changeBackground(a);}
        else return false;
    }
    return result;
}

bool Runtime::changeBackground(ActionRef a){
    background* b = (background*)(a.get());
    bgColor = svgNameToRgb(b->color.c_str());
    return true;
}

bool Runtime::addFlock(ActionRef a){
    make* m = (make*)(a.get());
    string icon = m->icon;
    if (mIcons.find(icon) != mIcons.end()) icon = mIcons[icon];
    Flock f = Flock(m->num, icon, m->color, &mCorpus);
    mFlocks.emplace(std::make_pair(m->name,f));
    mFlockNames.push_back(m->name);
    return true;
}

bool Runtime::makeMap(ActionRef a){
    using map = actions::map;
    map* m = (map*)(a.get());
    fs::path filePath = app::getAssetPath(m->file);
    if (filePath.empty()) return false;
    if (filePath.extension() == ".png"){
            mCorpus.mChannel = loadImage(app::loadAsset(filePath));
            mCorpus.mTexture = gl::Texture2d::create(mCorpus.mChannel);
            mCorpus.mEngine = 0;
    }
    else if (filePath.extension() == ".wav"){
            auto ctx = audio::Context::master();
            auto src = audio::load(app::loadAsset(filePath));
            auto buf = src->loadBuffer();
            mCorpus.slice(buf, src->getSampleRate());
            mCorpus.project();
            mCorpus.makeWaves();
            mCorpus.mEngine = 1;
    }
    else return false;
    return true;
}
