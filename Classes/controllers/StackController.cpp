#include "controllers/StackController.h"

namespace tripeaks
{

void StackController::initialize(GameModel* model, GameView* view)
{
    _model = model;
    _view = view;
}

bool StackController::handleStockTap(UndoMove& outMove)
{
    if (!_model || !_view)
    {
        return false;
    }

    if (_model->getStockCardIds().empty())
    {
        _view->showStatusMessage("Stock is empty");
        return false;
    }

    // 获取stock中最后一张牌的索引（用于回退）
    const int stockIndex = static_cast<int>(_model->getStockCardIds().size()) - 1;
    const int cardId = _model->drawCardFromStock();
    
    // 记录当前手牌区顶部牌
    const int previousTrayCardId = _model->getTrayCardId();

    // 记录回退信息
    outMove = UndoMove{};
    outMove.type = UndoMove::Type::ReplaceTrayFromStock;
    outMove.movedCardId = cardId;
    outMove.previousTrayCardId = previousTrayCardId;
    outMove.previousStockIndex = stockIndex;

    // 替换手牌区顶部牌
    _model->setCardFaceUp(cardId, true);
    _model->replaceTrayCard(cardId);

    // 执行动画：stock牌平移到手牌区替换顶部牌
    _view->replaceTrayCardWithStockCard(cardId, previousTrayCardId, true);

    _view->layoutStock();
    _view->refreshCardStates();

    return true;
}

void StackController::undoDraw(const UndoMove& move)
{
    if (!_model || !_view)
    {
        return;
    }

    if (move.type != UndoMove::Type::ReplaceTrayFromStock)
    {
        return;
    }

    const int cardId = move.movedCardId;
    
    // 恢复手牌区顶部牌
    _model->setTrayCard(move.previousTrayCardId);
    
    // 恢复stock牌
    _model->setCardFaceUp(cardId, false);
    _model->returnCardToStock(cardId);

    // 执行回退动画：手牌区牌平移回stock
    _view->undoReplaceTrayCardFromStock(cardId, move.previousTrayCardId, move.previousStockIndex, true);

    _view->layoutStock();
    _view->refreshCardStates();
}

bool StackController::drawInitialCard()
{
    if (!_model || !_view)
    {
        return false;
    }

    const int cardId = _model->drawCardFromStock();
    if (cardId < 0)
    {
        return false;
    }

    _model->setCardFaceUp(cardId, true);
    _model->setTrayCard(cardId);

    _view->layoutStock();
    _view->placeInitialTrayCard(cardId);
    _view->refreshCardStates();

    return true;
}

} // namespace tripeaks


