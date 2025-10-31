#pragma once

#include "models/GameModel.h"

#include <vector>

namespace tripeaks
{

class CardMatchService
{
public:
    static bool canMatch(CardFaceType faceA, CardFaceType faceB);

    static bool hasMatchableCardInPlayfield(const GameModel& model, CardFaceType face);

    static std::vector<CardFaceType> getMatchableFacesInPlayfield(const GameModel& model);

    static int countFaceInPlayfield(const GameModel& model, CardFaceType face);

    static void randomizeCard(GameModel& model, int cardId);
};

} // namespace tripeaks


