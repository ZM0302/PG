/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "HelloWorldScene.h"

#include "views/GameView.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

bool HelloWorld::init()
{
    if (!Scene::init())
    {
        return false;
    }

    using namespace tripeaks;

    const auto visibleSize = Director::getInstance()->getVisibleSize();
    const auto origin = Director::getInstance()->getVisibleOrigin();

    auto background = LayerColor::create(Color4B(20, 70, 120, 255), visibleSize.width, visibleSize.height);
    background->setPosition(origin);
    addChild(background, -1);

    _gameView = GameView::create();
    if (!_gameView)
    {
        return false;
    }
    addChild(_gameView, 1);

    _gameController = std::make_unique<GameController>();

    const std::string levelPath = FileUtils::getInstance()->fullPathForFilename("levels/level_tripeaks_standard.json");
    if (levelPath.empty() || !_gameController->init(_gameView, levelPath))
    {
        auto message = Label::createWithSystemFont("Failed to load level", "Arial", 36);
        message->setPosition(Vec2(origin.x + visibleSize.width * 0.5F,
                                   origin.y + visibleSize.height * 0.5F));
        addChild(message, 10);
    }

    return true;
}
