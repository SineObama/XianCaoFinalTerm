#include "GameScene.h"
#include "LoseScene.h"
#include "WinScene.h"
#include "cocostudio/CocoStudio.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"
#include <cmath>

using namespace std;
using namespace cocostudio;
using namespace CocosDenshion;
using namespace cocostudio::timeline;

USING_NS_CC;

#define NDEBUG
//#define CANT_DIE
//#define CANT_LOSE
#define CHEAT

int GameScene::totalScore = 0;

const float GameScene::plateSpeed = 300;
const Size GameScene::ballSize(30, 30);
const float GameScene::ballBaseSpeed = 250;
const float GameScene::ballSpeedGrowth = 0.05f;
const float GameScene::ballMaxAngle = 60;
const Size GameScene::toolSize(30, 30);
const float GameScene::toolBaseSpeed = 250;
const float GameScene::toolSpeedFluctuation = 0.2;
const float GameScene::toolAverageRefreshTime = 1;
const float GameScene::throughDuration = 10;
PhysicsMaterial GameScene::elasticMaterial(0, 1, 0);

Scene* GameScene::createScene()
{
    auto scene = Scene::createWithPhysics();
#ifndef NDEBUG
    scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
#endif // !NDEBUG
    scene->getPhysicsWorld()->setGravity(Point(0, 0));

    auto layer = GameScene::create();
    layer->physicsWorld = scene->getPhysicsWorld();

    scene->addChild(layer);
    return scene;
}

