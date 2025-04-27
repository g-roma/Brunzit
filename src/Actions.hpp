#pragma once

#include <string>

using std::string;

namespace actions{

enum class ActionType{
    go, up, down, left, right, turn, stop,
    die, volume, seek, wander, avoid, join,
    align, make, map, background};

struct Action{
    ActionType type;
    int freq = 0;
    Action(ActionType t) : type(t){}
};

struct go: public Action{
    float mult = 1.0;
    go():Action::Action(ActionType::go){freq = 3;}
    
};

struct up: public Action{
    up():Action::Action(ActionType::up){freq = 0;}
};

struct down: public Action{
    down():Action::Action(ActionType::down){freq = 0;}
};

struct left: public Action{
    left():Action::Action(ActionType::left){freq = 0;}
};

struct right: public Action{
    right():Action::Action(ActionType::right){freq = 0;}
};

struct turn: public Action{
    float angle;
    turn():Action::Action(ActionType::turn){freq = 0;}
};

struct stop: public Action{
    stop():Action::Action(ActionType::stop){freq = 0;}
};

struct die: public Action{
    float prob;
    die():Action::Action(ActionType::die){freq = 0;}
};

struct volume: public Action{
    float vol;
    volume():Action::Action(ActionType::volume){freq = 0;}
};

struct seek: public Action{
    float x;
    float y;
    seek():Action::Action(ActionType::seek){freq = 3;}
};

struct wander: public Action{
    float prob;
    wander():Action::Action(ActionType::wander){freq = 2;}
};

struct avoid: public Action{
    float threshold;
    float strength{1};
    string target{""};
    avoid():Action::Action(ActionType::avoid){freq = 3;}
};

struct join: public Action{
    float threshold;
    float strength{1};
    string target{""};
    join():Action::Action(ActionType::join){freq = 3;}
};

struct align: public Action{
    float threshold;
    float strength{1};
    string target{""};
    align():Action::Action(ActionType::align){freq = 3;}
};

struct make: public Action{
    string name;
    int num;
    string icon{"▶"};
    string color{"white"};
    make():Action::Action(ActionType::make){}
};

struct map: public Action{
    map():Action::Action(ActionType::map){}
    string file;
};

struct background: public Action{
    string color = "Black";
    background():Action::Action(ActionType::background){}
};


}
