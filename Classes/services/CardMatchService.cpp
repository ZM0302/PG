#include "services/CardMatchService.h"

#include "cocos2d.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace tripeaks
{

namespace
{

int toFaceIndex(CardFaceType face)
{
    return static_cast<int>(face);
}

CardFaceType fromFaceIndex(int index)
{
    index = (index % 13 + 13) % 13;
    return static_cast<CardFaceType>(index);
}

} // namespace

bool CardMatchService::canMatch(CardFaceType faceA, CardFaceType faceB)
{
    const int indexA = toFaceIndex(faceA);
    const int indexB = toFaceIndex(faceB);
    const int diff = std::abs(indexA - indexB);
    return diff == 1 || diff == 12;
}

bool CardMatchService::hasMatchableCardInPlayfield(const GameModel& model, CardFaceType face)
{
    const auto& playfieldIds = model.getPlayfieldCardIds();
    for (int cardId : playfieldIds)
    {
        const Card* card = model.getCardById(cardId);
        if (!card || card->removed || !card->faceUp)
        {
            continue;
        }

        if (!model.isCardExposed(cardId))
        {
            continue;
        }

        if (canMatch(face, card->face))
        {
            return true;
        }
    }
    return false;
}

std::vector<CardFaceType> CardMatchService::getMatchableFacesInPlayfield(const GameModel& model)
{
    std::unordered_set<int> faces;
    const auto& playfieldIds = model.getPlayfieldCardIds();
    for (int cardId : playfieldIds)
    {
        const Card* card = model.getCardById(cardId);
        if (!card || card->removed || !card->faceUp)
        {
            continue;
        }
        if (!model.isCardExposed(cardId))
        {
            continue;
        }
        faces.emplace(toFaceIndex(card->face));
    }

    std::vector<CardFaceType> result;
    result.reserve(faces.size());
    for (int faceIndex : faces)
    {
        result.emplace_back(fromFaceIndex(faceIndex));
    }
    return result;
}

int CardMatchService::countFaceInPlayfield(const GameModel& model, CardFaceType face)
{
    int count = 0;
    const auto targetIndex = toFaceIndex(face);
    const auto& playfieldIds = model.getPlayfieldCardIds();
    for (int cardId : playfieldIds)
    {
        const Card* card = model.getCardById(cardId);
        if (!card || card->removed)
        {
            continue;
        }
        if (toFaceIndex(card->face) == targetIndex)
        {
            ++count;
        }
    }
    return count;
}

void CardMatchService::randomizeCard(GameModel& model, int cardId)
{
    Card* card = model.getCardById(cardId);
    if (!card)
    {
        return;
    }

    const int faceIndex = cocos2d::random(0, 12);
    const int suitIndex = cocos2d::random(0, 3);
    model.setCardFaceAndSuit(cardId, static_cast<CardFaceType>(faceIndex), static_cast<CardSuit>(suitIndex));
}

} // namespace tripeaks


