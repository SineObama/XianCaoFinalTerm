#include "MenuScene.h"
#include "GameScene.h"
#include "LoseScene.h"
#include "WinScene.h"
#include "cocostudio/CocoStudio.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"

using namespace cocostudio;
using namespace CocosDenshion;
USING_NS_CC;

Scene* MenuScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();

    // 'layer' is an autorelease object
    auto layer = MenuScene::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool MenuScene::init()
{

    if (!Layer::init())
    {
        return false;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto bp = Sprite::create("bg/menubp.jpg");
    bp->setPosition(Vec2(visibleSize.width / 2 + origin.x + 250, visibleSize.height / 2 + origin.y + 150));
    this->addChild(bp, 0);

    auto title = Label::createWithTTF("LET THE BALL FLY!", "fonts/Marker Felt.ttf", 80);
    title->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y + 200));
    this->addChild(title, 1);

    auto startItem = MenuItemImage::create("start-0.png", "start-1.png", CC_CALLBACK_1(MenuScene::ChangeScene, this));
    startItem->setPosition(Vec2(750 + origin.x, origin.y + 200));

    auto start = Menu::create(startItem, NULL);
    start->setPosition(Vec2::ZERO);
    this->addChild(start, 3);

    // 播放背景音乐
    SimpleAudioEngine::getInstance()->preloadBackgroundMusic("music/bgm.mp3");
    SimpleAudioEngine::getInstance()->playBackgroundMusic("music/bgm.mp3", true);

    return true;
}

void MenuScene::ChangeScene(Ref *) {
    auto gamescene = GameScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5, gamescene, Color3B(0, 0, 0)));
}


