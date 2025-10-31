#pragma once

#include "controllers/PlayFieldController.h"
#include "controllers/StackController.h"
#include "managers/UndoManager.h"
#include "services/GameModelFromLevelGenerator.h"

#include <string>

namespace tripeaks
{

class GameController
{
public:
    bool init(GameView* view, const std::string& levelPath);

    void onCardTapped(int cardId);
    void onStockTapped();
    void onUndoTapped();

    GameModel& getModel() { return _model; }
    const GameModel& getModel() const { return _model; }

private:
    void refreshCardStates();
    void updateStockView();
    void handleVictoryCheck();

    GameModel _model;
    UndoManager _undoManager;
    PlayFieldController _playfieldController;
    StackController _stackController;
    GameView* _view = nullptr;
};

} // namespace tripeaks


