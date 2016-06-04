#ifndef __GAMESCENE_H__
#define __GAMESCENE_H__

#include "cocos2d.h"
#include "TagDictionary.h"

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

    Size visibleSize;
    Vec2 origin;
    PhysicsMaterial elasticMaterial;

    TagDictionary *dic;
    PhysicsWorld *m_world;

    Sprite *plate, *ball;
    Vector<Sprite*> bricks;
    Label *life, *score;
    int _life, _score;

    bool pressA, pressD, pressLeft, pressRight;
    bool playing;

    void onKeyPressed(EventKeyboard::KeyCode, Event*);
    void onKeyReleased(EventKeyboard::KeyCode, Event*);
    bool onConcactBegan(PhysicsContact &);

    void update(float);

    void die();
    void lose();
};

#endif // __GAMESCENE_H__
