#pragma once

#include "configs/loaders/LevelConfigLoader.h"
#include "models/GameModel.h"

#include <string>

namespace tripeaks
{

class GameModelFromLevelGenerator
{
public:
    static bool generateFromLevel(const std::string& configPath,
                                  GameModel& outModel,
                                  std::string* errorMessage = nullptr);
};

} // namespace tripeaks


