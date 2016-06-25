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

const float GameScene::plateSpeed = 300;
const float GameScene::toolBaseSpeed = 200;
const float GameScene::ballMaxAngle = 60;
const float GameScene::toolAverageRefreshTime = 2;
const float GameScene::throughDuration = 10;
const float GameScene::toRad = 3.141592f / 180;
const float GameScene::toAngle = 180 / 3.1416f;
PhysicsMaterial GameScene::elasticMaterial(0, 1, 0);

Scene* GameScene::createScene()
{
	auto scene = Scene::createWithPhysics();
	//scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
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
	bgsprite->setScale(visibleSize.width / bgsprite->getContentSize().width, visibleSize.height / bgsprite->getContentSize().height);
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
	auto bottomBody = PhysicsBody::createBox(Size(visibleSize.width, gap));
	bottomBody->setDynamic(false);
	bottomBody->setCategoryBitmask(bottomBit);
	bottomBody->setCollisionBitmask(ballBit | toolBit);
	bottomBody->setContactTestBitmask(ballBit | toolBit);
	bottom->setPhysicsBody(bottomBody);
	this->addChild(bottom);

	// 设置滑板
	plate = Sprite::create("sprite/plate.png");
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
	case cocos2d::EventKeyboard::KeyCode::KEY_C:
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_B:// 用于调试：进入下一关
		nextLevel();
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:
		// 空格发射
		if (!launched) {
			SimpleAudioEngine::getInstance()->playEffect("effect/faqiu.wav");
			ballRoot->getChildByName("ball")->getPhysicsBody()->setVelocity(Vec2(1, 0).rotateByAngle(Vec2(), random(90 - ballMaxAngle, 90 + ballMaxAngle) * toRad) * ballSpeed);
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
				if (brickRoot->getChildrenCount() == 0)
					scheduleOnce(schedule_selector(GameScene::scheduleNextLevel, this), 0);
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
			through = true;
			_damage = 10;
			unschedule(schedule_selector(GameScene::endThrough));
			scheduleOnce(schedule_selector(GameScene::endThrough), throughDuration);
		}
		else if (name == "multi") {
			if (launched) {
				auto ball = createBall();
				auto sball = ballRoot->getChildByName("ball");
				ball->setPosition(sball->getPosition());
				auto vec = sball->getPhysicsBody()->getVelocity();
				vec.x *= -1;
				ball->getPhysicsBody()->setVelocity(vec);
				ballRoot->addChild(ball);
			}
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
		auto angle = v.getAngle() * toAngle;
		if (90 - angle > ballMaxAngle) {
			//v = Vec2(1, 0).rotateByAngle(Vec2(), ballMaxAngle * toRad) * ballSpeed;
			v = Vec2(1, 0).rotateByAngle(Vec2(), (angle + 5) * toRad) * ballSpeed;
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
	if (!launched)
		ballRoot->getChildByName("ball")->setPosition(plate->getPositionX(), plate->getPositionY() + plate->getContentSize().height / 2 + ballRoot->getChildByName("ball")->getContentSize().height / 2);
}

void GameScene::randomCreateTools(float deltaTime) {
	if (random(0.0f, toolAverageRefreshTime / deltaTime) <= 1) {
		Sprite *tool;
		switch (random(1, 3)) {
		case 1:
			tool = Sprite::create("sprite/addLife.png");
			tool->setName("addLife");
			break;
		case 2:
			tool = Sprite::create("sprite/through.png");
			tool->setName("through");
			break;
		case 3:
			tool = Sprite::create("sprite/multi.png");
			tool->setName("multi");
			break;
		}
		auto size = tool->getContentSize();
		tool->setPosition(random(size.width / 2, visibleSize.width - size.width / 2), visibleSize.height - size.height / 2);
		auto toolBody = PhysicsBody::createBox(size, elasticMaterial);
		toolBody->setGroup(-1);
		toolBody->setCategoryBitmask(toolBit);
		toolBody->setCollisionBitmask(bottomBit | plateBit);
		toolBody->setContactTestBitmask(bottomBit | plateBit);
		toolBody->setVelocity(Vec2(0, -random(1.0f, 1.5f) * toolBaseSpeed));
		tool->setPhysicsBody(toolBody);
		this->addChild(tool, 2);
	}
}

void GameScene::endThrough(float) {
	through = false;
	_damage = 1;
}

void GameScene::scheduleNextLevel(float) {
	nextLevel();
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
		unschedule(schedule_selector(GameScene::randomCreateTools));
		unschedule(schedule_selector(GameScene::endThrough));
		setLabel(level, _level);
		pressA = pressD = pressLeft = pressRight = false;
		_damage = 1;
		launched = false;
		through = false;
		auto v = ballRoot->getChildren();
		while (v.size() > 1) {
			v.at(1)->removeFromParentAndCleanup(1);
			v.erase(1);
		}
		auto ball = v.at(0);
		ball->getPhysicsBody()->setVelocity(Vec2());
		ballSpeed = plateSpeed * (1 + 0.1f * _level);
		ball->setPosition(plate->getPositionX(), plate->getPositionY() + plate->getContentSize().height / 2 + ball->getContentSize().height / 2);
		plate->setPosition(visibleSize.width / 2, gap + plate->getContentSize().height / 2);
		static const float sx = visibleSize.width / 2 - brickWidth * (col - 1) / 2, sy = 550;
		for (int i = 0; i < row; i++) {
			for (int j = 0; j < col; j++) {
				Sprite *brick;
				switch (map[_level][i][j]) {
				case 0: continue;
				case 1: brick = createBrick("sprite/brick.png", 1, Color3B(255, 255, 255)); break;
				case 2: brick = createBrick("sprite/brick.png", 2, Color3B(255, 0, 255)); break;
				case 9: brick = createBrick("sprite/stonebrick.png", 1 << 30, Color3B(255, 255, 255)); break;
				}
				brick->setPosition(sx + j * brickWidth, sy - i * brickHeight);
				brickRoot->addChild(brick, 1);
			}
		}
	}
}

void GameScene::win() {
	unscheduleUpdate();
	this->removeAllChildrenWithCleanup(1);
	auto gamescene = WinScene::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(0.5, gamescene, Color3B(0, 0, 0)));
}

void GameScene::lose() {
	auto gamescene = LoseScene::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(0.5, gamescene, Color3B(0, 0, 0)));
}

Sprite *GameScene::createBrick(const std::string &filename, int life, const Color3B &color) {
	auto brick = Sprite::create(filename);
	brick->setName("brick");
	brick->setColor(color);
	brick->setScale(brickWidth / brick->getContentSize().width, brickHeight / brick->getContentSize().height);
	brick->setContentSize(Size(brickWidth, brickHeight));
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
	auto ball = Sprite::create("ball.png");
	ball->setName("ball");
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
