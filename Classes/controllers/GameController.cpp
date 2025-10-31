#include "controllers/GameController.h"

#include "views/GameView.h"

namespace tripeaks
{

bool GameController::init(GameView* view, const std::string& levelPath)
{
    _view = view;

    std::string errorMessage;
    if (!GameModelFromLevelGenerator::generateFromLevel(levelPath, _model, &errorMessage))
    {
        if (_view)
        {
            _view->showStatusMessage(errorMessage.empty() ? "Failed to load level" : errorMessage);
        }
        return false;
    }

    if (_view)
    {
        _view->bindModel(&_model);
        _view->setCardTapCallback([this](int cardId) { onCardTapped(cardId); });
        _view->setStockTapCallback([this]() { onStockTapped(); });
        _view->setUndoCallback([this]() { onUndoTapped(); });
        _view->buildInitialLayout();
    }

    _undoManager.clear();
    _playfieldController.initialize(&_model, _view);
    _stackController.initialize(&_model, _view);

    if (!_stackController.drawInitialCard())
    {
        if (_view)
        {
            _view->showStatusMessage("No card available to draw");
        }
    }
    _undoManager.clear();

    refreshCardStates();
    updateStockView();
    handleVictoryCheck();

    return true;
}

void GameController::onCardTapped(int cardId)
{
    UndoMove move;
    if (!_playfieldController.handleCardTap(cardId, move))
    {
        return;
    }

    _undoManager.push(move);
    handleVictoryCheck();
}

void GameController::onStockTapped()
{
    UndoMove move;
    if (!_stackController.handleStockTap(move))
    {
        return;
    }

    _undoManager.push(move);
    handleVictoryCheck();
}

void GameController::onUndoTapped()
{
    UndoMove move;
    if (!_undoManager.pop(move))
    {
        if (_view)
        {
            _view->showStatusMessage("Nothing to undo");
        }
        return;
    }

    switch (move.type)
    {
    case UndoMove::Type::PlayfieldMatch:
        _playfieldController.undoMatch(move);
        break;
    case UndoMove::Type::ReplaceTrayFromStock:
        _stackController.undoDraw(move);
        break;
    default:
        break;
    }

    handleVictoryCheck();
}

void GameController::refreshCardStates()
{
    if (_view)
    {
        _view->refreshCardStates();
    }
}

void GameController::updateStockView()
{
    if (_view)
    {
        _view->layoutStock();
    }
}

void GameController::handleVictoryCheck()
{
    if (!_view)
    {
        return;
    }

    if (_model.isVictory())
    {
        _view->showVictory();
    }
    else
    {
        _view->hideVictory();
    }

    _view->refreshCardStates();
}

} // namespace tripeaks


