#pragma once
#include <sstream>
#include <iostream>
#include <vector>
#include <list>

#include "Runtime.hpp"
#include "Actions.hpp"


using std::string;
using std::vector;
using actions::Action;

using ActionRef = shared_ptr<Action>;

enum class ParserErrors {NoError,  NoAgents, NoActions, WrongArgs, AgentNotFound, Other};

class Parser{
    public:
        Parser(Runtime& r);
        ParserErrors parse(string code);
        vector<string> split(const string&, const char delim );
        size_t parseAgents(const string& code, string& name, bool& err);
        vector<ActionRef> parseActions(const string& code, bool& err);
        ActionRef parseAction(const string& code, bool& err);
    
        size_t parseFreq(const string& freq, bool& err);
        void checkNumParams(list<string> params,int min, int max, bool& err);
    
        ActionRef parseGo(list<string> params, bool& err);
        ActionRef parseTurn(list<string> params, bool& err);
        ActionRef parseUp(list<string> params, bool& err);
        ActionRef parseDown(list<string> params, bool& err);
        ActionRef parseLeft(list<string> params, bool& err);
        ActionRef parseRight(list<string> params, bool& err);
        ActionRef parseWander(list<string> params, bool& err);
        ActionRef parseStop(list<string> params, bool& err);
        ActionRef parseSeek(list<string> params, bool& err);
        ActionRef parseDie(list<string> params, bool& err);
        ActionRef parseVolume(list<string> params, bool& err);
        ActionRef parseAvoid(list<string> params, bool& err);
        ActionRef parseJoin(list<string> params, bool& err);
        ActionRef parseAlign(list<string> params, bool& err);
        ActionRef parseMap(list<string> params, bool& err);
        ActionRef parseMake(list<string> params, bool& err);
        ActionRef parseBackground(list<string> params, bool& err);

private:
    Runtime& mRuntime;
        
};

Parser::Parser(Runtime& r):mRuntime(r){}

size_t Parser::parseAgents(const string& code, string& name, bool& err){
    std::size_t pos = code.find(":");
    if (pos == std::string::npos){
        err = true;
        return 0;
    };
    std::size_t start = code.find_first_not_of(" ");
    name = code.substr(start, pos);
    return pos;
}

ParserErrors Parser::parse(string code){
    Behaviour b;
    bool err = false;
    size_t pos;

    pos = parseAgents(code, b.flockName, err);
    if (err) return ParserErrors::NoAgents;
    if (!mRuntime.hasFlock(b.flockName))
        return ParserErrors::AgentNotFound;
    pos++;
    code = code.substr(pos, code.size());
    b.actions = parseActions(code, err);
    if(!err) err = mRuntime.addBehaviour(b);
    return ParserErrors::NoError;
}

vector<string> Parser::split(const string& code, const char delim =','){// TODO: delim?
    vector<string> strings;
    istringstream stream(code);
    string s;
    while (getline(stream, s, delim)) strings.push_back(s);
    return strings;
}

void Parser::checkNumParams(list<string> params, int min, int max, bool& err){
    size_t numParams = params.size();
    if(numParams < min || numParams > max ){
        err = true;
    }
}

vector<ActionRef> Parser::parseActions(const string& code, bool& err){
    vector<ActionRef> results;
    vector<string>  strings = split(code);
    if(strings.empty()) err = true;
    for(auto& s:strings){
        ActionRef a = parseAction(s, err);
        if (!err) results.push_back(a);
    }
    return results;
}


