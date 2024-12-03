#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include "objects.hpp"

using namespace geode::prelude;

std::vector<std::string> splitString(const std::string& s, const std::string& delimiter);
std::string joinString(const std::vector<std::string>& elems, const std::string& delimiter);

class $modify(MakeLevelLayoutLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge))
            return false;
        
        auto menu = this->getChildByID("left-side-menu");
        if (menu) {
            auto btn = CCMenuItemSpriteExtra::create(
                CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png"),
                this, menu_selector(MakeLevelLayoutLayer::onButton)
            );
            btn->setID("export-button"_spr);
            menu->addChild(btn);
            menu->updateLayout();
        }

        return true;
    }

    void onButton(CCObject*) {
		GameLevelManager *gameLevelManager = GameLevelManager::sharedState();
		GJGameLevel *level = this->m_level;
		// GJGameLevel *level = gameLevelManager->getSavedLevel(26681070);
		std::string levelString = ZipUtils::decompressString(level->m_levelString, false, 0);
		log::info("{}", levelString);

		std::vector<std::string> levelStringSplit = splitString(levelString, ";");
		std::string firstElement = levelStringSplit.front();
		levelStringSplit.erase(levelStringSplit.begin());

		for (std::string& objectStr : levelStringSplit) {
			bool isDecoration = false;
			bool dontFade = false;  // 64
			bool dontEnter = false; // 67
			bool customRotationSpeed = false; // 97
			std::string newObjectStr = "";

			std::vector<std::string> splitStrings = splitString(objectStr, ",");
			std::vector<std::vector<std::string>> splitStringsPairs;
			for (int i = 0; i < splitStrings.size() - 1; i += 2) {
				splitStringsPairs.push_back({ splitStrings[i], splitStrings[i + 1] });
			}

			// log::info("Object before: {}", objectStr);

			for (std::vector<std::string>& pair : splitStringsPairs) {
				int propID = std::stoi(pair[0]);

				if (propID == 1) { // id
					if (std::find(objects.begin(), objects.end(), std::stoi(pair[1])) == objects.end()) { // Check if object is decoration
						isDecoration = true;
						objectStr = "";
						break;
					} else {
						newObjectStr += "1," + pair[1] + ",";
						continue;
					}
				}

				if (isDecoration) break;

				switch (propID) {
					case 21: // color 1 to channel 1
						newObjectStr += "21,1,";
						continue;
					case 22: // color 2 to channel 1
						newObjectStr += "22,1,";
						continue;
					case 43: // delete HSV 1
						continue;
					case 44: // delete HSV 2
						continue;
					case 64: // don't fade
						dontFade = true;
						newObjectStr += "64,1,";
						continue;
					case 67: // don't enter
						dontEnter = true;
						newObjectStr += "67,1,";
						continue;
					case 97: // custom rotation speed
						customRotationSpeed = true;
						newObjectStr += "97,69.000000,";
						continue;
					case 98: // turn off disable rotation
						continue;
					case 103: // turn off high detail
						continue;
					default:
						newObjectStr += pair[0] + "," + pair[1] + ",";
						continue;
				}
			}

			if (!dontFade) newObjectStr += "64,1,";
			if (!dontEnter) newObjectStr += "67,1,";
			if (!customRotationSpeed) newObjectStr += "97,69.000000,";

			if (!newObjectStr.empty()) {
				newObjectStr.pop_back(); // Remove the last comma
				objectStr = newObjectStr;
			} else {
				objectStr = "";
			}

			// log::info("Object after: {}", objectStr);
		}

		std::string newFirstElement = "kS38,1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1000_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1001_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1009_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1002_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1013_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1014_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1005_5_1_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1006_5_1_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1004_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1007_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1003_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1012_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1010_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1011_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1_5_1_7_1.000000_15_1.000000_9_3_10_180.000000a1.000000a1.000000a1a1_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_2_5_1_7_1.000000_15_1.000000_9_3_10_0.000000a1.000000a1.000000a1a1_18_0_8_1|,kA13,0.000000,kA15,0,kA16,0,kA14,,kA6,13,kA7,1,kA25,0,kA17,1,kA18,6,kS39,0,kA2,0,kA3,0,kA8,0,kA4,0,kA9,0,kA10,0,kA22,0,kA23,0,kA24,0,kA27,1,kA40,1,kA41,1,kA42,1,kA28,0,kA29,0,kA31,1,kA32,1,kA36,0,kA43,0,kA44,0,kA45,1,kA46,0,kA33,1,kA34,1,kA35,0,kA37,1,kA38,1,kA39,1,kA19,0,kA26,0,kA20,0,kA21,0,kA11,0";
		std::string modifiedLevelString = newFirstElement + ";" + joinString(levelStringSplit, ";");
		
		FLAlertLayer::create(
			"Level Comparison",
			std::string("Successfully created layout of ") + level->m_levelName.c_str(),
			"OK"
		)->show();

		// log::info("{}", levelString);
		// log::info("{}", modifiedLevelString);


		// create new level
        GJGameLevel* newLevel = gameLevelManager->createNewLevel();
        newLevel->m_levelName = "Test " + level->m_levelName;
        newLevel->m_levelString = modifiedLevelString;
		newLevel->m_levelDesc = ZipUtils::base64URLEncode("Comparison of " + level->m_levelName);
		newLevel->m_songID = level->m_songID;
    }
};

std::vector<std::string> splitString(const std::string& s, const std::string& delimiter) {
    std::vector<std::string> splitStrings;
    size_t pos = 0, found;
    while ((found = s.find(delimiter, pos)) != std::string::npos) {
        if (found != pos) { // Avoid adding empty strings
            splitStrings.push_back(s.substr(pos, found - pos));
        }
        pos = found + delimiter.length();
    }
    if (pos < s.size()) {
        splitStrings.push_back(s.substr(pos));
    }
    return splitStrings;
}

std::string joinString(const std::vector<std::string>& elems, const std::string& delimiter) {
    std::stringstream ss;
    for (size_t i = 0; i < elems.size(); ++i) {
        if (i != 0)
            ss << delimiter;
        ss << elems[i];
    }
    return ss.str();
}
