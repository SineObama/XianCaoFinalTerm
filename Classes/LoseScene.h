#pragma once
#ifndef __LOSESCENE_H__
#define __LOSESCENE_H__

#include "cocos2d.h"

class LoseScene : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    // a selector callback
    //void startMenuCallback(cocos2d::Ref* pSender);

    // implement the "static create()" method manually
    CREATE_FUNC(LoseScene);

    void ChangeScene(Ref *);
};

#endif // __MENU_SEBCE_H__