ActionRef Parser::parseAction(const string& code, bool& err){
    ActionRef result;
    istringstream stream(code);
    string word;
    list<string> words;
    while(stream >> word) words.push_back(word);
    string action = words.front();
    words.pop_front();
    try{
        if(action == "go") result = parseGo(words, err);
        else if(action == "turn") result =  parseTurn(words, err);
        else if(action == "up") result =  parseUp(words, err);
        else if(action == "down") result =  parseDown(words, err);
        else if(action == "left") result =  parseLeft(words, err);
        else if(action == "right") result =  parseRight(words, err);
        else if(action == "wander") result =  parseWander(words, err);
        else if(action == "stop") result =  parseStop(words, err);
        else if(action == "make") result =  parseMake(words, err);
        else if(action == "map") result =  parseMap(words, err);
        else if(action == "background") result =  parseBackground(words, err);
        else if(action == "seek") result =  parseSeek(words, err);
        else if(action == "volume") result =  parseVolume(words, err);
        else if(action == "die") result =  parseDie(words, err);
        else if(action == "avoid") result =  parseAvoid(words, err);
        else if(action == "join") result =  parseJoin(words, err);
        else if(action == "align") result =  parseAlign(words, err);
        else err = true;
    } catch (exception ex){
        err = true;
    }
    return result;
}

size_t Parser::parseFreq(const string& freq, bool& err){
    vector<string> freqs = {"once", "sometimes", "often", "always"};
    size_t pos = std::find(freqs.begin(), freqs.end(), freq) - freqs.begin();
    if(pos >= freqs.size()) err = 0;
    return pos;
}



ActionRef Parser::parseGo(list<string> params, bool& err){
    using namespace std;
    actions::go action;
    checkNumParams(params, 0, 2, err);
    if (!params.empty()) {action.mult = stof(params.front()); params.pop_front();}
    if (!params.empty()) {action.freq = parseFreq(params.front(), err); params.pop_front();}
    return ActionRef(std::make_shared<actions::go>(action));
}

ActionRef Parser::parseTurn(list<string> params, bool& err){
    using namespace std;
    actions::turn action;
    checkNumParams(params, 1, 2, err);
    if (!params.empty()) {action.angle = stof(params.front()); params.pop_front();}
    if (!params.empty()) {action.freq = parseFreq(params.front(), err); params.pop_front();}
    return ActionRef(std::make_shared<actions::turn>(action));
}

ActionRef Parser::parseWander(list<string> params, bool& err){
    using namespace std;
    actions::wander action;
    checkNumParams(params, 1, 2, err);
    if (!params.empty()) {action.prob = stof(params.front()); params.pop_front();}
    if (!params.empty()) {action.freq = parseFreq(params.front(), err); params.pop_front();}
    return ActionRef(std::make_shared<actions::wander>(action));
}

ActionRef Parser::parseUp(list<string> params, bool& err){
    using namespace std;
    actions::up action;
    checkNumParams(params, 0, 1, err);
    if (!params.empty()) {action.freq = parseFreq(params.front(), err); params.pop_front();}
    return ActionRef(std::make_shared<actions::up>(action));
}

ActionRef Parser::parseDown(list<string> params, bool& err){
    using namespace std;
    actions::down action;
    checkNumParams(params, 0, 1, err);
    if (!params.empty()) {action.freq = parseFreq(params.front(), err); params.pop_front();}
    return ActionRef(std::make_shared<actions::down>(action));
}

ActionRef Parser::parseLeft(list<string> params, bool& err){
    using namespace std;
    actions::left action;
    checkNumParams(params, 0, 1, err);
    if (!params.empty()) {action.freq = parseFreq(params.front(), err); params.pop_front();}
    return ActionRef(std::make_shared<actions::left>(action));
}

ActionRef Parser::parseRight(list<string> params, bool& err){
    using namespace std;
    actions::right action;
    checkNumParams(params, 0, 1, err);
    if (!params.empty()) {action.freq = parseFreq(params.front(), err); params.pop_front();}
    return ActionRef(std::make_shared<actions::right>(action));
}


ActionRef Parser::parseStop(list<string> params, bool& err){
    using namespace std;
    actions::stop action;
    checkNumParams(params, 0, 1, err);
    if (!params.empty()) {action.freq = parseFreq(params.front(), err); params.pop_front();}
    return ActionRef(std::make_shared<actions::stop>(action));
}

