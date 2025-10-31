#pragma once

#include "cocos2d.h"

#include <vector>

namespace tripeaks
{

enum class CardSuit
{
    Clubs = 0,
    Diamonds = 1,
    Hearts = 2,
    Spades = 3
};

enum class CardFaceType
{
    Ace = 0,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Ten,
    Jack,
    Queen,
    King
};

struct LevelCardConfig
{
    int cardFace = -1;                     // 0~12 maps to A~K, -1 means random
    int cardSuit = -1;                     // 0~3 maps to suits, -1 means random
    cocos2d::Vec2 position;                // card position in the scene
    bool faceUp = false;                   // initial face-up state
    std::vector<int> coveredBy;            // IDs of cards covering this card
};

struct LevelConfig
{
    std::vector<LevelCardConfig> playfieldCards; // cards placed on the playfield
    std::vector<LevelCardConfig> stackCards;     // cards in the stock pile
};

} // namespace tripeaks


