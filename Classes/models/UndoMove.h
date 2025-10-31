#pragma once

#include <vector>

namespace tripeaks
{

struct CardFlipState
{
    int cardId = -1;
    bool previousFaceUp = false;
};

struct UndoMove
{
    enum class Type
    {
        PlayfieldMatch,      // 桌面牌匹配替换手牌区顶部牌
        ReplaceTrayFromStock // 从备用牌堆翻牌替换手牌区顶部牌
    };

    Type type = Type::PlayfieldMatch;
    int movedCardId = -1;              // 移动的卡牌ID（桌面牌或stock牌）
    int previousTrayCardId = -1;       // 之前的手牌区顶部牌ID; -1表示无牌
    int previousPlayfieldIndex = -1;   // 桌面牌在playfield数组中的位置（仅PlayfieldMatch类型有效）
    int previousStockIndex = -1;       // stock牌在stock数组中的位置（仅ReplaceTrayFromStock类型有效）
    std::vector<CardFlipState> flipStates; // 自动翻开的卡牌状态
};

} // namespace tripeaks


