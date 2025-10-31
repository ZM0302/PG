#pragma once

#include "configs/models/LevelConfig.h"

#include "cocos2d.h"

#include <unordered_map>
#include <vector>

namespace tripeaks
{

struct Card
{
    int id = -1;
    CardFaceType face = CardFaceType::Ace;
    CardSuit suit = CardSuit::Clubs;
    cocos2d::Vec2 position;
    bool faceUp = false;
    bool removed = false;
    bool isInPlayfield = true;

    std::vector<int> coveredByCardIds;   // cards that block this card
    std::vector<int> coveringCardIds;    // cards that this card blocks
};

class GameModel
{
public:
    void reset();

    Card* getCardById(int cardId);
    const Card* getCardById(int cardId) const;

    std::vector<int>& getPlayfieldCardIds();
    const std::vector<int>& getPlayfieldCardIds() const;

    std::vector<int>& getStockCardIds();
    const std::vector<int>& getStockCardIds() const;

    Card* addCard(const LevelCardConfig& config, bool isPlayfieldCard);

    int getPlayfieldIndex(int cardId) const;

    bool isCardExposed(int cardId) const;
    bool isCardRemoved(int cardId) const;

    void setCardFaceUp(int cardId, bool faceUp);
    void setCardRemoved(int cardId, bool removed);

    void removeCardFromPlayfield(int cardId);
    void restoreCardToPlayfield(int cardId, int insertIndex);

    // Tray (手牌区) - 只有一张顶部牌
    void setTrayCard(int cardId);
    int getTrayCardId() const;
    int replaceTrayCard(int newCardId);  // 返回旧的tray card ID，如果没有则返回-1

    int drawCardFromStock();
    void returnCardToStock(int cardId);

    void setCardFaceAndSuit(int cardId, CardFaceType face, CardSuit suit);

    void rebuildCoveringRelations();

    bool isVictory() const;

private:
    std::vector<Card> _cards;
    std::unordered_map<int, std::size_t> _cardIndexById;
    std::vector<int> _playfieldCardIds;
    std::vector<int> _stockCardIds;
    int _trayCardId = -1;  // 手牌区顶部牌ID，-1表示无牌
};

} // namespace tripeaks


