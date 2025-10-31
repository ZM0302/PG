#include "models/GameModel.h"

#include <algorithm>

namespace tripeaks
{

namespace
{

int generateCardId(std::size_t index)
{
    return static_cast<int>(index);
}

} // namespace

void GameModel::reset()
{
    _cards.clear();
    _cardIndexById.clear();
    _playfieldCardIds.clear();
    _stockCardIds.clear();
    _trayCardId = -1;
}

Card* GameModel::getCardById(int cardId)
{
    auto iter = _cardIndexById.find(cardId);
    if (iter == _cardIndexById.end())
    {
        return nullptr;
    }
    return &_cards[iter->second];
}

const Card* GameModel::getCardById(int cardId) const
{
    auto iter = _cardIndexById.find(cardId);
    if (iter == _cardIndexById.end())
    {
        return nullptr;
    }
    return &_cards[iter->second];
}

std::vector<int>& GameModel::getPlayfieldCardIds()
{
    return _playfieldCardIds;
}

const std::vector<int>& GameModel::getPlayfieldCardIds() const
{
    return _playfieldCardIds;
}

std::vector<int>& GameModel::getStockCardIds()
{
    return _stockCardIds;
}

const std::vector<int>& GameModel::getStockCardIds() const
{
    return _stockCardIds;
}

std::vector<int>& GameModel::getWastePileIds()
{
    return _wastePileIds;
}

const std::vector<int>& GameModel::getWastePileIds() const
{
    return _wastePileIds;
}

Card* GameModel::addCard(const LevelCardConfig& config, bool isPlayfieldCard)
{
    const int newId = generateCardId(_cards.size());

    Card card;
    card.id = newId;
    const int faceValue = config.cardFace >= 0 ? std::min(config.cardFace, 12) : 0;
    const int suitValue = config.cardSuit >= 0 ? std::min(config.cardSuit, 3) : 0;
    card.face = static_cast<CardFaceType>(faceValue);
    card.suit = static_cast<CardSuit>(suitValue);
    card.position = config.position;
    card.faceUp = config.faceUp;
    card.removed = false;
    card.isInPlayfield = isPlayfieldCard;
    card.coveredByCardIds = config.coveredBy;

    const std::size_t index = _cards.size();
    _cards.emplace_back(std::move(card));
    _cardIndexById.emplace(newId, index);

    if (isPlayfieldCard)
    {
        _playfieldCardIds.emplace_back(newId);
    }
    else
    {
        _stockCardIds.emplace_back(newId);
    }

    return &_cards.back();
}

int GameModel::getPlayfieldIndex(int cardId) const
{
    const auto iter = std::find(_playfieldCardIds.begin(), _playfieldCardIds.end(), cardId);
    if (iter == _playfieldCardIds.end())
    {
        return -1;
    }
    return static_cast<int>(std::distance(_playfieldCardIds.begin(), iter));
}

bool GameModel::isCardExposed(int cardId) const
{
    const Card* card = getCardById(cardId);
    if (!card || card->removed)
    {
        return false;
    }

    for (int coveringId : card->coveredByCardIds)
    {
        const Card* coveringCard = getCardById(coveringId);
        if (coveringCard && !coveringCard->removed)
        {
            return false;
        }
    }

    return true;
}

bool GameModel::isCardRemoved(int cardId) const
{
    const Card* card = getCardById(cardId);
    return !card || card->removed;
}

void GameModel::setCardFaceUp(int cardId, bool faceUp)
{
    Card* card = getCardById(cardId);
    if (!card)
    {
        return;
    }
    card->faceUp = faceUp;
}

void GameModel::setCardRemoved(int cardId, bool removed)
{
    Card* card = getCardById(cardId);
    if (!card)
    {
        return;
    }
    card->removed = removed;
}

void GameModel::removeCardFromPlayfield(int cardId)
{
    setCardRemoved(cardId, true);

    auto iter = std::find(_playfieldCardIds.begin(), _playfieldCardIds.end(), cardId);
    if (iter != _playfieldCardIds.end())
    {
        _playfieldCardIds.erase(iter);
    }
}

void GameModel::restoreCardToPlayfield(int cardId, int insertIndex)
{
    setCardRemoved(cardId, false);

    if (insertIndex < 0 || insertIndex > static_cast<int>(_playfieldCardIds.size()))
    {
        insertIndex = static_cast<int>(_playfieldCardIds.size());
    }

    _playfieldCardIds.insert(_playfieldCardIds.begin() + insertIndex, cardId);
}

void GameModel::setTrayCard(int cardId)
{
    _trayCardId = cardId;
}

int GameModel::getTrayCardId() const
{
    return _trayCardId;
}

int GameModel::replaceTrayCard(int newCardId)
{
    const int oldCardId = _trayCardId;
    _trayCardId = newCardId;
    return oldCardId;
}

int GameModel::drawCardFromStock()
{
    if (_stockCardIds.empty())
    {
        return -1;
    }

    const int cardId = _stockCardIds.back();
    _stockCardIds.pop_back();
    return cardId;
}

void GameModel::returnCardToStock(int cardId)
{
    _stockCardIds.emplace_back(cardId);
}

void GameModel::setCardFaceAndSuit(int cardId, CardFaceType face, CardSuit suit)
{
    Card* card = getCardById(cardId);
    if (!card)
    {
        return;
    }
    card->face = face;
    card->suit = suit;
}

void GameModel::rebuildCoveringRelations()
{
    for (Card& card : _cards)
    {
        card.coveringCardIds.clear();
    }

    for (const Card& card : _cards)
    {
        for (int coveringId : card.coveredByCardIds)
        {
            Card* coveringCard = getCardById(coveringId);
            if (coveringCard)
            {
                coveringCard->coveringCardIds.emplace_back(card.id);
            }
        }
    }
}

bool GameModel::isVictory() const
{
    return _playfieldCardIds.empty();
}

} // namespace tripeaks


