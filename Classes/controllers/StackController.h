#pragma once

#include "managers/UndoManager.h"
#include "models/GameModel.h"
#include "views/GameView.h"

namespace tripeaks
{

class StackController
{
public:
    void initialize(GameModel* model, GameView* view);

    bool handleStockTap(UndoMove& outMove);
    void undoDraw(const UndoMove& move);

    bool drawInitialCard();

private:
    GameModel* _model = nullptr;
    GameView* _view = nullptr;
};

} // namespace tripeaks


