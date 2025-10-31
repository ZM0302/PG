#include "views/GameView.h"

#include "ui/CocosGUI.h"

#include <algorithm>
#include <sstream>

namespace tripeaks
{

namespace
{

template <typename T>
T clampValue(T value, T minValue, T maxValue)
{
    if (value < minValue)
    {
        return minValue;
    }
    if (value > maxValue)
    {
        return maxValue;
    }
    return value;
}

std::string faceToString(CardFaceType face)
{
    switch (face)
    {
    case CardFaceType::Ace:
        return "A";
    case CardFaceType::Two:
        return "2";
    case CardFaceType::Three:
        return "3";
    case CardFaceType::Four:
        return "4";
    case CardFaceType::Five:
        return "5";
    case CardFaceType::Six:
        return "6";
    case CardFaceType::Seven:
        return "7";
    case CardFaceType::Eight:
        return "8";
    case CardFaceType::Nine:
        return "9";
    case CardFaceType::Ten:
        return "10";
    case CardFaceType::Jack:
        return "J";
    case CardFaceType::Queen:
        return "Q";
    case CardFaceType::King:
        return "K";
    default:
        return "?";
    }
}

bool isRedSuit(CardSuit suit)
{
    return suit == CardSuit::Hearts || suit == CardSuit::Diamonds;
}

constexpr float kDesignWidth = 1080.0F;
constexpr float kDesignHeight = 2080.0F;

} // namespace

bool GameView::init()
{
    if (!Node::init())
    {
        return false;
    }

    auto director = cocos2d::Director::getInstance();
    const cocos2d::Size visibleSize = director->getVisibleSize();
    const cocos2d::Vec2 origin = director->getVisibleOrigin();
    const cocos2d::Vec2 center(origin.x + visibleSize.width * 0.5F,
                               origin.y + visibleSize.height * 0.5F);

    _cardLayer = cocos2d::Node::create();
    _cardLayer->setPosition(center);
    const float layoutRatio = std::min(visibleSize.width / kDesignWidth, visibleSize.height / kDesignHeight);
    _boardScale = clampValue(layoutRatio * 0.85F, 0.6F, 1.0F);
    _cardLayer->setScale(_boardScale);
    addChild(_cardLayer, 1);

    _uiLayer = cocos2d::Node::create();
    _uiLayer->setPosition(origin);
    addChild(_uiLayer, 10);

    const float bottomAreaOffset = visibleSize.height * 0.32F;
    _trayBasePosition = cocos2d::Vec2(0.0F, -bottomAreaOffset);
    _stockBasePosition = cocos2d::Vec2(visibleSize.width * 0.24F, -bottomAreaOffset);
    _stockOffset = std::max(visibleSize.width * 0.022F, 16.0F);

    _stockCountLabel = cocos2d::Label::createWithSystemFont("Stock: 0", "Arial", 26);
    _stockCountLabel->setAnchorPoint({1.0F, 0.5F});
    _stockCountLabel->setPosition(origin.x + visibleSize.width - 40.0F,
                                  origin.y + 50.0F);
    _uiLayer->addChild(_stockCountLabel);

    _statusLabel = cocos2d::Label::createWithSystemFont("", "Arial", 28);
    _statusLabel->setAnchorPoint({0.5F, 0.5F});
    _statusLabel->setPosition(origin.x + visibleSize.width * 0.5F,
                              origin.y + visibleSize.height - 40.0F);
    _uiLayer->addChild(_statusLabel);

    auto undoLabel = cocos2d::Label::createWithSystemFont("Undo", "Arial", 32);
    auto undoItem = cocos2d::MenuItemLabel::create(undoLabel, [this](cocos2d::Ref*) {
        if (_onUndoTapped)
        {
            _onUndoTapped();
        }
    });
    undoItem->setPosition(origin.x + visibleSize.width - 80.0F,
                          origin.y + visibleSize.height - 60.0F);
    auto menu = cocos2d::Menu::create(undoItem, nullptr);
    menu->setPosition({0.0F, 0.0F});
    _uiLayer->addChild(menu);

    _victoryLabel = cocos2d::Label::createWithSystemFont("Victory!", "Arial", 48);
    _victoryLabel->setAnchorPoint({0.5F, 0.5F});
    _victoryLabel->setPosition(origin.x + visibleSize.width * 0.5F,
                               origin.y + visibleSize.height * 0.6F);
    _victoryLabel->setVisible(false);
    _uiLayer->addChild(_victoryLabel);

    cocos2d::Sprite* stockPlaceholder = cocos2d::Sprite::create("res/card_general.png");
    if (!stockPlaceholder)
    {
        stockPlaceholder = cocos2d::Sprite::create();
        stockPlaceholder->setTextureRect({0.0F, 0.0F, 128.0F, 170.0F});
        stockPlaceholder->setColor({40, 40, 40});
    }
    stockPlaceholder->setOpacity(0);
    stockPlaceholder->setScale(_cardScale);
    stockPlaceholder->setPosition(_stockBasePosition);
    _cardLayer->addChild(stockPlaceholder, 5);
    _stockTouchNode = stockPlaceholder;

    auto listener = cocos2d::EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [this](cocos2d::Touch* touch, cocos2d::Event* event) {
        if (!_onStockTapped)
        {
            return false;
        }

        const cocos2d::Vec2 location = _cardLayer->convertToNodeSpace(touch->getLocation());
        const cocos2d::Rect bounds = _stockTouchNode->getBoundingBox();
        if (bounds.containsPoint(location))
        {
            _onStockTapped();
            return true;
        }
        return false;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, _stockTouchNode);

    updateCardScale();

    return true;
}

void GameView::bindModel(GameModel* model)
{
    _model = model;
}

void GameView::setCardTapCallback(const std::function<void(int)>& callback)
{
    _onCardTapped = callback;
}

void GameView::setStockTapCallback(const std::function<void()>& callback)
{
    _onStockTapped = callback;
}

void GameView::setUndoCallback(const std::function<void()>& callback)
{
    _onUndoTapped = callback;
}

void GameView::buildInitialLayout()
{
    if (!_model)
    {
        return;
    }

    _cardVisuals.clear();
    if (_stockTouchNode && _stockTouchNode->getParent())
    {
        _stockTouchNode->removeFromParentAndCleanup(false);
    }

    _cardLayer->removeAllChildrenWithCleanup(true);

    auto director = cocos2d::Director::getInstance();
    const cocos2d::Size visibleSize = director->getVisibleSize();
    const cocos2d::Vec2 origin = director->getVisibleOrigin();
    const cocos2d::Vec2 center(origin.x + visibleSize.width * 0.5F,
                               origin.y + visibleSize.height * 0.5F);

    const float layoutRatio = std::min(visibleSize.width / kDesignWidth, visibleSize.height / kDesignHeight);
    _boardScale = clampValue(layoutRatio * 0.85F, 0.6F, 1.0F);
    const float bottomAreaOffset = visibleSize.height * 0.32F;
    _trayBasePosition = cocos2d::Vec2(0.0F, -bottomAreaOffset);
    _stockBasePosition = cocos2d::Vec2(visibleSize.width * 0.24F, -bottomAreaOffset);
    _stockOffset = std::max(visibleSize.width * 0.022F, 16.0F);
    _cardLayer->setPosition(center);
    _cardLayer->setScale(_boardScale);

    if (_stockTouchNode)
    {
        _cardLayer->addChild(_stockTouchNode, 900);
        _stockTouchNode->setPosition(_stockBasePosition);
    }

    const auto& playfieldIds = _model->getPlayfieldCardIds();
    for (int cardId : playfieldIds)
    {
        const Card* card = _model->getCardById(cardId);
        if (!card)
        {
            continue;
        }
        CardVisual visual = createCardVisual(*card);
        visual.homePosition = card->position;
        visual.root->setPosition(card->position);
        visual.inStock = false;
        visual.inTray = false;
        attachCardListener(cardId, visual);
        _cardLayer->addChild(visual.root, static_cast<int>(1000 - card->position.y));
        _cardVisuals.emplace(cardId, visual);
    }

    const auto& stockIds = _model->getStockCardIds();
    for (std::size_t index = 0; index < stockIds.size(); ++index)
    {
        int cardId = stockIds[index];
        const Card* card = _model->getCardById(cardId);
        if (!card)
        {
            continue;
        }
        CardVisual visual = createCardVisual(*card);
        visual.inStock = true;
        visual.inTray = false;
        visual.homePosition = getStockCardPosition(static_cast<int>(index));
        visual.root->setPosition(visual.homePosition);
        visual.root->setLocalZOrder(static_cast<int>(500 + index));
        _cardLayer->addChild(visual.root, visual.root->getLocalZOrder());
        _cardVisuals.emplace(cardId, visual);
    }

    // 显示手牌区顶部牌（如果tray card已经存在，更新其状态；否则创建新的visual）
    const int trayCardId = _model->getTrayCardId();
    if (trayCardId >= 0)
    {
        CardVisual* existingVisual = getVisual(trayCardId);
        if (existingVisual)
        {
            // 更新现有visual的状态
            existingVisual->inTray = true;
            existingVisual->inStock = false;
            existingVisual->homePosition = getTrayCardPosition();
            existingVisual->root->setPosition(existingVisual->homePosition);
            existingVisual->root->setLocalZOrder(800);
        }
        else
        {
            // 创建新的visual（通常不会发生，因为tray card是从stock中抽取的）
            const Card* trayCard = _model->getCardById(trayCardId);
            if (trayCard)
            {
                CardVisual visual = createCardVisual(*trayCard);
                visual.inTray = true;
                visual.inStock = false;
                visual.homePosition = getTrayCardPosition();
                visual.root->setPosition(visual.homePosition);
                visual.root->setLocalZOrder(800);
                _cardLayer->addChild(visual.root, 800);
                _cardVisuals.emplace(trayCardId, visual);
            }
        }
    }

    // keep stock touch node above card sprites
    if (_stockTouchNode)
    {
        _stockTouchNode->removeFromParent();
        _cardLayer->addChild(_stockTouchNode, 900);
        _stockTouchNode->setScale(_cardScale);
    }

    layoutStock();
    refreshCardStates();
}


void GameView::moveCardBackToPlayfield(int cardId, bool animated)
{
    CardVisual* visual = getVisual(cardId);
    if (!visual)
    {
        return;
    }

    visual->inTray = false;
    visual->inStock = false;
    const cocos2d::Vec2 target = visual->homePosition;
    visual->root->stopAllActions();
    visual->root->setLocalZOrder(static_cast<int>(1000 - target.y));

    if (animated)
    {
        auto move = cocos2d::MoveTo::create(0.2F, target);
        visual->root->runAction(move);
    }
    else
    {
        visual->root->setPosition(target);
    }
}

void GameView::moveCardToStock(int cardId, int stockIndex, bool animated)
{
    CardVisual* visual = getVisual(cardId);
    if (!visual)
    {
        return;
    }

    visual->inTray = false;
    visual->inStock = true;
    const cocos2d::Vec2 target = getStockCardPosition(stockIndex);
    visual->homePosition = target;
    visual->root->stopAllActions();
    visual->root->setLocalZOrder(static_cast<int>(500 + stockIndex));

    if (animated)
    {
        auto move = cocos2d::MoveTo::create(0.2F, target);
        visual->root->runAction(move);
    }
    else
    {
        visual->root->setPosition(target);
    }
}

void GameView::replaceTrayCardWithPlayfieldCard(int playfieldCardId, int oldTrayCardId, bool animated)
{
    CardVisual* playfieldVisual = getVisual(playfieldCardId);
    CardVisual* oldTrayVisual = getVisual(oldTrayCardId);
    
    if (!playfieldVisual)
    {
        return;
    }

    const cocos2d::Vec2 trayPos = getTrayCardPosition();
    
    // 更新新tray牌状态
    playfieldVisual->inTray = true;
    playfieldVisual->inStock = false;
    playfieldVisual->homePosition = trayPos;
    playfieldVisual->root->stopAllActions();
    playfieldVisual->root->setLocalZOrder(800);

    // 如果有旧的tray牌，需要将其移回stock（如果oldTrayCardId有效）
    if (oldTrayCardId >= 0 && oldTrayVisual)
    {
        oldTrayVisual->inTray = false;
        oldTrayVisual->inStock = true;
        const int stockIndex = static_cast<int>(_model->getStockCardIds().size());
        const cocos2d::Vec2 stockPos = getStockCardPosition(stockIndex);
        oldTrayVisual->homePosition = stockPos;
        oldTrayVisual->root->stopAllActions();
        oldTrayVisual->root->setLocalZOrder(static_cast<int>(500 + stockIndex));
        
        if (animated)
        {
            auto moveOld = cocos2d::MoveTo::create(0.2F, stockPos);
            oldTrayVisual->root->runAction(moveOld);
        }
        else
        {
            oldTrayVisual->root->setPosition(stockPos);
        }
    }

    // 移动桌面牌到手牌区
    if (animated)
    {
        auto move = cocos2d::MoveTo::create(0.2F, trayPos);
        playfieldVisual->root->runAction(move);
    }
    else
    {
        playfieldVisual->root->setPosition(trayPos);
    }
}

void GameView::replaceTrayCardWithStockCard(int stockCardId, int oldTrayCardId, bool animated)
{
    CardVisual* stockVisual = getVisual(stockCardId);
    CardVisual* oldTrayVisual = getVisual(oldTrayCardId);
    
    if (!stockVisual)
    {
        return;
    }

    const cocos2d::Vec2 trayPos = getTrayCardPosition();
    
    // 更新新tray牌状态
    stockVisual->inTray = true;
    stockVisual->inStock = false;
    stockVisual->homePosition = trayPos;
    stockVisual->root->stopAllActions();
    stockVisual->root->setLocalZOrder(800);

    // 如果有旧的tray牌，需要将其移回stock
    if (oldTrayCardId >= 0 && oldTrayVisual)
    {
        oldTrayVisual->inTray = false;
        oldTrayVisual->inStock = true;
        const int stockIndex = static_cast<int>(_model->getStockCardIds().size());
        const cocos2d::Vec2 stockPos = getStockCardPosition(stockIndex);
        oldTrayVisual->homePosition = stockPos;
        oldTrayVisual->root->stopAllActions();
        oldTrayVisual->root->setLocalZOrder(static_cast<int>(500 + stockIndex));
        
        if (animated)
        {
            auto moveOld = cocos2d::MoveTo::create(0.2F, stockPos);
            oldTrayVisual->root->runAction(moveOld);
        }
        else
        {
            oldTrayVisual->root->setPosition(stockPos);
        }
    }

    // 移动stock牌到手牌区
    if (animated)
    {
        auto move = cocos2d::MoveTo::create(0.2F, trayPos);
        stockVisual->root->runAction(move);
    }
    else
    {
        stockVisual->root->setPosition(trayPos);
    }
}

void GameView::undoReplaceTrayCard(int playfieldCardId, int oldTrayCardId, bool animated)
{
    CardVisual* playfieldVisual = getVisual(playfieldCardId);
    CardVisual* oldTrayVisual = getVisual(oldTrayCardId);
    
    if (!playfieldVisual)
    {
        return;
    }

    // 恢复桌面牌位置
    const Card* card = _model->getCardById(playfieldCardId);
    if (card)
    {
        playfieldVisual->inTray = false;
        playfieldVisual->inStock = false;
        playfieldVisual->homePosition = card->position;
        playfieldVisual->root->stopAllActions();
        playfieldVisual->root->setLocalZOrder(static_cast<int>(1000 - card->position.y));

        if (animated)
        {
            auto move = cocos2d::MoveTo::create(0.2F, card->position);
            playfieldVisual->root->runAction(move);
        }
        else
        {
            playfieldVisual->root->setPosition(card->position);
        }
    }

    // 恢复旧tray牌位置
    if (oldTrayCardId >= 0 && oldTrayVisual)
    {
        oldTrayVisual->inTray = true;
        oldTrayVisual->inStock = false;
        const cocos2d::Vec2 trayPos = getTrayCardPosition();
        oldTrayVisual->homePosition = trayPos;
        oldTrayVisual->root->stopAllActions();
        oldTrayVisual->root->setLocalZOrder(800);
        
        if (animated)
        {
            auto moveOld = cocos2d::MoveTo::create(0.2F, trayPos);
            oldTrayVisual->root->runAction(moveOld);
        }
        else
        {
            oldTrayVisual->root->setPosition(trayPos);
        }
    }
}

void GameView::undoReplaceTrayCardFromStock(int stockCardId, int oldTrayCardId, int stockIndex, bool animated)
{
    CardVisual* stockVisual = getVisual(stockCardId);
    CardVisual* oldTrayVisual = getVisual(oldTrayCardId);
    
    if (!stockVisual)
    {
        return;
    }

    // 恢复stock牌位置
    stockVisual->inTray = false;
    stockVisual->inStock = true;
    const cocos2d::Vec2 stockPos = getStockCardPosition(stockIndex);
    stockVisual->homePosition = stockPos;
    stockVisual->root->stopAllActions();
    stockVisual->root->setLocalZOrder(static_cast<int>(500 + stockIndex));

    if (animated)
    {
        auto move = cocos2d::MoveTo::create(0.2F, stockPos);
        stockVisual->root->runAction(move);
    }
    else
    {
        stockVisual->root->setPosition(stockPos);
    }

    // 恢复旧tray牌位置
    if (oldTrayCardId >= 0 && oldTrayVisual)
    {
        oldTrayVisual->inTray = true;
        oldTrayVisual->inStock = false;
        const cocos2d::Vec2 trayPos = getTrayCardPosition();
        oldTrayVisual->homePosition = trayPos;
        oldTrayVisual->root->stopAllActions();
        oldTrayVisual->root->setLocalZOrder(800);
        
        if (animated)
        {
            auto moveOld = cocos2d::MoveTo::create(0.2F, trayPos);
            oldTrayVisual->root->runAction(moveOld);
        }
        else
        {
            oldTrayVisual->root->setPosition(trayPos);
        }
    }
}

void GameView::flipCard(int cardId, bool faceUp)
{
    CardVisual* visual = getVisual(cardId);
    if (!visual)
    {
        return;
    }

    if (visual->faceUp == faceUp)
    {
        return;
    }

    visual->faceUp = faceUp;

    auto firstHalf = cocos2d::ScaleTo::create(0.1F, 0.0F, _cardScale);
    auto show = cocos2d::CallFunc::create([visual, faceUp]() {
        if (visual->front)
        {
            visual->front->setVisible(faceUp);
        }
        if (visual->back)
        {
            visual->back->setVisible(!faceUp);
        }
    });
    auto secondHalf = cocos2d::ScaleTo::create(0.1F, _cardScale, _cardScale);
    auto sequence = cocos2d::Sequence::create(firstHalf, show, secondHalf, nullptr);
    visual->root->runAction(sequence);
}

void GameView::refreshCardStates()
{
    if (!_model)
    {
        return;
    }

    const auto& playfieldIds = _model->getPlayfieldCardIds();
    for (int cardId : playfieldIds)
    {
        CardVisual* visual = getVisual(cardId);
        const Card* card = _model->getCardById(cardId);
        if (!visual || !card)
        {
            continue;
        }

        const bool active = !card->removed && card->faceUp && _model->isCardExposed(cardId);
        if (visual->front)
        {
            visual->front->setColor(active ? cocos2d::Color3B::WHITE : cocos2d::Color3B(160, 160, 160));
        }
        if (visual->listener)
        {
            visual->listener->setEnabled(active);
        }
    }

    const bool stockAvailable = _model->getStockCardIds().empty()
        ? false
        : true;
    _stockTouchNode->setColor(stockAvailable ? cocos2d::Color3B::WHITE : cocos2d::Color3B(120, 120, 120));
}

void GameView::layoutStock(int skipCardId)
{
    if (!_model)
    {
        return;
    }

    const auto& stockIds = _model->getStockCardIds();
    for (std::size_t index = 0; index < stockIds.size(); ++index)
    {
        if (stockIds[index] == skipCardId)
        {
            continue;
        }
        moveCardToStock(stockIds[index], static_cast<int>(index), false);
    }

    std::ostringstream oss;
    oss << "Stock: " << stockIds.size();
    _stockCountLabel->setString(oss.str());
}

void GameView::placeInitialTrayCard(int cardId)
{
    CardVisual* visual = getVisual(cardId);
    if (!visual)
    {
        return;
    }

    visual->inTray = true;
    visual->inStock = false;
    const cocos2d::Vec2 target = getTrayCardPosition();
    visual->homePosition = target;
    visual->root->setPosition(target);
    visual->root->setLocalZOrder(800);
    flipCard(cardId, true);
}

void GameView::showStatusMessage(const std::string& text)
{
    if (_statusLabel)
    {
        _statusLabel->setString(text);
        _statusLabel->stopAllActions();
        _statusLabel->setOpacity(255);
        auto delay = cocos2d::DelayTime::create(1.0F);
        auto fade = cocos2d::FadeOut::create(0.4F);
        _statusLabel->runAction(cocos2d::Sequence::create(delay, fade, nullptr));
    }
}

void GameView::clearStatusMessage()
{
    if (_statusLabel)
    {
        _statusLabel->setString("");
    }
}

void GameView::showVictory()
{
    if (_victoryLabel)
    {
        _victoryLabel->setVisible(true);
        _victoryLabel->runAction(cocos2d::RepeatForever::create(
            cocos2d::Sequence::create(
                cocos2d::ScaleTo::create(0.4F, 1.2F),
                cocos2d::ScaleTo::create(0.4F, 1.0F),
                nullptr)));
    }
}

void GameView::hideVictory()
{
    if (_victoryLabel)
    {
        _victoryLabel->stopAllActions();
        _victoryLabel->setScale(1.0F);
        _victoryLabel->setVisible(false);
    }
}

cocos2d::Vec2 GameView::getTrayBasePosition() const
{
    return _trayBasePosition;
}

cocos2d::Vec2 GameView::getStockBasePosition() const
{
    return _stockBasePosition;
}

GameView::CardVisual* GameView::getVisual(int cardId)
{
    auto iter = _cardVisuals.find(cardId);
    if (iter == _cardVisuals.end())
    {
        return nullptr;
    }
    return &iter->second;
}

const GameView::CardVisual* GameView::getVisual(int cardId) const
{
    auto iter = _cardVisuals.find(cardId);
    if (iter == _cardVisuals.end())
    {
        return nullptr;
    }
    return &iter->second;
}

GameView::CardVisual GameView::createCardVisual(const Card& card)
{
    CardVisual visual;
    visual.root = cocos2d::Node::create();
    visual.root->setScale(_cardScale);

    visual.back = cocos2d::Sprite::create("res/card_general.png");
    if (!visual.back)
    {
        visual.back = cocos2d::Sprite::create();
        visual.back->setTextureRect({0.0F, 0.0F, 128.0F, 170.0F});
    }
    visual.back->setColor({26, 82, 173});
    visual.back->setAnchorPoint({0.5F, 0.5F});
    visual.root->addChild(visual.back, 0);

    visual.front = cocos2d::Sprite::create("res/card_general.png");
    if (!visual.front)
    {
        visual.front = cocos2d::Sprite::create();
        visual.front->setTextureRect({0.0F, 0.0F, 128.0F, 170.0F});
        visual.front->setColor({235, 235, 235});
    }
    visual.front->setAnchorPoint({0.5F, 0.5F});
    visual.root->addChild(visual.front, 1);

    const cocos2d::Size cardSize = visual.front->getContentSize();
    const float halfWidth = cardSize.width * 0.5F;
    const float halfHeight = cardSize.height * 0.5F;

    const bool red = isRedSuit(card.suit);
    const std::string numberPath = getNumberSpritePath(card.face, red);
    const std::string suitPath = getSuitSpritePath(card.suit);

    const float paddingX = cardSize.width * 0.08F;
    const float paddingY = cardSize.height * 0.08F;
    const float cornerSpacing = cardSize.height * 0.02F;
    const float cornerShiftX = cardSize.width * 0.5F;
    const float cornerShiftY = cardSize.height * 0.5F;
    const float rankTargetHeight = cardSize.height * 0.22F;
    const float cornerSuitTargetHeight = cardSize.height * 0.14F;
    const float centerSuitTargetHeight = cardSize.height * 0.32F;
    const float centerShiftX = cardSize.width * 0.5F;
    const float centerShiftY = cardSize.height * 0.5F;

    auto getNodeSize = [](cocos2d::Node* node) {
        cocos2d::Size size = node->getContentSize();
        size.width *= node->getScaleX();
        size.height *= node->getScaleY();
        return size;
    };

    auto createRankNode = [&]() -> cocos2d::Node* {
        if (auto sprite = cocos2d::Sprite::create(numberPath))
        {
            const float contentHeight = sprite->getContentSize().height;
            if (contentHeight > 0.0F)
            {
                sprite->setScale(rankTargetHeight / contentHeight);
            }
            return sprite;
        }

        auto label = cocos2d::Label::createWithSystemFont(faceToString(card.face), "Arial", rankTargetHeight);
        label->setColor(red ? cocos2d::Color3B::RED : cocos2d::Color3B::BLACK);
        return label;
    };

    auto createCornerSuitNode = [&]() -> cocos2d::Sprite* {
        cocos2d::Sprite* sprite = cocos2d::Sprite::create(suitPath);
        if (!sprite)
        {
            sprite = cocos2d::Sprite::create();
            sprite->setTextureRect({0.0F, 0.0F, 50.0F, 50.0F});
            sprite->setColor(red ? cocos2d::Color3B::RED : cocos2d::Color3B::BLACK);
        }
        const float contentHeight = sprite->getContentSize().height;
        if (contentHeight > 0.0F)
        {
            sprite->setScale(cornerSuitTargetHeight / contentHeight);
        }
        return sprite;
    };

    auto topCornerContainer = cocos2d::Node::create();
    topCornerContainer->ignoreAnchorPointForPosition(false);
    topCornerContainer->setAnchorPoint({0.0F, 1.0F});
    topCornerContainer->setPosition({-halfWidth + paddingX + cornerShiftX, halfHeight - paddingY + cornerShiftY});
    visual.front->addChild(topCornerContainer, 2);

    cocos2d::Node* topRankNode = createRankNode();
    topRankNode->setAnchorPoint({0.0F, 1.0F});
    topRankNode->setPosition({0.0F, 0.0F});
    topCornerContainer->addChild(topRankNode);
    visual.numberNode = topRankNode;

    const cocos2d::Size rankSize = getNodeSize(topRankNode);

    cocos2d::Sprite* topSuitNode = createCornerSuitNode();
    topSuitNode->setAnchorPoint({0.0F, 1.0F});
    topSuitNode->setPosition({0.0F, -rankSize.height - cornerSpacing});
    topCornerContainer->addChild(topSuitNode);
    visual.cornerSuitNode = topSuitNode;

    auto bottomCornerContainer = cocos2d::Node::create();
    bottomCornerContainer->ignoreAnchorPointForPosition(false);
    bottomCornerContainer->setAnchorPoint({1.0F, 0.0F});
    bottomCornerContainer->setPosition({halfWidth - paddingX + cornerShiftX, -halfHeight + paddingY + cornerShiftY});
    bottomCornerContainer->setRotation(180.0F);
    visual.front->addChild(bottomCornerContainer, 2);

    cocos2d::Node* bottomRankNode = createRankNode();
    bottomRankNode->setAnchorPoint({0.0F, 1.0F});
    bottomRankNode->setPosition({0.0F, 0.0F});
    bottomCornerContainer->addChild(bottomRankNode);
    visual.mirroredNumberNode = bottomRankNode;

    const cocos2d::Size mirroredRankSize = getNodeSize(bottomRankNode);

    cocos2d::Sprite* bottomSuitNode = createCornerSuitNode();
    bottomSuitNode->setAnchorPoint({0.0F, 1.0F});
    bottomSuitNode->setPosition({0.0F, -mirroredRankSize.height - cornerSpacing});
    bottomCornerContainer->addChild(bottomSuitNode);
    visual.mirroredCornerSuitNode = bottomSuitNode;

    cocos2d::Sprite* centerSuit = cocos2d::Sprite::create(suitPath);
    if (!centerSuit)
    {
        centerSuit = cocos2d::Sprite::create();
        centerSuit->setTextureRect({0.0F, 0.0F, 50.0F, 50.0F});
        centerSuit->setColor(red ? cocos2d::Color3B::RED : cocos2d::Color3B::BLACK);
    }
    const float centerContentHeight = centerSuit->getContentSize().height;
    if (centerContentHeight > 0.0F)
    {
        centerSuit->setScale(centerSuitTargetHeight / centerContentHeight);
    }
    centerSuit->setAnchorPoint({0.5F, 0.5F});
    centerSuit->setPosition({centerShiftX, centerShiftY});
    visual.front->addChild(centerSuit, 1);
    visual.suitNode = centerSuit;

    visual.front->setVisible(card.faceUp);
    visual.back->setVisible(!card.faceUp);
    visual.faceUp = card.faceUp;

    return visual;
}

void GameView::attachCardListener(int cardId, CardVisual& visual)
{
    auto listener = cocos2d::EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [this, cardId](cocos2d::Touch* touch, cocos2d::Event* event) {
        if (!_model || !_onCardTapped)
        {
            return false;
        }

        GameView::CardVisual* visualPtr = getVisual(cardId);
        if (!visualPtr || !visualPtr->root)
        {
            return false;
        }

        const cocos2d::Vec2 locationInRoot = visualPtr->root->convertToNodeSpace(touch->getLocation());
        cocos2d::Rect touchBounds;
        if (visualPtr->front)
        {
            touchBounds = visualPtr->front->getBoundingBox();
        }
        else if (visualPtr->back)
        {
            touchBounds = visualPtr->back->getBoundingBox();
        }
        else
        {
            touchBounds = cocos2d::Rect(-64.0F, -85.0F, 128.0F, 170.0F);
        }

        if (!touchBounds.containsPoint(locationInRoot))
        {
            return false;
        }

        _onCardTapped(cardId);
        return true;
    };
    visual.listener = listener;
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, visual.root);
}

