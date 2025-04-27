#include <random>
#include <chrono>

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/CinderImGui.h"

#include "Agent.hpp"
#include "Flock.hpp"
#include "Corpus.hpp"
#include "Parser.hpp"
#include "Runtime.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

static const int CODE_SIZE = 2048;

class Brunzit : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;
    void drawUI();
    Corpus& getCorpus();
    
  private:
    std::vector<char> mCode;
    std::vector<string> mFlocks;
    Runtime mRuntime;
    Parser mParser{mRuntime};
    bool mError{false};
    bool mShowTextbox{true};
    bool mShowHelp{false};
    
};


void Brunzit::setup()
{
    std::mt19937_64 rng;
    uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
    rng.seed(ss);
    ImGui::Initialize(ImGui::Options().autoRender(true));
    mCode = std::vector<char>(CODE_SIZE);
    std::fill(mCode.begin(), mCode.end(),'\0');
}

void Brunzit::update()
{
    drawUI();
    mRuntime.update();
}

void Brunzit::drawUI(){
    float currentHeight = static_cast<float>(getWindowHeight());
    float currentWidth = static_cast<float>(getWindowWidth());
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))){
        mShowTextbox = !mShowTextbox;
    }
    
    if(ImGui::IsKeyPressed(282)){// F1 on mac
        mShowHelp = !mShowHelp;
    }

    ImGuiWindowFlags window_flags = 0;
    
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoBackground;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    
    if(mShowHelp){
        ImGui::SetNextWindowSize({currentWidth - 200,400});
        ImGui::SetNextWindowPos({
            100.0f,
            100
        });
        ImGui::Begin("help", NULL, window_flags);
        ImVec4 textColor = ImVec4(1.0, 1.0, 1.0, 0.5f);
        ImGui::Text("World actions:");
        ImGui::Text("make - name num (icon) (color)");
        ImGui::Text("map - filename");
        ImGui::Text(" ");
        ImGui::Text("Flock actions:");
        ImGui::Text("volume - vol");
        ImGui::Text("join - threshold (strength) (target) (freq)");
        ImGui::Text("avoid - threshold (strength) (target) (freq)");
        ImGui::Text("align - threshold (strength) (target) (freq)");
        ImGui::Text(" ");
        ImGui::Text("Agent actions:");
        ImGui::Text("go - (mult) (freq)");
        ImGui::Text("turn - angle (freq)");
        ImGui::Text("up - (freq)");
        ImGui::Text("down - (freq)");
        ImGui::Text("left - (freq)");
        ImGui::Text("right - (freq)");
        ImGui::Text("seek - x y (freq)");
        ImGui::Text("wander - prob (freq)");
        ImGui::Text("die - prob (freq)");
        ImGui::End();
    }
    
    if (!mShowTextbox) return;
    ImVec4 color = ImVec4(0.2, 0.2, 0.2, 1.0);
    ImGui::PushStyleColor(ImGuiCol_TitleBg, color);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, color);
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, color);
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, color);

    ImGui::SetNextWindowSize({currentWidth - 200,100});
    ImGui::SetNextWindowPos({
        100.0f,
        currentHeight - 120
    });
    ImGui::Begin("code", NULL, window_flags);
    ImGui::PushItemWidth(-1);
    ImGuiInputTextFlags textFlags = 0;
    textFlags |= ImGuiInputTextFlags_EnterReturnsTrue;
    ImVec4 textColor = ImVec4(1.0, 1.0, 1.0, 0.5f);
    if(mError) textColor = ImVec4(1.0, 0.0, 0.0, 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    ImGui::SetKeyboardFocusHere(0);
    bool enter = ImGui::InputText("input", &mCode[0], 2048, textFlags);
    ImGui::PopStyleColor();
    if (enter){
        auto end = std::find(mCode.begin(),mCode.end(), '\0' );
        ParserErrors errors =  mParser.parse(std::string(mCode.begin(),end));
        mError = (errors != ParserErrors::NoError);
        if(!mError) std::fill(mCode.begin(), mCode.end(), '\0');
        
    }
    ImGui::PopItemWidth();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::End();
}

void Brunzit::draw()
{
    mRuntime.draw();
}

CINDER_APP(
        Brunzit,
        RendererGl(RendererGl::Options().msaa(2)),
        [&](App::Settings *settings) {
            settings->setWindowSize(1280, 720);
            settings->setFrameRate(30.0f);
            settings->setHighDensityDisplayEnabled(false);
        }
)
