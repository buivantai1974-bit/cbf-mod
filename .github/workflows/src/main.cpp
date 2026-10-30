#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/CCEGLView.hpp>
#include <deque>

using namespace geode::prelude;

struct InputEvent {
    double time;
    bool down;
    int button;
    bool player1;
};

struct Step {
    InputEvent input;
    double deltaFactor;
    bool endStep;
};

static std::deque<Step> g_stepQueue;
static double g_lastFrameTime = 0.0;
static double g_currentFrameTime = 0.0;
static bool g_enableInput = true;

static double nowSeconds() {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

class $modify(CCEGLView) {
    void pollEvents() {
        g_currentFrameTime = nowSeconds();
        CCEGLView::pollEvents();
    }
};

class $modify(GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        if (g_enableInput) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
        } else {
            InputEvent ev{g_currentFrameTime, down, button, isPlayer1};
            if (!g_stepQueue.empty()) {
                g_stepQueue.back().input = ev;
            }
        }
    }

    float getModifiedDelta(float dt) {
        float delta = GJBaseGameLayer::getModifiedDelta(dt);
        g_stepQueue.clear();

        int stepCount = static_cast<int>(std::round(std::max(1.0f, delta * 240.0f)));
        double stepDelta = static_cast<double>(delta) / stepCount;

        for (int i = 0; i < stepCount; ++i) {
            Step s;
            s.input = InputEvent{0.0, false, 0, true};
            s.deltaFactor = 1.0;
            s.endStep = (i == stepCount - 1);
            g_stepQueue.push_back(s);
        }

        return delta;
    }
};

class $modify(PlayerObject) {
    void update(float delta) {
        if (g_stepQueue.empty()) {
            PlayerObject::update(delta);
            return;
        }

        double baseDelta = static_cast<double>(delta) / g_stepQueue.size();

        for (auto& step : g_stepQueue) {
            g_enableInput = true;
            if (step.input.time > 0.0) {
                auto* layer = GJBaseGameLayer::get();
                if (layer) {
                    layer->handleButton(step.input.down, step.input.button, step.input.player1);
                }
            }
            g_enableInput = false;
            PlayerObject::update(static_cast<float>(baseDelta * step.deltaFactor));
        }

        g_enableInput = true;
        g_stepQueue.clear();
    }
};