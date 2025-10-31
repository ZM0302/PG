#pragma once

#include "managers/UndoManager.h"
#include "models/GameModel.h"
#include "views/GameView.h"

namespace tripeaks
{

class PlayFieldController
{
public:
    void initialize(GameModel* model, GameView* view);

    bool handleCardTap(int cardId, UndoMove& outMove);
    void undoMatch(const UndoMove& move);

private:
    GameModel* _model = nullptr;
    GameView* _view = nullptr;
};

} // namespace tripeaks


