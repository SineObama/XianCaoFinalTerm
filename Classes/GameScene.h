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
    Size visibleSize;
    Vec2 origin;
    PhysicsMaterial material;

    TagDictionary *dic;
    PhysicsWorld* m_world;

    Sprite *ball, *plate;
    Vector<Sprite*> bricks;
    Label *life, *score;
    int _life, _score;

    bool pressA, pressD, pressLeft, pressRight;
    bool playing;

    void preloadMusic();
    void playBgm();

    void addBackground();
    void addEdge();
    void addPlate();
    void addBall();

    void addKeyboardListener();
    void addContactListener();

    void onKeyPressed(EventKeyboard::KeyCode code, Event* event);
    void onKeyReleased(EventKeyboard::KeyCode code, Event* event);
    bool onConcactBegan(PhysicsContact& contact);

    static const int gap = 10;  // °åµ½µ×±ßµÄ¾àÀë
    static const int plateSpeed = 300;

    void update(float);

    void die();
    void lose();
};

#endif // __GAMESCENE_H__
