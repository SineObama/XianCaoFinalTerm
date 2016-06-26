#ifndef __GAMESCENE_H__
#define __GAMESCENE_H__

#include "cocos2d.h"

USING_NS_CC;

class GameScene : public cocos2d::Layer
{
public:

    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();

    // implement the "static create()" method manually
    CREATE_FUNC(GameScene);

    static int totalScore;

private:

    static const int gap = 5;  // °åµ½µ×±ßµÄ¾àÀë
    static const float plateSpeed;
    static const Size ballSize;
    static const float ballBaseSpeed;
    static const float ballSpeedGrowth;
    static const float ballMaxAngle;
    static const Size toolSize;
    static const float toolBaseSpeed;
    static const float toolSpeedFluctuation;
    static const float toolAverageRefreshTime;
    static const float throughDuration;
    static const int brickWidth = 71, brickHeight = 21;

    static const int boundBit = 1 << 0;
    static const int bottomBit = 1 << 1;
    static const int plateBit = 1 << 2;
    static const int ballBit = 1 << 3;
    static const int brickBit = 1 << 4;
    static const int toolBit = 1 << 5;

    Size visibleSize;
    Vec2 origin;
    static PhysicsMaterial elasticMaterial;

    PhysicsWorld *physicsWorld;

    Sprite *bottom, *plate;
    Node *ballRoot, *brickRoot;
    Label *life, *score, *level;
    int _life, _score, _damage, _level;
    float ballSpeed;

    // ÓÎÏ·×´Ì¬
    bool pressA, pressD, pressLeft, pressRight;
    bool launched, through;

    void onKeyPressed(EventKeyboard::KeyCode, Event*);
    void onKeyReleased(EventKeyboard::KeyCode, Event*);
    bool onConcactBegan(PhysicsContact &);
    bool onContactSeparate(PhysicsContact &);

    void update(float);
    void randomCreateTools(float);
    void endThrough(float);

    void scheduleDivide(float);
    void checkBrick(float);
    void nextLevel();
    void win();
    void lose();

    static Sprite *createBrick(const std::string &filename, int life, const Color3B &);
    static Sprite *createBall();
    static void setLabel(Label *, int);
};

#endif // __GAMESCENE_H__
