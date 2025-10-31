#include "services/GameModelFromLevelGenerator.h"

#include "services/CardMatchService.h"

#include "cocos2d.h"

#include <algorithm>

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

CardFaceType toFaceType(int value)
{
    value = clampValue(value, 0, 12);
    return static_cast<CardFaceType>(value);
}

CardSuit toSuitType(int value)
{
    value = clampValue(value, 0, 3);
    return static_cast<CardSuit>(value);
}

void assignFaceAndSuit(GameModel& model, int cardId, const LevelCardConfig& config)
{
    if (config.cardFace >= 0 && config.cardSuit >= 0)
    {
        model.setCardFaceAndSuit(cardId, toFaceType(config.cardFace), toSuitType(config.cardSuit));
        return;
    }

    if (config.cardFace >= 0 && config.cardSuit < 0)
    {
        const int suit = cocos2d::random(0, 3);
        model.setCardFaceAndSuit(cardId, toFaceType(config.cardFace), toSuitType(suit));
        return;
    }

    if (config.cardFace < 0 && config.cardSuit >= 0)
    {
        const int face = cocos2d::random(0, 12);
        model.setCardFaceAndSuit(cardId, toFaceType(face), toSuitType(config.cardSuit));
        return;
    }

    CardMatchService::randomizeCard(model, cardId);
}

} // namespace

bool GameModelFromLevelGenerator::generateFromLevel(const std::string& configPath,
                                                    GameModel& outModel,
                                                    std::string* errorMessage)
{
    LevelConfig levelConfig;
    if (!LevelConfigLoader::loadFromFile(configPath, levelConfig, errorMessage))
    {
        return false;
    }

    outModel.reset();

    for (const LevelCardConfig& cardConfig : levelConfig.playfieldCards)
    {
        Card* card = outModel.addCard(cardConfig, true);
        if (card)
        {
            assignFaceAndSuit(outModel, card->id, cardConfig);
        }
    }

    for (const LevelCardConfig& cardConfig : levelConfig.stackCards)
    {
        Card* card = outModel.addCard(cardConfig, false);
        if (card)
        {
            assignFaceAndSuit(outModel, card->id, cardConfig);
        }
    }

    outModel.rebuildCoveringRelations();

    return true;
}

} // namespace tripeaks