void GameView::updateCardVisibility(CardVisual& visual)
{
    visual.front->setVisible(visual.faceUp);
    visual.back->setVisible(!visual.faceUp);
}

void GameView::updateCardScale()
{
    const float minScale = 0.5F;
    const float maxScale = 0.7F;
    const cocos2d::Size visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    const float referenceHeight = kDesignHeight;
    _cardScale = clampValue(visibleSize.height / referenceHeight * 0.55F, minScale, maxScale);

    const float layoutRatio = std::min(visibleSize.width / kDesignWidth, visibleSize.height / kDesignHeight);
    _boardScale = clampValue(layoutRatio * 0.85F, 0.6F, 1.0F);
    if (_cardLayer)
    {
        _cardLayer->setScale(_boardScale);
    }

    for (auto& pair : _cardVisuals)
    {
        if (pair.second.root)
        {
            pair.second.root->setScale(_cardScale);
        }
    }

    if (_stockTouchNode)
    {
        _stockTouchNode->setScale(_cardScale);
    }
}

std::string GameView::getNumberSpritePath(CardFaceType face, bool isRed) const
{
    const std::string prefix = isRed ? "res/number/small_red_" : "res/number/small_black_";
    return prefix + faceToString(face) + ".png";
}

std::string GameView::getSuitSpritePath(CardSuit suit) const
{
    switch (suit)
    {
    case CardSuit::Clubs:
        return "res/suits/club.png";
    case CardSuit::Diamonds:
        return "res/suits/diamond.png";
    case CardSuit::Hearts:
        return "res/suits/heart.png";
    case CardSuit::Spades:
        return "res/suits/spade.png";
    default:
        return "";
    }
}

cocos2d::Vec2 GameView::getStockCardPosition(int index) const
{
    return _stockBasePosition + cocos2d::Vec2(index * _stockOffset * 0.4F, index * 0.5F);
}

cocos2d::Vec2 GameView::getTrayCardPosition() const
{
    return _trayBasePosition;
}

} // namespace tripeaks


