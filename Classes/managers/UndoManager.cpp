#include "managers/UndoManager.h"

#include <utility>

namespace tripeaks
{

void UndoManager::clear()
{
    _moves.clear();
}

void UndoManager::push(const UndoMove& move)
{
    _moves.emplace_back(move);
}

bool UndoManager::canUndo() const
{
    return !_moves.empty();
}

bool UndoManager::pop(UndoMove& outMove)
{
    if (_moves.empty())
    {
        return false;
    }

    UndoMove move = _moves.back();
    _moves.pop_back();
    outMove = std::move(move);
    return true;
}

const UndoMove* UndoManager::peek() const
{
    if (_moves.empty())
    {
        return nullptr;
    }
    return &_moves.back();
}

} // namespace tripeaks