ActionRef Parser::parseSeek(list<string> params, bool& err){
    using namespace std;
    actions::seek action;
    checkNumParams(params, 2, 3, err);
    if (!params.empty()) {action.x = stof(params.front()); params.pop_front();}
    if (!params.empty()) {action.y = stof(params.front()); params.pop_front();}
    if (!params.empty()) {action.freq = parseFreq(params.front(), err); params.pop_front();}
    return ActionRef(std::make_shared<actions::seek>(action));
}


ActionRef Parser::parseVolume(list<string> params, bool& err){
    using namespace std;
    actions::volume action;
    checkNumParams(params, 1, 2, err);
    if (!params.empty()) {action.vol = stof(params.front()); params.pop_front();}
    if (!params.empty()) {action.freq = parseFreq(params.front(), err); params.pop_front();}
    return ActionRef(std::make_shared<actions::volume>(action));
}

ActionRef Parser::parseDie(list<string> params, bool& err){
    using namespace std;
    actions::die action;
    checkNumParams(params, 1, 2, err);
    if (!params.empty()) {action.prob = stof(params.front()); params.pop_front();}
    if (!params.empty()) {action.freq = parseFreq(params.front(), err); params.pop_front();}
    return ActionRef(std::make_shared<actions::die>(action));
}



// flock actions
ActionRef Parser::parseAvoid(list<string> params, bool& err){
    using namespace std;
    actions::avoid action;
    checkNumParams(params, 1, 4, err);
    if (!params.empty()) {action.threshold = stof(params.front()); params.pop_front();}
    if (!params.empty()) {action.strength = stof(params.front()); params.pop_front();}
    if (!params.empty()) {action.target = params.front(); params.pop_front();}
    if (!params.empty()) {action.freq = parseFreq(params.front(), err); params.pop_front();}
    return ActionRef(std::make_shared<actions::avoid>(action));
}

ActionRef Parser::parseJoin(list<string> params, bool& err){
    using namespace std;
    actions::join action;
    checkNumParams(params, 1, 4, err);
    if (!params.empty()) {action.threshold = stof(params.front()); params.pop_front();}
    if (!params.empty()) {action.strength = stof(params.front()); params.pop_front();}
    if (!params.empty()) {action.target = params.front(); params.pop_front();}
    if (!params.empty()) {action.freq = parseFreq(params.front(), err); params.pop_front();}
    return ActionRef(std::make_shared<actions::join>(action));
}

ActionRef Parser::parseAlign(list<string> params, bool& err){
    using namespace std;
    actions::align action;
    checkNumParams(params, 1, 4, err);
    if (!params.empty()) {action.threshold = stof(params.front()); params.pop_front();}
    if (!params.empty()) {action.strength = stof(params.front()); params.pop_front();}
    if (!params.empty()) {action.target = params.front(); params.pop_front();}
    if (!params.empty()) {action.freq = parseFreq(params.front(), err); params.pop_front();}
    return ActionRef(std::make_shared<actions::align>(action));
}


// World actions
ActionRef Parser::parseBackground(list<string> params, bool& err){
    using namespace std;
    actions::background action;
    checkNumParams(params, 1, 1, err);
    if (!params.empty()) {action.color = params.front(); params.pop_front();}
    action.color[0] = toupper(action.color[0]);
    return ActionRef(std::make_shared<actions::background>(action));
}


ActionRef Parser::parseMake(list<string> params, bool& err){
    using namespace std;
    actions::make action;
    checkNumParams(params, 2, 4, err);
    try{
        if (!params.empty()) {action.name = params.front(); params.pop_front();}
        if (!params.empty()) {action.num = stoi(params.front()); params.pop_front();}
        if (!params.empty()) {action.icon = params.front(); params.pop_front();}
        if (!params.empty()) {action.color = params.front(); params.pop_front();}
    } catch (exception ex){
        err = true;
    }
    return ActionRef(std::make_shared<actions::make>(action));
}

ActionRef Parser::parseMap(list<string> params, bool& err){
    using namespace std;
    actions::map action;
    checkNumParams(params, 1, 1, err);
    if (!params.empty()) {action.file = params.front(); params.pop_front();}
    return ActionRef(std::make_shared<actions::map>(action));
}
