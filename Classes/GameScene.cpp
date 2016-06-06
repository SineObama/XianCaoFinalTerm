#include "GameScene.h"
#include "cocostudio/CocoStudio.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"
#include <cmath>

using namespace std;
using namespace cocostudio;
using namespace CocosDenshion;
using namespace cocostudio::timeline;

USING_NS_CC;

const float GameScene::plateSpeed = 300;
const float GameScene::ballSpeed = GameScene::plateSpeed * 1.0f;
const float GameScene::ballMaxAngle = 60;
const float GameScene::toRad = 3.141592f / 180;
const float GameScene::toAngle = 180 / 3.1416f;
PhysicsMaterial GameScene::elasticMaterial(0, 1, 0);

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

bool GameScene::init()
{
    if (!Layer::init())
        return false;

    visibleSize = Director::getInstance()->getVisibleSize();
    origin = Director::getInstance()->getVisibleOrigin();

    // 预载音乐
    SimpleAudioEngine::getInstance()->preloadBackgroundMusic("music/bgm.mp3");
    //SimpleAudioEngine::getInstance()->preloadEffect("music/meet_stone.wav");

    // 循环播放背景音乐
    SimpleAudioEngine::getInstance()->playBackgroundMusic("music/bgm.mp3", true);

    // 设置背景图片
    auto bgsprite = Sprite::create("bg.jpg");
    bgsprite->setPosition(visibleSize / 2);
    bgsprite->setScale(visibleSize.width / bgsprite->getContentSize().width, visibleSize.height / bgsprite->getContentSize().height);
    this->addChild(bgsprite, 0);

    // todo 可以考虑使用粒子背景
    /*auto ps = ParticleSystemQuad::create("black_hole.plist");
    ps->setPosition(visibleSize / 2);
    this->addChild(ps);*/

    // 添加边界
    auto bound = Sprite::create();
    bound->setName("bound");
    bound->setPosition(Point(visibleSize.width / 2, visibleSize.height / 2));
    auto boundBody = PhysicsBody::createEdgeBox(visibleSize, elasticMaterial);
    boundBody->setDynamic(false);
    bound->setPhysicsBody(boundBody);
    this->addChild(bound);

    // 添加底部边界，用于检测落地
    bottom = Sprite::create();
    bottom->setName("bottom");
    bottom->setPosition(Point(visibleSize.width / 2, gap / 2));
    auto bottomBody = PhysicsBody::createBox(Size(visibleSize.width, gap));
    bottomBody->setDynamic(false);
    bottomBody->setCategoryBitmask(1);
    bottomBody->setCollisionBitmask(1);
    bottomBody->setContactTestBitmask(1);
    bottom->setPhysicsBody(bottomBody);
    this->addChild(bottom);

    // 设置滑板
    plate = Sprite::create("plate.png");
    plate->setName("plate");
    auto plateBody = PhysicsBody::createBox(plate->getContentSize(), elasticMaterial);
    plateBody->setDynamic(false);
    plateBody->setCategoryBitmask(7);
    plateBody->setCollisionBitmask(1);
    plateBody->setContactTestBitmask(1);
    plateBody->setAngularVelocityLimit(0);
    plate->setPhysicsBody(plateBody);
    addChild(plate, 1);

    // 设置球
    ball = Sprite::create("ball.png");
    ball->setName("ball");
    auto ballBody = PhysicsBody::createCircle(13, elasticMaterial);
    ballBody->setCategoryBitmask(1);
    ballBody->setCollisionBitmask(1);
    ballBody->setContactTestBitmask(1);
    ballBody->setAngularVelocityLimit(0);
    ball->setPhysicsBody(ballBody);
    addChild(ball, 1);

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
    _eventDispatcher->addEventListenerWithFixedPriority(contactListener, 1);

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
    case cocos2d::EventKeyboard::KeyCode::KEY_B:
        nextLevel();
        break;
    case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:
        // 空格发射
        if (!launched) {
            ball->getPhysicsBody()->setVelocity(Vec2(1, 0).rotateByAngle(Vec2(), random(90 - ballMaxAngle, 90 + ballMaxAngle) * toRad) * ballSpeed);
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
    Node *other;
    // 碰撞体其一是球，可能是碰地，碰砖或者碰板
    if (A == ball || B == ball) {
        other = A == ball ? B : A;
        auto name = other->getName();
        if (name == "bottom") {  // 碰地
            ball->getPhysicsBody()->setVelocity(Vec2());

            if (_life <= 0)
                lose();
            else {
                launched = false;
                // 扣血
                _life--;
                char s[10] = {};
                sprintf(s, "%d", _life);
                life->setString(s);
            }
        }
        else if (name == "brick") {  // 碰砖
            ComAttribute *com = dynamic_cast<ComAttribute *>(other->getComponent("brick"));
            int life = com->getInt("life");
            life -= _damage;
            if (life <= 0) {
                // 加分
                _score += com->getInt("score");
                char s[10] = {};
                sprintf(s, "%d", _score);
                score->setString(s);

                other->removeFromParentAndCleanup(1);
                if (brickRoot->getChildrenCount() == 0)
                    nextLevel();
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
            if (ball->getPositionX() <= plate->getPositionX() + plate->getContentSize().width / 2 && ball->getPositionX() >= plate->getPositionX() - plate->getContentSize().width / 2) {
                float degree = (pos * ballMaxAngle * 2 + 90) * toRad;
                Vec2 v = Vec2(1, 0).rotateByAngle(Vec2(0, 0), degree) * ballSpeed;
                ball->getPhysicsBody()->setVelocity(v);
            }
        }
    }
    // 碰撞体其一是滑板，有可能是碰道具
    else if (A == plate || B == plate) {
        other = A == plate ? B : A;
        auto name = other->getName();
        // todo 碰道具
    }
    // 碰撞体其一是底部，只剩下道具的情况
    else if (A == bottom || B == bottom) {
        other = A == bottom ? B : A;
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
    if (A == ball || B == ball) {
        auto v = ball->getPhysicsBody()->getVelocity();
        int x = (v.x > 0 ? 1 : -1), y = (v.y > 0 ? 1 : -1);
        v.x *= x;
        v.y *= y;
        auto angle = v.getAngle() * toAngle;
        if (90 - angle > ballMaxAngle) {
            v = Vec2(1, 0).rotateByAngle(Vec2(), ballMaxAngle * toRad) * ballSpeed;
            v.x *= x;
            v.y *= y;
            ball->getPhysicsBody()->setVelocity(v);
        }
    }
    return true;
}

void GameScene::update(float time) {
    // 实现板的移动
    int direction = 0;
    if (pressA | pressLeft)
        direction--;
    if (pressD | pressRight)
        direction++;
    if (direction) {
        float newX = plate->getPositionX() + direction * plateSpeed * time;
        float halfWidth = plate->getContentSize().width / 2;
        if (newX > visibleSize.width - halfWidth)
            newX = visibleSize.width - halfWidth;
        else if (newX < halfWidth)
            newX = halfWidth;
        plate->setPositionX(newX);
    }

    // 实现未发射时球随板移动
    if (!launched)
        ball->setPosition(plate->getPositionX(), plate->getPositionY() + plate->getContentSize().height / 2 + ball->getContentSize().height / 2);
}

void GameScene::nextLevel() {
    // 定义砖块布局。0没有，1普通砖，2打两次才消灭的，9打不烂的
    static const int maxLevel = 2, col = 13, row = 23;
    static const int map[maxLevel + 10][row][col] = {
        {},
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
    };

    brickRoot->removeAllChildren();
    _level++;
    if (_level > maxLevel)
        win();
    else {
        char s[10] = {};
        sprintf(s, "%d", _level);
        level->setString(s);
        pressA = pressD = pressLeft = pressRight = false;
        launched = false;
        through = false;
        ball->getPhysicsBody()->setVelocity(Vec2());
        ball->setPosition(plate->getPositionX(), plate->getPositionY() + plate->getContentSize().height / 2 + ball->getContentSize().height / 2);
        plate->setPosition(visibleSize.width / 2, gap + plate->getContentSize().height / 2);
        int brickWidth = 70, brickHeight = 21;
        float sx = visibleSize.width / 2 - brickWidth * (col - 1) / 2, sy = 550;
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                Sprite *brick;
                switch (map[_level][i][j]) {
                case 0: continue;
                case 1: brick = createBrick("brick.png", 1); break;
                case 2: brick = createBrick("brick2.png", 2); break;
                case 9: brick = createBrick("brick999.png", 1 << 30); break;
                }
                brick->setPosition(sx + j * brickWidth, sy - i * brickHeight);
                brickRoot->addChild(brick, 1);
            }
        }
        // 方法二：每关用特定规则创建
        //switch (_level)
        //{
        //case 1:
        //    for (int i = 0; i < row; i++) {
        //        for (int j = 0; j < col; j++) {
        //            if (i + j < (row + 1) / 2 || i + j >(row + 1) / 2 + col - 3)
        //                continue;
        //            auto brick = createBrick("brick.png", 1);
        //            brick->setPosition(sx + j * brickWidth, sy - i * brickHeight);
        //            brickRoot->addChild(brick, 1);
        //        }
        //    }
        //    break;
        //case 2:
        //    for (int i = 0; i < row; i++) {
        //        for (int j = 0; j < col; j++) {
        //            if ((i * col + j) % 2)
        //                continue;
        //            auto brick = createBrick("brick2.png", 2);
        //            brick->setPosition(sx + j * brickWidth, sy - i * brickHeight);
        //            brickRoot->addChild(brick, 1);
        //        }
        //    }
        //    break;
        //case 3:
        //    break;
        //case 4:
        //    break;
        //case 5:
        //    break;
        //}
    }
}

void GameScene::win() {

}

void GameScene::lose() {

}

Sprite *GameScene::createBrick(const std::string &filename, int life) {
    auto brick = Sprite::create(filename);
    brick->setName("brick");
    auto brickBody = PhysicsBody::createBox(brick->getContentSize(), elasticMaterial);
    brickBody->setDynamic(false);
    brickBody->setCategoryBitmask(1);
    brickBody->setCollisionBitmask(1);
    brickBody->setContactTestBitmask(1);
    brickBody->setAngularVelocityLimit(0);
    brick->setPhysicsBody(brickBody);
    auto attr = ComAttribute::create();
    attr->setName("brick");
    attr->setInt("life", life);
    attr->setInt("score", life * 10);
    brick->addComponent(attr);
    return brick;
}
