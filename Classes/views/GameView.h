#pragma once

#include "cocos2d.h"

#include "models/GameModel.h"

#include <functional>
#include <string>
#include <unordered_map>

namespace tripeaks
{

class GameView : public cocos2d::Node
{
public:
    CREATE_FUNC(GameView);

    bool init() override;

    void bindModel(GameModel* model);

    void setCardTapCallback(const std::function<void(int)>& callback);
    void setStockTapCallback(const std::function<void()>& callback);
    void setUndoCallback(const std::function<void()>& callback);

    void buildInitialLayout();

    void moveCardBackToPlayfield(int cardId, bool animated = true);
    void moveCardToStock(int cardId, int stockIndex, bool animated = true);

    // 手牌区相关动画
    void replaceTrayCardWithPlayfieldCard(int playfieldCardId, int oldTrayCardId, bool animated = true);
    void replaceTrayCardWithStockCard(int stockCardId, int oldTrayCardId, bool animated = true);
    void undoReplaceTrayCard(int playfieldCardId, int oldTrayCardId, bool animated = true);
    void undoReplaceTrayCardFromStock(int stockCardId, int oldTrayCardId, int stockIndex, bool animated = true);
    void placeInitialTrayCard(int cardId);

    void flipCard(int cardId, bool faceUp);

    void refreshCardStates();
    void layoutStock(int skipCardId = -1);

    void showStatusMessage(const std::string& text);
    void clearStatusMessage();

    void showVictory();
    void hideVictory();

    cocos2d::Vec2 getTrayBasePosition() const;
    cocos2d::Vec2 getStockBasePosition() const;

private:
    struct CardVisual
    {
        cocos2d::Node* root = nullptr;
        cocos2d::Sprite* front = nullptr;
        cocos2d::Sprite* back = nullptr;
        cocos2d::Node* numberNode = nullptr;
        cocos2d::Node* mirroredNumberNode = nullptr;
        cocos2d::Sprite* suitNode = nullptr;
        cocos2d::Sprite* cornerSuitNode = nullptr;
        cocos2d::Sprite* mirroredCornerSuitNode = nullptr;
        cocos2d::EventListenerTouchOneByOne* listener = nullptr;
        cocos2d::Vec2 homePosition;
        bool faceUp = false;
        bool inStock = false;
        bool inTray = false;
    };

    CardVisual* getVisual(int cardId);
    const CardVisual* getVisual(int cardId) const;

    CardVisual createCardVisual(const Card& card);
    void attachCardListener(int cardId, CardVisual& visual);
    void updateCardVisibility(CardVisual& visual);
    void updateCardScale();

    std::string getNumberSpritePath(CardFaceType face, bool isRed) const;
    std::string getSuitSpritePath(CardSuit suit) const;

    cocos2d::Vec2 getStockCardPosition(int index) const;
    cocos2d::Vec2 getTrayCardPosition() const;

    GameModel* _model = nullptr;
    std::unordered_map<int, CardVisual> _cardVisuals;

    cocos2d::Node* _cardLayer = nullptr;
    cocos2d::Node* _uiLayer = nullptr;
    cocos2d::Label* _stockCountLabel = nullptr;
    cocos2d::Label* _statusLabel = nullptr;
    cocos2d::Label* _victoryLabel = nullptr;

    cocos2d::Node* _stockTouchNode = nullptr;

    std::function<void(int)> _onCardTapped;
    std::function<void()> _onStockTapped;
    std::function<void()> _onUndoTapped;

    float _cardScale = 0.55F;
    float _boardScale = 1.0F;
    cocos2d::Vec2 _trayBasePosition;
    cocos2d::Vec2 _stockBasePosition;
    float _stockOffset = 24.0F;
};

} // namespace tripeaks


