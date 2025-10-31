#include "configs/loaders/LevelConfigLoader.h"

#include "cocos2d.h"

#include "json/document.h"
#include <utility>

namespace tripeaks
{
namespace
{

bool hasMember(const rapidjson::Value& value, const char* keyLower, const char* keyUpper)
{
    return value.HasMember(keyLower) || value.HasMember(keyUpper);
}

const rapidjson::Value& getMember(const rapidjson::Value& value, const char* keyLower, const char* keyUpper)
{
    if (value.HasMember(keyLower))
    {
        return value[keyLower];
    }
    return value[keyUpper];
}

bool parseCardArray(const rapidjson::Value& jsonArray, std::vector<LevelCardConfig>& outCards)
{
    if (!jsonArray.IsArray())
    {
        return false;
    }

    outCards.clear();
    outCards.reserve(jsonArray.Size());

    for (rapidjson::SizeType i = 0; i < jsonArray.Size(); ++i)
    {
        const auto& cardValue = jsonArray[i];
        if (!cardValue.IsObject())
        {
            return false;
        }

        LevelCardConfig config;

        if (hasMember(cardValue, "cardFace", "CardFace"))
        {
            const auto& faceValue = getMember(cardValue, "cardFace", "CardFace");
            if (faceValue.IsInt())
            {
                config.cardFace = faceValue.GetInt();
            }
        }

        if (hasMember(cardValue, "cardSuit", "CardSuit"))
        {
            const auto& suitValue = getMember(cardValue, "cardSuit", "CardSuit");
            if (suitValue.IsInt())
            {
                config.cardSuit = suitValue.GetInt();
            }
        }

        if (hasMember(cardValue, "faceUp", "FaceUp"))
        {
            const auto& faceUpValue = getMember(cardValue, "faceUp", "FaceUp");
            if (faceUpValue.IsBool())
            {
                config.faceUp = faceUpValue.GetBool();
            }
        }

        if (hasMember(cardValue, "position", "Position"))
        {
            const auto& posValue = getMember(cardValue, "position", "Position");
            if (posValue.IsObject())
            {
                float x = 0.0F;
                float y = 0.0F;
                if (hasMember(posValue, "x", "X"))
                {
                    const auto& xValue = getMember(posValue, "x", "X");
                    if (xValue.IsNumber())
                    {
                        x = static_cast<float>(xValue.GetDouble());
                    }
                }
                if (hasMember(posValue, "y", "Y"))
                {
                    const auto& yValue = getMember(posValue, "y", "Y");
                    if (yValue.IsNumber())
                    {
                        y = static_cast<float>(yValue.GetDouble());
                    }
                }
                config.position = cocos2d::Vec2(x, y);
            }
        }

        if (hasMember(cardValue, "coveredBy", "CoveredBy"))
        {
            const auto& coveredByValue = getMember(cardValue, "coveredBy", "CoveredBy");
            if (coveredByValue.IsArray())
            {
                for (rapidjson::SizeType j = 0; j < coveredByValue.Size(); ++j)
                {
                    const auto& idValue = coveredByValue[j];
                    if (idValue.IsInt())
                    {
                        config.coveredBy.emplace_back(idValue.GetInt());
                    }
                }
            }
        }

        outCards.emplace_back(std::move(config));
    }

    return true;
}

} // namespace

bool LevelConfigLoader::loadFromFile(const std::string& filePath,
                                     LevelConfig& outConfig,
                                     std::string* errorMessage)
{
    using cocos2d::FileUtils;

    auto fileUtils = FileUtils::getInstance();
    if (!fileUtils->isFileExist(filePath))
    {
        if (errorMessage)
        {
            *errorMessage = "Config file does not exist: " + filePath;
        }
        return false;
    }

    std::string fileData = fileUtils->getStringFromFile(filePath);
    if (fileData.empty())
    {
        if (errorMessage)
        {
            *errorMessage = "Config file is empty: " + filePath;
        }
        return false;
    }

    rapidjson::Document document;
    document.Parse(fileData.c_str());
    if (document.HasParseError() || !document.IsObject())
    {
        if (errorMessage)
        {
            *errorMessage = "Config file parse error: " + filePath;
        }
        return false;
    }

    if (!document.HasMember("playfieldCards") || !document["playfieldCards"].IsArray())
    {
        if (errorMessage)
        {
            *errorMessage = "Missing playfieldCards definition";
        }
        return false;
    }

    if (!parseCardArray(document["playfieldCards"], outConfig.playfieldCards))
    {
        if (errorMessage)
        {
            *errorMessage = "Invalid playfieldCards data";
        }
        return false;
    }

    if (document.HasMember("stackCards"))
    {
        if (!parseCardArray(document["stackCards"], outConfig.stackCards))
        {
            if (errorMessage)
            {
                *errorMessage = "Invalid stackCards data";
            }
            return false;
        }
    }
    else
    {
        outConfig.stackCards.clear();
    }

    return true;
}

} // namespace tripeaks


