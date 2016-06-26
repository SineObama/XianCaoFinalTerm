#include "WinScene.h"
#include "MenuScene.h"
#include "GameScene.h"
USING_NS_CC;

Scene* WinScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();

    // 'layer' is an autorelease object
    auto layer = WinScene::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool WinScene::init()
{

    if (!Layer::init())
    {
        return false;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto winbp = Sprite::create("bg/winbp.jpg");
    winbp->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(winbp, 0);

    auto title = Label::createWithTTF("You win. Congratulations!", "fonts/Marker Felt.ttf", 80);
    title->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y + 100));
    this->addChild(title, 1);

    auto Restart = Label::createWithTTF("Try again", "fonts/Marker Felt.ttf", 80);

    auto RestartItem = MenuItemLabel::create(Restart, CC_CALLBACK_1(WinScene::ChangeScene, this));
    RestartItem->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    this->addChild(RestartItem, 1);

    auto RestartMenu = Menu::create(RestartItem, NULL);
    RestartMenu->setPosition(Vec2::ZERO);
    this->addChild(RestartMenu, 2);

    return true;
}

void WinScene::ChangeScene(Ref *) {
    auto menuscene = MenuScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5, menuscene, Color3B(0, 0, 0)));
}