bool GameScene::init()
{
    if (!Layer::init())
        return false;

    visibleSize = Director::getInstance()->getVisibleSize();
    origin = Director::getInstance()->getVisibleOrigin();

    // 预载音效
    SimpleAudioEngine::getInstance()->preloadEffect("effect/chidaoju.wav");
    SimpleAudioEngine::getInstance()->preloadEffect("effect/dead.wav");
    SimpleAudioEngine::getInstance()->preloadEffect("effect/faqiu.wav");
    SimpleAudioEngine::getInstance()->preloadEffect("effect/jizhong.wav");

    // 设置背景图片
    auto bgsprite = Sprite::create("bg/gamebp.jpg");
    bgsprite->setPosition(visibleSize / 2);
    bgsprite->setScale(visibleSize.width / bgsprite->getContentSize().width,
        visibleSize.height / bgsprite->getContentSize().height);
    this->addChild(bgsprite, 0);

    // todo 可以考虑使用粒子背景
    /*auto ps = ParticleSystemQuad::create("black_hole.plist");
    ps->setPosition(visibleSize / 2);
    this->addChild(ps);*/

    // 添加边界
    auto bound = Sprite::create();
    bound->setName("bound");
    bound->setPosition(visibleSize / 2);
    auto boundBody = PhysicsBody::createEdgeBox(visibleSize, elasticMaterial);
    boundBody->setDynamic(false);
    boundBody->setCategoryBitmask(boundBit);
    boundBody->setCollisionBitmask(ballBit);
    boundBody->setContactTestBitmask(ballBit);
    bound->setPhysicsBody(boundBody);
    this->addChild(bound);

    // 添加底部边界，用于检测落地
    bottom = Sprite::create();
    bottom->setName("bottom");
    bottom->setPosition(Point(visibleSize.width / 2, gap / 2));
    auto bottomBody = PhysicsBody::createBox(Size(visibleSize.width, gap), elasticMaterial);
    bottomBody->setDynamic(false);
    bottomBody->setCategoryBitmask(bottomBit);
    bottomBody->setCollisionBitmask(ballBit | toolBit);
    bottomBody->setContactTestBitmask(ballBit | toolBit);
    bottom->setPhysicsBody(bottomBody);
    this->addChild(bottom);

    // 设置滑板
    plate = Sprite::create("sprite/plate.jpg");
    plate->setName("plate");
    auto plateBody = PhysicsBody::createBox(plate->getContentSize(), elasticMaterial);
    plateBody->setDynamic(false);
    plateBody->setCategoryBitmask(plateBit);
    plateBody->setCollisionBitmask(ballBit | toolBit);
    plateBody->setContactTestBitmask(ballBit | toolBit);
    plate->setPhysicsBody(plateBody);
    addChild(plate, 1);

    // 设置球
    ballRoot = Node::create();
    this->addChild(ballRoot, 1);
    ballRoot->addChild(createBall());

    // 设置砖块根节点
    brickRoot = Node::create();
    this->addChild(brickRoot);

    // 添加键盘和碰撞事件
    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = CC_CALLBACK_2(GameScene::onKeyPressed, this);
    keyboardListener->onKeyReleased = CC_CALLBACK_2(GameScene::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(GameScene::onConcactBegan, this);
    contactListener->onContactSeparate = CC_CALLBACK_1(GameScene::onContactSeparate, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);

    // 显示生命和得分
    _life = 3;
    _score = 0;
    _damage = 1;
    _level = 0;
    auto levell = Label::create("level:", "fonts/arial.ttf", 18);
    auto lifel = Label::create("life:", "fonts/arial.ttf", 18);
    auto scorel = Label::create("score:", "fonts/arial.ttf", 18);
    char s[10] = {};
    sprintf(s, "%d", _life);
    level = Label::create("0", "fonts/arial.ttf", 18);
    life = Label::create(s, "fonts/arial.ttf", 18);
    score = Label::create("0", "fonts/arial.ttf", 18);
    int width = 50, height = 15;
    levell->setPosition(visibleSize - Size(width * 2, height));
    lifel->setPosition(visibleSize - Size(width * 2, height * 2));
    scorel->setPosition(visibleSize - Size(width * 2, height * 3));
    level->setPosition(visibleSize - Size(width, height));
    life->setPosition(visibleSize - Size(width, height * 2));
    score->setPosition(visibleSize - Size(width, height * 3));
    this->addChild(levell, 2);
    this->addChild(lifel, 2);
    this->addChild(scorel, 2);
    this->addChild(level, 2);
    this->addChild(life, 2);
    this->addChild(score, 2);

    nextLevel();

    scheduleUpdate();

    return true;
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
#ifdef CHEAT
    case cocos2d::EventKeyboard::KeyCode::KEY_B:// 作弊键，用于调试，进入下一关
        nextLevel();
        break;
#endif // CHEAT
    case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:
        // 空格发射
        if (!launched) {
            SimpleAudioEngine::getInstance()->playEffect("effect/faqiu.wav");
            auto rad = CC_DEGREES_TO_RADIANS(random(90 - ballMaxAngle, 90 + ballMaxAngle));  // 随机发射角度
            auto velocity = Vec2(1, 0).rotateByAngle(Vec2(), rad) * ballSpeed;
            ballRoot->getChildByName("ball")->getPhysicsBody()->setVelocity(velocity);
            schedule(schedule_selector(GameScene::randomCreateTools), 0.2f);
            launched = true;
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
    auto An = A->getName();
    auto Bn = B->getName();
    Node *ball, *other;
    // 碰撞体其一是球，可能是碰地，碰砖或者碰板
    if (An == "ball" || Bn == "ball") {
        if (An == "ball")
            ball = A, other = B;
        else
            ball = B, other = A;
        auto name = other->getName();
        if (name == "bottom") {  // 碰地
#ifndef CANT_DIE
            if (ballRoot->getChildrenCount() > 1) {
                ball->removeFromParentAndCleanup(1);
            }
            else {
                ball->getPhysicsBody()->setVelocity(Vec2());
                if (_life <= 0)
                    lose();
                else {
                    SimpleAudioEngine::getInstance()->playEffect("effect/dead.wav");
                    unschedule(schedule_selector(GameScene::randomCreateTools));
                    launched = false;
                    // 扣血
                    setLabel(life, --_life);
                }
            }
#endif // !CANT_DIE
        }
        else if (name == "brick") {  // 碰砖
            SimpleAudioEngine::getInstance()->playEffect("effect/jizhong.wav");
            ComAttribute *com = dynamic_cast<ComAttribute *>(other->getComponent("brick"));
            int life = com->getInt("life");
            life -= _damage;
            if (life <= 0) {
                // 加分
                _score += com->getInt("score");
                setLabel(score, _score);

                other->removeFromParentAndCleanup(1);
                scheduleOnce(schedule_selector(GameScene::checkBrick, this), 0);
                if (through)
                    return false;
            }
            else {
                com->setInt("life", life);
            }
        }
        else if (name == "plate") {  // 碰板
            // 根据在碰撞的位置决定发射角度
            float pos = (plate->getPositionX() - ball->getPositionX()) / plate->getContentSize().width;  // -0.5~0.5
            auto ballX = ball->getPositionX();
            auto plateX = plate->getPositionX();
            auto half = plate->getContentSize().width / 2;
            if (ballX <= plateX + half && ballX >= plateX - half) {
                float rad = CC_DEGREES_TO_RADIANS(pos * ballMaxAngle * 2 + 90);
                Vec2 v = Vec2(1, 0).rotateByAngle(Vec2(0, 0), rad) * ballSpeed;
                ball->getPhysicsBody()->setVelocity(v);
            }
        }
    }
    // 碰撞体其一是滑板，碰道具
    else if (An == "plate" || Bn == "plate") {
        other = An == "plate" ? B : A;
        auto name = other->getName();
        SimpleAudioEngine::getInstance()->playEffect("effect/chidaoju.wav");
        if (name == "addLife") {
            setLabel(life, ++_life);
        }
        else if (name == "through") {
            // 穿透实现方法：增加撞击伤害并设置击毁后直行
            unschedule(schedule_selector(GameScene::endThrough));
            through = true;
            _damage = 10;
            scheduleOnce(schedule_selector(GameScene::endThrough), throughDuration);
        }
        else if (name == "multi") {
            if (launched)
                scheduleOnce(schedule_selector(GameScene::scheduleDivide), 0);
        }
        other->removeFromParentAndCleanup(1);
    }
    // 碰撞体其一是底部，只剩下道具的情况
    else if (An == "bottom" || Bn == "bottom") {
        other = An == "bottom" ? B : A;
        other->removeFromParentAndCleanup(1);
    }
    return true;
}

// 碰撞后检查球的角度，避免近似平行的移动
bool GameScene::onContactSeparate(PhysicsContact &contact) {
    if (!launched)
        return true;
    auto A = contact.getShapeA()->getBody()->getNode();
    auto B = contact.getShapeB()->getBody()->getNode();
    if (A == NULL || B == NULL)
        return true;
    auto ball = A->getName() == "ball" ? A : B;
    if (ball->getName() == "ball") {
        auto v = ball->getPhysicsBody()->getVelocity();
        int x = (v.x > 0 ? 1 : -1), y = (v.y > 0 ? 1 : -1);
        v.x *= x;
        v.y *= y;
        auto angle = CC_RADIANS_TO_DEGREES(v.getAngle());
        if (90 - angle > ballMaxAngle) {
            auto newRad = CC_DEGREES_TO_RADIANS(angle + 5);
            v = Vec2(1, 0).rotateByAngle(Vec2(), newRad) * ballSpeed;
            v.x *= x;
            v.y *= y;
            ball->getPhysicsBody()->setVelocity(v);
        }
    }
    return true;
}

void GameScene::update(float deltaTime) {
    // 实现板的移动
    int direction = 0;
    if (pressA | pressLeft)
        direction--;
    if (pressD | pressRight)
        direction++;
    if (direction) {
        float newX = plate->getPositionX() + direction * plateSpeed * deltaTime;
        float halfWidth = plate->getContentSize().width / 2;
        if (newX > visibleSize.width - halfWidth)
            newX = visibleSize.width - halfWidth;
        else if (newX < halfWidth)
            newX = halfWidth;
        plate->setPositionX(newX);
    }

    // 实现未发射时球随板移动
    if (!launched) {
        auto ball = ballRoot->getChildByName("ball");
        ball->setPosition(plate->getPositionX(), plate->getPositionY() + plate->getContentSize().height / 2 + ball->getContentSize().height / 2);
    }
}

void GameScene::randomCreateTools(float deltaTime) {
    if (random(0.0f, toolAverageRefreshTime / deltaTime) <= 1) {
        Sprite *tool;
        switch (random(1, 3)) {
        case 1:
            tool = Sprite::create("sprite/addLife.jpg");
            tool->setName("addLife");
            break;
        case 2:
            tool = Sprite::create("sprite/through.jpg");
            tool->setName("through");
            break;
        case 3:
            tool = Sprite::create("sprite/multi.jpg");
            tool->setName("multi");
            break;
        }
        auto size = tool->getContentSize();
        tool->setScale(toolSize.width / size.width, toolSize.height / size.height);
        auto size2 = tool->getContentSize();
        tool->setPosition(random(toolSize.width / 2, visibleSize.width - toolSize.width / 2), visibleSize.height - toolSize.height / 2);
        auto toolBody = PhysicsBody::createBox(size, elasticMaterial);
        toolBody->setGroup(-1);
        toolBody->setCategoryBitmask(toolBit);
        toolBody->setCollisionBitmask(bottomBit | plateBit);
        toolBody->setContactTestBitmask(bottomBit | plateBit);
        toolBody->setVelocity(Vec2(0, -random(1.0f - toolSpeedFluctuation, 1.0f + toolSpeedFluctuation) * toolBaseSpeed));
        tool->setPhysicsBody(toolBody);
        this->addChild(tool, 2);
    }
}

void GameScene::endThrough(float) {
    through = false;
    _damage = 1;
}

void GameScene::scheduleDivide(float) {
    auto ball = createBall();
    auto sball = ballRoot->getChildByName("ball");
    ball->setPosition(sball->getPosition());
    auto vec = sball->getPhysicsBody()->getVelocity();
    vec.x *= -1;
    ball->getPhysicsBody()->setVelocity(vec);
    ballRoot->addChild(ball);
}

void GameScene::checkBrick(float) {
    auto vec = brickRoot->getChildren();
    auto it = vec.begin();
    bool pass = true;
    while (it != vec.end()) {
        if ((*it)->getTag() != 1) {
            pass = false;
            break;
        }
        it++;
    }
    if (pass)
        nextLevel();
}

void GameScene::nextLevel() {
    // 定义砖块布局。0没有，1普通砖，2打两次才消灭的，9打不烂的
    static const int maxLevel = 6, col = 13, row = 23;
    static const int map[maxLevel + 10][row][col] = {
        {},
        // 1
        {
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,1,0,0,0,0,0,0 },
            { 0,0,0,0,0,1,1,1,0,0,0,0,0 },
            { 0,0,0,0,1,1,1,1,1,0,0,0,0 },
            { 0,0,0,1,1,1,1,1,1,1,0,0,0 },
            { 0,0,1,1,1,1,1,1,1,1,1,0,0 },
            { 0,1,1,1,1,1,1,1,1,1,1,1,0 },
            { 0,0,1,1,1,1,1,1,1,1,1,0,0 },
            { 0,0,0,1,1,1,1,1,1,1,0,0,0 },
            { 0,0,0,0,1,1,1,1,1,0,0,0,0 },
            { 0,0,0,0,0,1,1,1,0,0,0,0,0 },
            { 0,0,0,0,0,0,1,0,0,0,0,0,0 },
        },
        // 2
        {
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,1,0,0,0,0,0,1,0,0,0 },
            { 0,0,1,0,1,0,0,0,1,0,1,0,0 },
            { 0,1,0,0,0,1,0,1,0,0,0,1,0 },
            { 1,0,0,0,0,0,1,0,0,0,0,0,1 },
            { 0,0,0,2,0,0,0,0,0,2,0,0,0 },
            { 0,0,2,0,2,0,0,0,2,0,2,0,0 },
            { 0,2,0,0,0,2,0,2,0,0,0,2,0 },
            { 2,0,0,0,0,0,2,0,0,0,0,0,2 },
            { 0,0,0,1,0,0,0,0,0,1,0,0,0 },
            { 0,0,1,0,1,0,0,0,1,0,1,0,0 },
            { 0,1,0,0,0,1,0,1,0,0,0,1,0 },
            { 1,0,0,0,0,0,1,0,0,0,0,0,1 },
        },
        // 3
        {
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 1,0,0,0,2,0,0,0,1,0,0,0,1 },
            { 0,2,0,1,0,0,0,0,0,2,0,2,0 },
            { 0,0,1,0,0,0,0,0,0,0,1,0,0 },
            { 0,2,0,2,0,0,0,0,0,2,0,2,0 },
            { 1,0,0,0,1,0,0,0,1,0,0,0,1 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 9,9,9,9,9,0,0,0,9,9,9,9,9 },
        },
        // 4
        {
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,9,1,1,1,1,1,1,1,1,1,9,0 },
            { 0,9,0,0,0,0,0,0,0,0,0,9,0 },
            { 0,9,2,2,2,2,2,2,2,2,2,9,0 },
            { 0,9,0,0,0,0,0,0,0,0,0,9,0 },
            { 0,9,1,1,1,1,1,1,1,1,1,9,0 },
            { 0,9,0,0,0,0,0,0,0,0,0,9,0 },
            { 2,9,0,0,0,0,0,0,0,0,0,9,2 },
            { 0,0,9,0,0,0,0,0,0,0,9,0,0 },
            { 0,0,0,9,0,0,0,0,0,9,0,0,0 },
            { 0,0,0,0,9,0,0,0,9,0,0,0,0 },
            { 0,0,0,0,0,9,2,9,0,0,0,0,0 },
            { 0,0,0,0,0,0,9,0,0,0,0,0,0 },
        },
        // 5
        {
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 0,0,0,0,0,0,0,0,0,0,0,0,0 },
            { 2,0,2,0,2,0,2,0,2,0,2,0,2 },
            { 0,2,0,2,0,2,0,2,0,2,0,2,0 },
            { 2,0,2,0,2,0,2,0,2,0,2,0,2 },
            { 0,2,0,2,0,2,0,2,0,2,0,2,0 },
            { 2,0,2,0,2,0,2,0,2,0,2,0,2 },
            { 0,2,0,2,0,2,0,2,0,2,0,2,0 },
            { 2,0,2,0,2,0,2,0,2,0,2,0,2 },
        },
        // 6
        {
            { 9,9,9,9,9,9,9,9,9,9,9,9,9 },
            { 9,1,1,1,1,1,1,1,1,1,1,1,9 },
            { 9,1,1,1,1,1,1,1,1,1,1,1,9 },
            { 9,1,1,1,1,1,1,1,1,1,1,1,9 },
            { 9,1,1,1,1,1,1,1,1,1,1,1,9 },
            { 9,1,1,1,1,1,1,1,1,1,1,1,9 },
            { 9,1,1,1,1,1,1,1,1,1,1,1,9 },
            { 9,1,1,1,1,1,1,1,1,1,1,1,9 },
            { 9,1,1,1,1,1,1,1,1,1,1,1,9 },
            { 9,1,1,1,1,1,1,1,1,1,1,1,9 },
            { 9,1,1,1,1,1,1,1,1,1,1,1,9 },
            { 9,1,1,1,1,1,1,1,1,1,1,1,9 },
            { 9,1,1,1,1,1,1,1,1,1,1,1,9 },
            { 9,1,1,1,1,1,1,1,1,1,1,1,9 },
            { 9,1,1,1,1,1,1,1,1,1,1,1,9 },
            { 9,9,9,9,9,9,0,9,9,9,9,9,9 },
        },
    };

    brickRoot->removeAllChildren();
    _level++;
    if (_level > maxLevel)
        win();
    else {
        unschedule(schedule_selector(GameScene::randomCreateTools));
        unschedule(schedule_selector(GameScene::endThrough));
        setLabel(level, _level);
        pressA = pressD = pressLeft = pressRight = false;
        _damage = 1;
        launched = false;
        through = false;
        ballSpeed = ballBaseSpeed * (1 + ballSpeedGrowth * _level);
        // 删除多余的球
        auto v = ballRoot->getChildren();
        while (v.size() > 1) {
            v.at(1)->removeFromParentAndCleanup(1);
            v.erase(1);
        }
        v.at(0)->getPhysicsBody()->setVelocity(Vec2());
        plate->setPosition(visibleSize.width / 2, gap + plate->getContentSize().height / 2);
        static const float sx = visibleSize.width / 2 - brickWidth * (col - 1) / 2, sy = 550;
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                Sprite *brick;
                switch (map[_level][i][j]) {
                case 0: continue;
                case 1: brick = createBrick("sprite/brick.png", 1, Color3B(255, 255, 255)); break;
                case 2: brick = createBrick("sprite/brick.png", 2, Color3B(255, 0, 255)); break;
                case 9:
                    brick = createBrick("sprite/stonebrick.png", 1 << 30, Color3B(255, 255, 255));
                    brick->setTag(1);  // 表示不能被消灭，用于检测过关
                    break;
                }
                brick->setPosition(sx + j * brickWidth, sy - i * brickHeight);
                brickRoot->addChild(brick, 1);
            }
        }
    }
}

