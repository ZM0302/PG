#pragma once

#include "models/UndoMove.h"

#include <vector>

namespace tripeaks
{

class UndoManager
{
public:
    void clear();
    void push(const UndoMove& move);
    bool canUndo() const;
    bool pop(UndoMove& outMove);
    const UndoMove* peek() const;

private:
    std::vector<UndoMove> _moves;
};

} // namespace tripeaks


