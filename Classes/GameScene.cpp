#include "GameScene.h"
#include "cocostudio/CocoStudio.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"

using namespace std;
using namespace CocosDenshion;
using namespace cocostudio::timeline;

USING_NS_CC;

void GameScene::setPhysicsWorld(PhysicsWorld* world) { m_world = world; }

Scene* GameScene::createScene()
{
    auto scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
    scene->getPhysicsWorld()->setGravity(Point(0, 0));

    auto layer = GameScene::create();
    layer->setPhysicsWorld(scene->getPhysicsWorld());

    scene->addChild(layer);
    return scene;
}

// on "init" you need to initialize your instance
bool GameScene::init()
{
    if (!Layer::init())
        return false;

    visibleSize = Director::getInstance()->getVisibleSize();
    origin = Director::getInstance()->getVisibleOrigin();
    material.restitution = 1;
    material.friction = 0;

    dic = TagDictionary::getInstance();
    dic->add("ball");
    dic->add("plate");
    dic->add("edge");
    dic->add("brick");

    preloadMusic();
    playBgm();

    addBackground();
    addEdge();
    addPlate();
    addBall();

    addKeyboardListener();
    addContactListener();

    _life = 3;
    _score = 0;

    pressA = pressD = pressLeft = pressRight = false;
    playing = false;

    scheduleUpdate();
    return true;
}

void GameScene::preloadMusic() {
    SimpleAudioEngine::getInstance()->preloadBackgroundMusic("music/bgm.mp3");
    //SimpleAudioEngine::getInstance()->preloadEffect("music/meet_stone.wav");
}

void GameScene::playBgm() {
    SimpleAudioEngine::getInstance()->playBackgroundMusic("music/bgm.mp3", true);
}

void GameScene::addBackground() {
    auto bgsprite = Sprite::create("bg.jpg");
    bgsprite->setPosition(visibleSize / 2);
    bgsprite->setScale(visibleSize.width / bgsprite->getContentSize().width, visibleSize.height / bgsprite->getContentSize().height);
    this->addChild(bgsprite, 0);

    /*auto ps = ParticleSystemQuad::create("black_hole.plist");
    ps->setPosition(visibleSize / 2);
    this->addChild(ps);*/
}

void GameScene::addEdge() {
    auto bound = Sprite::create();
    auto boundBody = PhysicsBody::createEdgeBox(visibleSize, material);
    boundBody->setDynamic(false);
    boundBody->setTag(dic->get("edge"));
    bound->setPhysicsBody(boundBody);
    bound->setPosition(Point(visibleSize.width / 2, visibleSize.height / 2));
    this->addChild(bound);

    // ÓÃÓÚ¼ì²âÇòÂäµØ
    auto bottom = Sprite::create();
    auto bottomBody = PhysicsBody::createBox(Size(visibleSize.width, gap));
    bottomBody->setDynamic(false);
    bottomBody->setTag(dic->get("bottom"));
    bottom->setPhysicsBody(bottomBody);
    bottom->setPosition(Point(visibleSize.width / 2, gap / 2));
    this->addChild(bottom);
}

void GameScene::addPlate() {
    plate = Sprite::create("plate.png");
    plate->setAnchorPoint(Vec2(0.5, 0.5));
    auto plateBody = PhysicsBody::createBox(Size(100, 23), material);
    plateBody->setTag(dic->get("plate"));
    plateBody->setDynamic(false);
    plateBody->setAngularVelocityLimit(0);
    plate->setPhysicsBody(plateBody);
    plate->setPosition(visibleSize.width / 2, gap + plate->getContentSize().height / 2);
    addChild(plate, 1);
}

void GameScene::addBall() {
    ball = Sprite::create("ball.png");
    ball->setAnchorPoint(Vec2(0.5, 0.5));
    auto ballBody = PhysicsBody::createCircle(13, material);
    ballBody->setTag(dic->get("ball"));
    ballBody->setAngularVelocityLimit(0);
    ball->setPhysicsBody(ballBody);
    ball->setPosition(visibleSize.width / 2, plate->getPositionY() + plate->getContentSize().height / 2 + ball->getContentSize().height / 2);
    addChild(ball, 1);
}

void GameScene::addKeyboardListener() {
    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = CC_CALLBACK_2(GameScene::onKeyPressed, this);
    keyboardListener->onKeyReleased = CC_CALLBACK_2(GameScene::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);
}

void GameScene::addContactListener() {
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(GameScene::onConcactBegan, this);
    _eventDispatcher->addEventListenerWithFixedPriority(contactListener, 1);
}

void GameScene::onKeyPressed(EventKeyboard::KeyCode code, Event* event) {
    switch (code)
    {
    case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
        pressLeft = true;
        break;
    case cocos2d::EventKeyboard::KeyCode::KEY_A:
        pressA = true;
        break;
    case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
        pressRight = true;
        break;
    case cocos2d::EventKeyboard::KeyCode::KEY_D:
        pressD = true;
        break;
    case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:
        if (!playing) {
            ball->getPhysicsBody()->setVelocity(Vec2(-1, 0).rotateByAngle(Vec2(0, 0), random(45, 135)) * plateSpeed * 1.414f);
            playing = true;
        }
        break;
    }
}

void GameScene::onKeyReleased(EventKeyboard::KeyCode code, Event* event) {
    switch (code)
    {
    case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
        pressLeft = false;
        break;
    case cocos2d::EventKeyboard::KeyCode::KEY_A:
        pressA = false;
        break;
    case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
        pressRight = false;
        break;
    case cocos2d::EventKeyboard::KeyCode::KEY_D:
        pressD = false;
        break;
    }
}

bool GameScene::onConcactBegan(PhysicsContact& contact) {
    auto A = contact.getShapeA()->getBody()->getNode();
    auto B = contact.getShapeB()->getBody()->getNode();
    if (A == NULL || B == NULL)
        return true;
    Node *other;
    if (A == plate || B == plate) {
        other = A == plate ? B : A;
    }
    else if (A == ball || B == ball) {
        other = A == ball ? B : A;
        int tag = other->getTag();
        if (tag == dic->get("bottom"))
    }

    return true;
}

void GameScene::update(float time) {
    int direction = 0;
    if (pressA | pressLeft)
        direction--;
    if (pressD | pressRight)
        direction++;
    if (direction)
        plate->setPosition(plate->getPosition() + Vec2(direction, 0) * plateSpeed * time);
}

void GameScene::die() {
    _life--;
    if (_life)
}

void GameScene::lose() {
}
