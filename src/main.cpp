#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "objects.hpp"

using namespace geode::prelude;

std::vector<std::string> splitString(const std::string& s, const std::string& delimiter, bool skipEmpty);
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
		GJGameLevel *level1 = this->m_level; // nerfed
		GJGameLevel *level2 = gameLevelManager->getSavedLevel(88201288); // buffed
		std::vector<GJGameLevel*> levels = { level1, level2 };
		int count = 0;
		std::vector<std::string> levelStringSplit1, levelStringSplit2;
		std::string firstElement;

		for (GJGameLevel *level : levels) {
			std::string levelString = ZipUtils::decompressString(level->m_levelString, false, 0);

			std::vector<std::string> levelStringSplit = splitString(levelString, ";", true);
			firstElement = levelStringSplit.front();
			levelStringSplit.erase(levelStringSplit.begin());

			for (std::string& objectStr : levelStringSplit) {
				bool isDecoration = false;
				bool hasLayer1 = false; // 20
				bool hasColor1 = false; // 21
				bool hasColor2 = false; // 22
				bool hasLayer2 = false; // 61
				bool dontFade = false;  // 64
				bool dontEnter = false; // 67
				bool noGlow = false;    // 96
				bool customRotationSpeed = false; // 97
				bool noParticle = false; // 507
				std::string newObjectStr = "";

				std::vector<std::string> splitStrings = splitString(objectStr, ",", true);
				std::vector<std::vector<std::string>> splitStringsPairs;
				for (int i = 0; i < splitStrings.size() - 1; i += 2) {
					splitStringsPairs.push_back({ splitStrings[i], splitStrings[i + 1] });
				}

				log::info("Object before: {}", objectStr);

				for (std::vector<std::string>& pair : splitStringsPairs) {
					int propID = std::stoi(pair[0]);

					if (propID == 1) { // id
						if (std::find(objects.begin(), objects.end(), std::stoi(pair[1])) == objects.end()) { // Check if object is decoration
							isDecoration = true;
							objectStr = "";
							break;
						} else {
							int objID = std::stoi(pair[1]);

							for (const auto& objPair : blackObjects) {
								if (objID == objPair[0]) {
									objID = objPair[1];
									pair[1] = std::to_string(objID);
									break;
								}
							}

							newObjectStr += "1," + pair[1] + ",";
							continue;
						}
					}

					if (isDecoration) break;

					switch (propID) {
						case 20: // editor layer 1
							hasLayer1 = true;
							count == 0 ? newObjectStr += "20,1," : newObjectStr += "20,2,";
							continue;
						case 21: // color 1
							hasColor1 = true;
							count == 0 ? newObjectStr += "21,1," : newObjectStr += "21,2,";
							continue;
						case 22: // color 2
							hasColor2 = true;
							count == 0 ? newObjectStr += "22,1," : newObjectStr += "22,2,";
							continue;
						case 43: // delete HSV 1
							continue;
						case 44: // delete HSV 2
							continue;
						case 61: // editor layer 2
							hasLayer2 = true;
							count == 0 ? newObjectStr += "61,1," : newObjectStr += "61,2,";
							continue;
						case 64: // don't fade
							dontFade = true;
							newObjectStr += "64,1,";
							continue;
						case 67: // don't enter
							dontEnter = true;
							newObjectStr += "67,1,";
							continue;
						case 96: // no glow
							noGlow = true;
							newObjectStr += "96,1,";
							continue;
						case 97: // custom rotation speed
							customRotationSpeed = true;
							newObjectStr += "97,69.000000,";
							continue;
						case 98: // turn off disable rotation
							continue;
						case 103: // turn off high detail
							continue;
						case 507: // no particle
							noParticle = true;
							newObjectStr += "507,1,";
							continue; 
						default:
							newObjectStr += pair[0] + "," + pair[1] + ",";
							continue;
					}
				}


				if (!hasLayer1) { count == 0 ? newObjectStr += "20,1," : newObjectStr += "20,2,"; }
				if (!hasColor1) { count == 0 ? newObjectStr += "21,1," : newObjectStr += "21,2,"; }
				if (!hasColor2) { count == 0 ? newObjectStr += "22,1," : newObjectStr += "22,2,"; }
				if (!hasLayer2) { count == 0 ? newObjectStr += "61,1," : newObjectStr += "61,2,"; }
				if (!dontFade) newObjectStr += "64,1,";
				if (!dontEnter) newObjectStr += "67,1,";
				if (!noGlow) newObjectStr += "96,1,";
				if (!customRotationSpeed) newObjectStr += "97,69.000000,";
				if (!noParticle) newObjectStr += "507,1,";

				if (!newObjectStr.empty()) {
					newObjectStr.pop_back(); // Remove the last comma
					objectStr = newObjectStr;
				} else {
					objectStr = "";
				}

				log::info("Object after: {}", objectStr);
			}

			count == 0 ? levelStringSplit1 = levelStringSplit : levelStringSplit2 = levelStringSplit;
			count++;
		}

		std::vector<std::string> firstElementSplit = splitString(firstElement, ",", false);
		std::vector<std::vector<std::string>> firstElementPairs;
		for (int i = 0; i < firstElementSplit.size() - 1; i += 2) {
			firstElementPairs.push_back({ firstElementSplit[i], firstElementSplit[i + 1] });
		}
		log::info("{}", firstElement);
		log::info("{}", firstElementPairs);

		std::string newFirstElement;
		for (std::vector<std::string>& pair : firstElementPairs) {
			if (pair[0] == "kA6") { // background texture
				newFirstElement += "kA6,13,";
			}
			else if (pair[0] == "kA7") { // ground texture
				newFirstElement += "kA7,1,";
			}
			else if (pair[0] == "kA17") { // ground line
				newFirstElement += "kA17,1,";
			}
			else if (pair[0] == "kA18") { // font
				newFirstElement += "kA18,6,";
			}
			else if (pair[0] == "kA25") { // middleground texture
				newFirstElement += "kA25,0,";
			}
			else if (pair[0] == "kS38") { // colors
				newFirstElement += "kS38,1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1000_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1001_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1009_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1002_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1013_7_1.000000_15_1.000000_18_0_8_1|1_0_2_0_3_0_11_255_12_255_13_255_4_-1_6_1014_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1005_5_1_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1006_5_1_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1004_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1007_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1003_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1012_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1010_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1011_7_1.000000_15_1.000000_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_1_5_1_7_1.000000_15_1.000000_9_3_10_180.000000a1.000000a1.000000a1a1_18_0_8_1|1_255_2_255_3_255_11_255_12_255_13_255_4_-1_6_2_5_1_7_1.000000_15_1.000000_9_3_10_0.000000a1.000000a1.000000a1a1_18_0_8_1|,";
			}
			else if (pair[0] == "kS39") { // color page
				newFirstElement += "kS39,0,";
			}
			else {
				newFirstElement += pair[0] + "," + pair[1] + ",";
			}
		}
		
		std::string modifiedLevelString = newFirstElement + ";" + joinString(levelStringSplit1, ";") + joinString(levelStringSplit2, ";");
		
		
		FLAlertLayer::create(
			"Level Comparison",
			std::string("Successfully created layout of ") + level1->m_levelName.c_str(),
			"OK"
		)->show();
		

		auto menu = CCMenu::create();
		CCMenuItemToggle toggle1;

		// log::info("{}", levelString);
		// log::info("{}", modifiedLevelString);


		// create new level
        GJGameLevel* newLevel = gameLevelManager->createNewLevel();
        newLevel->m_levelName = "Test " + level1->m_levelName;
        newLevel->m_levelString = modifiedLevelString;
		newLevel->m_levelDesc = ZipUtils::base64URLEncode("Comparison of " + level1->m_levelName + " and " + level2->m_levelName);
		newLevel->m_songID = level1->m_songID;
    }
};

std::vector<std::string> splitString(const std::string& s, const std::string& delimiter, bool skipEmpty) {
    std::vector<std::string> splitStrings;
    size_t pos = 0, found;
    while ((found = s.find(delimiter, pos)) != std::string::npos) {
        std::string token = s.substr(pos, found - pos);
        if (!skipEmpty || !token.empty()) {
            splitStrings.push_back(token);
        }
        pos = found + delimiter.length();
    }
    // Handle the last token
    std::string lastToken = s.substr(pos);
    if (!skipEmpty || !lastToken.empty()) {
        splitStrings.push_back(lastToken);
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
