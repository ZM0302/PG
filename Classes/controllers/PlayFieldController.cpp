#include "controllers/PlayFieldController.h"

#include "services/CardMatchService.h"

#include <algorithm>

namespace tripeaks
{

void PlayFieldController::initialize(GameModel* model, GameView* view)
{
    _model = model;
    _view = view;
}

bool PlayFieldController::handleCardTap(int cardId, UndoMove& outMove)
{
    if (!_model || !_view)
    {
        return false;
    }

    Card* card = _model->getCardById(cardId);
    if (!card || card->removed || !card->faceUp || !_model->isCardExposed(cardId))
    {
        return false;
    }

    const int trayCardId = _model->getTrayCardId();
    const Card* trayCard = _model->getCardById(trayCardId);
    if (!trayCard)
    {
        _view->showStatusMessage("Draw from the stock first");
        return false;
    }

    if (!CardMatchService::canMatch(card->face, trayCard->face))
    {
        _view->showStatusMessage("Card cannot match the tray");
        return false;
    }

    const int playfieldIndex = _model->getPlayfieldIndex(cardId);
    if (playfieldIndex < 0)
    {
        return false;
    }

    // 记录回退信息
    outMove = UndoMove{};
    outMove.type = UndoMove::Type::PlayfieldMatch;
    outMove.movedCardId = cardId;
    outMove.previousTrayCardId = trayCardId;
    outMove.previousPlayfieldIndex = playfieldIndex;

    // 替换手牌区顶部牌，并将旧的tray card移回stock
    const int oldTrayCardId = _model->replaceTrayCard(cardId);
    if (oldTrayCardId >= 0)
    {
        _model->returnCardToStock(oldTrayCardId);
    }
    
    // 从桌面移除卡牌
    _model->removeCardFromPlayfield(cardId);

    // 执行动画：桌面牌平移到手牌区替换顶部牌
    _view->replaceTrayCardWithPlayfieldCard(cardId, oldTrayCardId, true);

    // 处理自动翻开的卡牌
    Card* removedCard = _model->getCardById(cardId);
    if (removedCard)
    {
        for (int coveredId : removedCard->coveringCardIds)
        {
            Card* coveredCard = _model->getCardById(coveredId);
            if (!coveredCard || coveredCard->removed)
            {
                continue;
            }

            if (_model->isCardExposed(coveredId) && !coveredCard->faceUp)
            {
                outMove.flipStates.push_back({coveredId, coveredCard->faceUp});
                _model->setCardFaceUp(coveredId, true);
                _view->flipCard(coveredId, true);
            }
        }
    }

    _view->refreshCardStates();

    return true;
}

void PlayFieldController::undoMatch(const UndoMove& move)
{
    if (!_model || !_view)
    {
        return;
    }

    if (move.type != UndoMove::Type::PlayfieldMatch)
    {
        return;
    }

    const int cardId = move.movedCardId;
    
    // 恢复桌面牌
    _model->restoreCardToPlayfield(cardId, move.previousPlayfieldIndex);
    _model->setCardFaceUp(cardId, true);
    
    // 从stock移除当前的tray card（如果它在stock中）
    const int currentTrayCardId = _model->getTrayCardId();
    if (currentTrayCardId >= 0)
    {
        auto& stockIds = _model->getStockCardIds();
        auto iter = std::find(stockIds.begin(), stockIds.end(), currentTrayCardId);
        if (iter != stockIds.end())
        {
            stockIds.erase(iter);
        }
    }
    
    // 恢复手牌区顶部牌
    _model->setTrayCard(move.previousTrayCardId);

    // 执行回退动画：手牌区牌平移回桌面
    _view->undoReplaceTrayCard(cardId, move.previousTrayCardId, true);

    // 恢复自动翻开的卡牌状态
    for (const CardFlipState& flip : move.flipStates)
    {
        _model->setCardFaceUp(flip.cardId, flip.previousFaceUp);
        _view->flipCard(flip.cardId, flip.previousFaceUp);
    }

    _view->refreshCardStates();
}

} // namespace tripeaks


