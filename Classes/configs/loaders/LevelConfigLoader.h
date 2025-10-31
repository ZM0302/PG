#pragma once

#include "configs/models/LevelConfig.h"

#include <string>

namespace tripeaks
{

class LevelConfigLoader
{
public:
    static bool loadFromFile(const std::string& filePath,
                             LevelConfig& outConfig,
                             std::string* errorMessage = nullptr);
};

} // namespace tripeaks


