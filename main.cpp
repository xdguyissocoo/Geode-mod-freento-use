#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/PauseLayer.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

using namespace geode::prelude;

namespace {
    bool g_indicator = true;
    bool g_showcase = false;
    float g_demoTime = 0.f;
    size_t g_demoIndex = 0;

    struct DemoEvent { float t; bool down; };
    constexpr DemoEvent DEMO[] = {
        {0.20f,true},{0.30f,false},{0.72f,true},{0.82f,false},
        {1.18f,true},{1.28f,false},{1.64f,true},{1.74f,false},
        {2.05f,true},{2.15f,false},{2.52f,true},{2.62f,false},
        {3.00f,true},{3.10f,false},{3.48f,true},{3.58f,false},
        {3.95f,true},{4.05f,false},{4.45f,true},{4.55f,false},
        {4.92f,true},{5.02f,false},{5.42f,true},{5.52f,false},
        {5.95f,true},{6.05f,false},{6.48f,true},{6.58f,false},
        {7.00f,true},{7.10f,false}
    };

    void resetDemo() {
        g_showcase = false;
        g_demoTime = 0.f;
        g_demoIndex = 0;
    }

    void flashIndicator(PlayerObject* player) {
        if (!g_indicator || !player) return;
        auto ring = CCSprite::create("click-ring.png"_spr);
        if (!ring) return;
        ring->setPosition({0.f, 0.f});
        ring->setScale(0.45f);
        ring->setOpacity(235);
        ring->setZOrder(10000);
        player->addChild(ring);
        ring->runAction(CCSequence::create(
            CCSpawn::create(
                CCEaseOut::create(CCScaleTo::create(0.13f, 0.90f), 2.f),
                CCEaseOut::create(CCFadeOut::create(0.13f), 2.f),
                nullptr
            ),
            CCCallFunc::create(ring, callfunc_selector(CCNode::removeFromParent)),
            nullptr
        ));
    }
}

class $modify(ClickIndicatorPlayer, PlayerObject) {
    bool pushButton(PlayerButton button) {
        auto result = PlayerObject::pushButton(button);
        auto pl = PlayLayer::get();
        if (pl && this == pl->m_player1)
            flashIndicator(this);
        return result;
    }
};

class $modify(ClickIndicatorPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        resetDemo();
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (!g_showcase || !m_player1) return;

        g_demoTime += dt;
        while (g_demoIndex < std::size(DEMO) && g_demoTime >= DEMO[g_demoIndex].t) {
            if (DEMO[g_demoIndex].down)
                m_player1->pushButton(PlayerButton::Jump);
            else
                m_player1->releaseButton(PlayerButton::Jump);
            ++g_demoIndex;
        }
        if (g_demoIndex >= std::size(DEMO)) resetDemo();
    }

    void onQuit() {
        resetDemo();
        PlayLayer::onQuit();
    }
};

class $modify(ClickIndicatorPause, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto win = CCDirector::sharedDirector()->getWinSize();
        auto menu = CCMenu::create();
        menu->setPosition({win.width - 85.f, win.height - 65.f});
        menu->setZOrder(5000);

        auto indicatorSprite = ButtonSprite::create(g_indicator ? "Click: ON" : "Click: OFF", "bigFont.fnt", "GJ_button_01.png", 0.55f);
        indicatorSprite->setScale(0.55f);
        auto indicator = CCMenuItemSpriteExtra::create(
            indicatorSprite, this, menu_selector(ClickIndicatorPause::toggleIndicator));
        indicator->setPosition({0.f, 0.f});

        auto demoSprite = ButtonSprite::create("Showcase", "bigFont.fnt", "GJ_button_02.png", 0.55f);
        demoSprite->setScale(0.55f);
        auto demo = CCMenuItemSpriteExtra::create(
            demoSprite, this, menu_selector(ClickIndicatorPause::startShowcase));
        demo->setPosition({0.f, -40.f});

        menu->addChild(indicator);
        menu->addChild(demo);
        addChild(menu);
    }

    void toggleIndicator(CCObject*) {
        g_indicator = !g_indicator;
    }

    void startShowcase(CCObject*) {
        if (!PlayLayer::get() || !PlayLayer::get()->m_player1) return;
        g_showcase = true;
        g_demoTime = 0.f;
        g_demoIndex = 0;
    }
};