void GameScene::win() {
    auto gamescene = WinScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5, gamescene, Color3B(0, 0, 0)));
}

void GameScene::lose() {
#ifndef CANT_LOSE
    totalScore = _score;
    auto gamescene = LoseScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5, gamescene, Color3B(0, 0, 0)));
#endif // !CANT_LOSE
}

Sprite *GameScene::createBrick(const std::string &filename, int life, const Color3B &color) {
    auto brick = Sprite::create(filename);
    brick->setName("brick");
    brick->setColor(color);
    brick->setScale(brickWidth / brick->getContentSize().width, brickHeight / brick->getContentSize().height);
    auto brickBody = PhysicsBody::createBox(brick->getContentSize(), elasticMaterial);
    brickBody->setDynamic(false);
    brickBody->setCategoryBitmask(brickBit);
    brickBody->setCollisionBitmask(ballBit);
    brickBody->setContactTestBitmask(ballBit);
    brickBody->setAngularVelocityLimit(0);
    brick->setPhysicsBody(brickBody);
    auto attr = ComAttribute::create();
    attr->setName("brick");
    attr->setInt("life", life);
    attr->setInt("score", life * 10);
    brick->addComponent(attr);
    return brick;
}

Sprite *GameScene::createBall() {
    auto ball = Sprite::create("sprite/ball.jpg");
    ball->setName("ball");
    ball->setScale(ballSize.width / ball->getContentSize().width, ballSize.height / ball->getContentSize().height);
    auto ballBody = PhysicsBody::createCircle(13, elasticMaterial);
    ballBody->setCategoryBitmask(ballBit);
    ballBody->setCollisionBitmask(0xffffffff & ~toolBit & ~ballBit);
    ballBody->setContactTestBitmask(0xffffffff & ~toolBit & ~ballBit);
    ballBody->setAngularVelocityLimit(0);
    ball->setPhysicsBody(ballBody);
    return ball;
}

void GameScene::setLabel(Label *label, int num) {
    char s[10] = {};
    sprintf(s, "%d", num);
    label->setString(s);
}
