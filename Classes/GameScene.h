#ifndef __GAMESCENE_H__
#define __GAMESCENE_H__

#include "cocos2d.h"

USING_NS_CC;

class GameScene : public cocos2d::Layer
{
public:

    void setPhysicsWorld(PhysicsWorld * world);
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();

    // implement the "static create()" method manually
    CREATE_FUNC(GameScene);

private:

    static const int gap = 5;  // 板到底边的距离
    static const float plateSpeed;
    static const float ballSpeed;
    static const float ballMaxAngle;
    static const float toRad;  // 角度转弧度
    static const float toAngle;  // 弧度转角度

    Size visibleSize;
    Vec2 origin;
    static PhysicsMaterial elasticMaterial;

    PhysicsWorld *m_world;

    Sprite *bottom, *plate, *ball;
    Node *brickRoot;
    Label *life, *score, *level;
    int _life, _score, _damage, _level;

    // 游戏状态
    bool pressA, pressD, pressLeft, pressRight;
    bool launched, through;

    void onKeyPressed(EventKeyboard::KeyCode, Event*);
    void onKeyReleased(EventKeyboard::KeyCode, Event*);
    bool onConcactBegan(PhysicsContact &);
    bool onContactSeparate(PhysicsContact &);

    void update(float);

    void nextLevel();
    void win();
    void lose();

    static Sprite *createBrick(const std::string &filename, int life);
};

#endif // __GAMESCENE_H__
