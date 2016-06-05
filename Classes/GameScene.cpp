#include "GameScene.h"
#include "cocostudio/CocoStudio.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"

using namespace std;
using namespace CocosDenshion;
using namespace cocostudio::timeline;

USING_NS_CC;

void GameScene::setPhysicsWorld(PhysicsWorld* world) { m_world = world; }

const float GameScene::plateSpeed = 300;
const float GameScene::ballSpeed = GameScene::plateSpeed * 1.0f;
const float GameScene::ballMaxAngle = 60;
const float GameScene::toRad = 3.1416f / 180;

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
    elasticMaterial.restitution = 1;
    elasticMaterial.friction = 0;

    dic = TagDictionary::getInstance();
    dic->add("ball");
    dic->add("plate");
    dic->add("edge");
    dic->add("brick");
    dic->add("bottom");
    //dic->add("")

    // Ԥ������
    SimpleAudioEngine::getInstance()->preloadBackgroundMusic("music/bgm.mp3");
    //SimpleAudioEngine::getInstance()->preloadEffect("music/meet_stone.wav");

    // ѭ�����ű�������
    SimpleAudioEngine::getInstance()->playBackgroundMusic("music/bgm.mp3", true);

    // ���ñ���ͼƬ
    auto bgsprite = Sprite::create("bg.jpg");
    bgsprite->setPosition(visibleSize / 2);
    bgsprite->setScale(visibleSize.width / bgsprite->getContentSize().width, visibleSize.height / bgsprite->getContentSize().height);
    this->addChild(bgsprite, 0);

    // todo ���Կ���ʹ�����ӱ���
    /*auto ps = ParticleSystemQuad::create("black_hole.plist");
    ps->setPosition(visibleSize / 2);
    this->addChild(ps);*/

    // ��ӱ߽�
    auto bound = Sprite::create();
    bound->setTag(dic->get("edge"));
    bound->setPosition(Point(visibleSize.width / 2, visibleSize.height / 2));
    auto boundBody = PhysicsBody::createEdgeBox(visibleSize, elasticMaterial);
    boundBody->setDynamic(false);
    bound->setPhysicsBody(boundBody);
    this->addChild(bound);

    // ��ӵײ��߽磬���ڼ�������
    auto bottom = Sprite::create();
    bottom->setTag(dic->get("bottom"));
    bottom->setPosition(Point(visibleSize.width / 2, gap / 2));
    auto bottomBody = PhysicsBody::createBox(Size(visibleSize.width, gap));
    bottomBody->setDynamic(false);
    bottomBody->setCategoryBitmask(1);
    bottomBody->setCollisionBitmask(1);
    bottomBody->setContactTestBitmask(1);
    bottom->setPhysicsBody(bottomBody);
    this->addChild(bottom);

    // ���û���
    plate = Sprite::create("plate.png");
    plate->setTag(dic->get("plate"));
    plate->setPosition(visibleSize.width / 2, gap + plate->getContentSize().height / 2);
    auto plateBody = PhysicsBody::createBox(Size(100, 23), elasticMaterial);
    plateBody->setDynamic(false);
    plateBody->setCategoryBitmask(1);
    plateBody->setCollisionBitmask(1);
    plateBody->setContactTestBitmask(1);
    plateBody->setAngularVelocityLimit(0);
    plate->setPhysicsBody(plateBody);
    addChild(plate, 1);

    // ������
    ball = Sprite::create("ball.png");
    ball->setTag(dic->get("ball"));
    ball->setPosition(plate->getPositionX(), plate->getPositionY() + plate->getContentSize().height / 2 + ball->getContentSize().height / 2);
    auto ballBody = PhysicsBody::createCircle(13, elasticMaterial);
    ballBody->setCategoryBitmask(1);
    ballBody->setCollisionBitmask(1);
    ballBody->setContactTestBitmask(1);
    ballBody->setAngularVelocityLimit(0);
    ball->setPhysicsBody(ballBody);
    addChild(ball, 1);

    // ����ש��
    int brickWidth = 70, brickHeight = 21;
    int m = visibleSize.width / brickWidth, n = visibleSize.height / 2 / brickHeight;
    float sx = visibleSize.width / 2 - brickWidth * (m - 1) / 2, sy = 250;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            auto brick = Sprite::create("brick.png");
            brick->setTag(dic->get("brick"));
            brick->setPosition(sx + j * brickWidth, sy + i * brickHeight);
            auto brickBody = PhysicsBody::createBox(Size(brickWidth, brickHeight), elasticMaterial);
            brickBody->setDynamic(false);
            brickBody->setCategoryBitmask(1);
            brickBody->setCollisionBitmask(1);
            brickBody->setContactTestBitmask(1);
            brickBody->setAngularVelocityLimit(0);
            brick->setPhysicsBody(brickBody);
            bricks.insert(bricks.size(), brick);
            this->addChild(brick, 1);
        }
    }

    // ��Ӽ��̺���ײ�¼�
    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = CC_CALLBACK_2(GameScene::onKeyPressed, this);
    keyboardListener->onKeyReleased = CC_CALLBACK_2(GameScene::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(GameScene::onConcactBegan, this);
    _eventDispatcher->addEventListenerWithFixedPriority(contactListener, 1);

    // ��ʾ�����͵÷�
    _life = 3;
    _score = 0;
    auto lifel = Label::create("life:", "fonts/arial.ttf", 18);
    auto scorel = Label::create("score:", "fonts/arial.ttf", 18);
    char tem[10] = {};
    sprintf(tem, "%d", _life);
    life = Label::create(tem, "fonts/arial.ttf", 18);
    score = Label::create("0", "fonts/arial.ttf", 18);
    int width = 50, height = 15;
    lifel->setPosition(visibleSize - Size(width * 2, height));
    scorel->setPosition(visibleSize - Size(width * 2, height * 2));
    life->setPosition(visibleSize - Size(width, height));
    score->setPosition(visibleSize - Size(width, height * 2));
    this->addChild(lifel, 2);
    this->addChild(scorel, 2);
    this->addChild(life, 2);
    this->addChild(score, 2);

    pressA = pressD = pressLeft = pressRight = false;
    playing = false;

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
    case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:
        if (!playing) {
            ball->getPhysicsBody()->setVelocity(Vec2(1, 0).rotateByAngle(Vec2(), random(90 - ballMaxAngle, 90 + ballMaxAngle) * toRad) * ballSpeed);
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
    static int count = 0;
    count++;
    // ��ײ����һ���򣬿���������,��ש��������
    if (A == ball || B == ball) {
        other = A == ball ? B : A;
        int tag = other->getTag();
        if (tag == dic->get("bottom")) {  // ����
            ball->getPhysicsBody()->setVelocity(Vec2());
            playing = false;
            die();
        }
        else if (tag == dic->get("brick")) {  // ��ש
            bricks.eraseObject(other);
            other->removeFromParentAndCleanup(1);
            _score += 10;
            refreshScore();
        }
        else if (tag == dic->get("plate")) {  // ���壬��������ײ��λ�þ�������Ƕ�
            float pos = (plate->getPositionX() - ball->getPositionX()) / plate->getContentSize().width;  // -0.5~0.5
            if (ball->getPositionX() <= plate->getPositionX() + plate->getContentSize().width / 2 && ball->getPositionX() >= plate->getPositionX() - plate->getContentSize().width / 2)
                ball->getPhysicsBody()->setVelocity(Vec2(1, 0).rotateByAngle(Vec2(0, 0), (pos * ballMaxAngle * 2 + 90) * toRad) * ballSpeed);
        }
    }
    // ��ײ����һ�ǻ��壬�п�����������
    else if (A == plate || B == plate) {
        other = A == plate ? B : A;
        int tag = other->getTag();
        // todo ������
    }
    return true;
}

void GameScene::update(float time) {
    int direction = 0;
    if (pressA | pressLeft)
        direction--;
    if (pressD | pressRight)
        direction++;
    if (direction) {
        Vec2 newPos = plate->getPosition() + Vec2(direction, 0) * plateSpeed * time;
        float halfWidth = plate->getContentSize().width / 2;
        if (newPos.x > visibleSize.width - halfWidth)
            newPos.x = visibleSize.width - halfWidth;
        else if (newPos.x < halfWidth)
            newPos.x = halfWidth;
        plate->setPosition(newPos);
    }
    if (!playing)
        ball->setPosition(plate->getPositionX(), plate->getPositionY() + plate->getContentSize().height / 2 + ball->getContentSize().height / 2);
}

void GameScene::refreshScore() {
    char s[10] = {};
    sprintf(s, "%d", _score);
    score->setString(s);
}

void GameScene::die() {
    _life--;
    char s[10] = {};
    sprintf(s, "%d", _life);
    life->setString(s);
    if (!_life)
        lose();
}

void GameScene::lose() {

}
