#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <cocos2d.h>
#include <cocos-ext.h>
#include <functional>
#include "objects.hpp"
#include "data.hpp"


using namespace geode::prelude;
using namespace cocos2d;
using namespace cocos2d::extension;

std::string createComparison(GJGameLevel* level1, GJGameLevel* level2, const ComparisonConfig& config);
std::vector<std::string> splitString(const std::string& s, const std::string& delimiter, bool skipEmpty);
std::string joinString(const std::vector<std::string>& elems, const std::string& delimiter);

class ComparisonMenu : public CCLayer, public TextInputDelegate {
public:
	std::function<void(
		int targetLevelID,
		bool isBuffed,
		float sawRotationSpeed
	)> onCreateCallback;

    int targetLevelID = 0;
    bool isBuffed = false;
    float sawRotationSpeed = 0.f;

    CCLabelBMFont* speedLabel = nullptr;
    CCMenuItemToggle* buffedToggle = nullptr;
    CCMenuItemToggle* nerfedToggle = nullptr;

    static ComparisonMenu* create(std::function<void(int, bool, float)> onCreate) {
		auto ret = new ComparisonMenu();
		if (ret && ret->init()) {
			ret->onCreateCallback = onCreate;
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}


    bool init() override {
        if (!CCLayer::init()) return false;

		this->setTouchEnabled(true);
		this->setTouchMode(kCCTouchesOneByOne);
		this->setKeypadEnabled(true);

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // === LOAD SAVED VALUES ===
        auto mod = Mod::get();
        targetLevelID = mod->getSavedValue<int>("target-level-id", 0);
        isBuffed = mod->getSavedValue<bool>("is-buffed", false);
        sawRotationSpeed = mod->getSavedValue<float>("saw-rotation-speed", 0.f);

        // === BACKGROUND ===
        auto bg = CCLayerColor::create(ccc4(255, 255, 255, 160));
        this->addChild(bg, -1);

        // === PANEL ===
        auto panel = CCScale9Sprite::create("square02b_001.png");
        panel->setContentSize({ 360.f, 260.f });
        panel->setPosition(winSize / 2);
        this->addChild(panel);

        // === MENU ===
        auto menu = CCMenu::create();
        menu->setPosition({ 0, 0 });
        panel->addChild(menu);

        // === TITLE ===
        auto title = CCLabelBMFont::create("Level Comparison", "bigFont.fnt");
        title->setPosition({ 180.f, 230.f });
		title->setScale(0.8f);
        panel->addChild(title);

        // === LEVEL ID INPUT ===
        auto idLabel = CCLabelBMFont::create("LEVEL ID", "goldFont.fnt");
        idLabel->setPosition({ 180.f, 195.f });
        panel->addChild(idLabel);

        auto idInput = CCTextInputNode::create(120.f, 40.f, "0", "bigFont.fnt");
		idInput->setMaxLabelLength(10);
		idInput->setAllowedChars("0123456789");
		idInput->setAnchorPoint({0.5f, 0.5f});
		idInput->setTouchEnabled(true);
		idInput->setID("level-id-input"_spr);
        idInput->setString(std::to_string(targetLevelID).c_str());
        idInput->setDelegate(this);
        panel->addChild(idInput);

        // === BUFFED / NERFED TOGGLES ===
        auto off = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        auto on = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");

        nerfedToggle = CCMenuItemToggle::createWithTarget(
            this,
            menu_selector(ComparisonMenu::onNerfed),
            CCMenuItemSprite::create(off, off),
            CCMenuItemSprite::create(on, on),
            nullptr
        );
        nerfedToggle->setPosition({ 120.f, 120.f });
        menu->addChild(nerfedToggle);

        auto nerfedLabel = CCLabelBMFont::create("NERFED", "goldFont.fnt");
        nerfedLabel->setPosition({ 180.f, 120.f });
        panel->addChild(nerfedLabel);

        buffedToggle = CCMenuItemToggle::createWithTarget(
            this,
            menu_selector(ComparisonMenu::onBuffed),
            CCMenuItemSprite::create(off, off),
            CCMenuItemSprite::create(on, on),
            nullptr
        );
        buffedToggle->setPosition({ 240.f, 120.f });
        menu->addChild(buffedToggle);

        auto buffedLabel = CCLabelBMFont::create("BUFFED", "goldFont.fnt");
        buffedLabel->setPosition({ 300.f, 120.f });
        panel->addChild(buffedLabel);

        // set initial toggle state
        buffedToggle->setSelectedIndex(isBuffed ? 1 : 0);
        nerfedToggle->setSelectedIndex(isBuffed ? 0 : 1);

        // === SAW SPEED ===
        auto speedText = CCLabelBMFont::create("SAW ROTATION", "goldFont.fnt");
        speedText->setPosition({ 180.f, 85.f });
        panel->addChild(speedText);

        speedLabel = CCLabelBMFont::create(std::to_string((int)sawRotationSpeed).c_str(), "bigFont.fnt");
        speedLabel->setPosition({ 180.f, 60.f });
        panel->addChild(speedLabel);

        auto slider = Slider::create(this, menu_selector(ComparisonMenu::onSlider));
        slider->setPosition({ 180.f, 35.f });
        slider->setScale(0.8f);
        slider->setValue((sawRotationSpeed + 360.f) / 720.f);
        panel->addChild(slider);

        // === BUTTONS ===
        auto abortBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Abort"),
            this,
            menu_selector(ComparisonMenu::onAbort)
        );
        abortBtn->setPosition({ 100.f, 15.f });
        menu->addChild(abortBtn);

        auto createBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Create"),
            this,
            menu_selector(ComparisonMenu::onCreate)
        );
        createBtn->setPosition({ 260.f, 15.f });
        menu->addChild(createBtn);

        log::info("Menu loaded | ID={} | Buffed={} | Speed={}", targetLevelID, isBuffed, sawRotationSpeed);
        return true;
    }

    // === CALLBACKS ===

    void onNerfed(CCObject*) {
        isBuffed = false;
        buffedToggle->setSelectedIndex(0);
		nerfedToggle->setSelectedIndex(1);
        log::info("Role set to NERFED");
    }

    void onBuffed(CCObject*) {
        isBuffed = true;
        nerfedToggle->setSelectedIndex(0);
		buffedToggle->setSelectedIndex(1);
        log::info("Role set to BUFFED");
    }

    void onSlider(CCObject* sender) {
        auto slider = static_cast<Slider*>(sender);
        sawRotationSpeed = slider->getValue() * 720.f - 360.f;
        speedLabel->setString(std::to_string((int)sawRotationSpeed).c_str());
        log::info("Saw rotation speed = {}", sawRotationSpeed);
    }

    void onAbort(CCObject*) {
        this->removeFromParentAndCleanup(true);
    }

    void onCreate(CCObject*) {
		auto mod = Mod::get();
		mod->setSavedValue("target-level-id", targetLevelID);
		mod->setSavedValue("is-buffed", isBuffed);
		mod->setSavedValue("saw-rotation-speed", sawRotationSpeed);

		log::info("CREATE pressed");
		log::info("ID={} | Buffed={} | Speed={}",
			targetLevelID, isBuffed, sawRotationSpeed);

		if (onCreateCallback) {
			onCreateCallback(
				targetLevelID,
				isBuffed,
				sawRotationSpeed
			);
		}

		this->removeFromParentAndCleanup(true);
	}


	void textChanged(CCTextInputNode* input) override {
		auto text = input->getString();
		if (!text.empty()) {
			targetLevelID = std::stoi(text);
		} else {
			targetLevelID = 0;
		}

		log::info("Level ID changed -> {}", targetLevelID);
	}

	bool ccTouchBegan(CCTouch*, CCEvent*) override {
		return false;
	}
};



class $modify(MakeLevelLayoutLayer, LevelInfoLayer) {
		bool init(GJGameLevel* level, bool challenge) {
		if (!LevelInfoLayer::init(level, challenge))
			return false;

		auto menu = this->getChildByType<CCMenu>(0);
		if (!menu) {
			log::error("No CCMenu found");
			return true;
		}

		auto btn = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png"),
			this,
			menu_selector(MakeLevelLayoutLayer::onButton)
		);

		btn->setID("test-button"_spr);
		menu->addChild(btn);
		menu->updateLayout();

		log::info("Button added to first CCMenu");

		return true;
	}


    void onButton(CCObject*) {
		auto scene = CCDirector::sharedDirector()->getRunningScene();
		scene->addChild(
			ComparisonMenu::create(
				[this](int levelID, bool isBuffed, float sawSpeed) {

					GameLevelManager* glm = GameLevelManager::sharedState();

					GJGameLevel* level1 = this->m_level;
					GJGameLevel* level2 = glm->getSavedLevel(132678721);

					ComparisonConfig config;
					config.isBuffed = isBuffed;
					config.sawSpeed = sawSpeed;

					std::string modifiedLevelString = createComparison(
						level1,
						level2,
						config
					);
					FLAlertLayer::create(
						"Level Comparison",
						std::string("Successfully created layout of ") + level1->m_levelName.c_str(),
						"OK"
					)->show();
					
					GJGameLevel* newLevel = glm->createNewLevel();
					newLevel->m_levelName = "Test " + level1->m_levelName;
					newLevel->m_levelString = modifiedLevelString;
					newLevel->m_levelDesc = ZipUtils::base64URLEncode("Comparison of " + level1->m_levelName + " and " + level2->m_levelName);
					newLevel->m_songID = level1->m_songID;
				}
			),
			999
		);
    }
};

std::string createComparison(GJGameLevel* level1, GJGameLevel* level2, const ComparisonConfig& config) {
	std::vector<GJGameLevel*> levels = { level1, level2 };
	bool first = !config.isBuffed;
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
			bool disableRotation = false; // 98
			bool noParticle = false; // 507
			std::string newObjectStr = "";

			std::vector<std::string> splitStrings = splitString(objectStr, ",", true);
			std::vector<std::vector<std::string>> splitStringsPairs;
			for (int i = 0; i < splitStrings.size() - 1; i += 2) {
				splitStringsPairs.push_back({ splitStrings[i], splitStrings[i + 1] });
			}

			//log::info("Object before: {}", objectStr);

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
						first ? newObjectStr += "20,1," : newObjectStr += "20,2,";
						continue;
					case 21: // color 1
						hasColor1 = true;
						first ? newObjectStr += "21,1," : newObjectStr += "21,2,";
						continue;
					case 22: // color 2
						hasColor2 = true;
						first ? newObjectStr += "22,1," : newObjectStr += "22,2,";
						continue;
					case 43: // delete HSV 1
						continue;
					case 44: // delete HSV 2
						continue;
					case 61: // editor layer 2
						hasLayer2 = true;
						first ? newObjectStr += "61,1," : newObjectStr += "61,2,";
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
						if (config.sawSpeed != 0) newObjectStr += "97," + std::to_string(config.sawSpeed) + ".000000,";
						continue;
					case 98: // turn off disable rotation
						disableRotation = true;
						if (config.sawSpeed == 0) newObjectStr += "98,1,";
						continue;
					case 103: // turn off high detail
						continue;
					case 135: // turn off hide
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


			if (!hasLayer1) { first ? newObjectStr += "20,1," : newObjectStr += "20,2,"; }
			if (!hasColor1) { first ? newObjectStr += "21,1," : newObjectStr += "21,2,"; }
			if (!hasColor2) { first ? newObjectStr += "22,1," : newObjectStr += "22,2,"; }
			if (!hasLayer2) { first ? newObjectStr += "61,1," : newObjectStr += "61,2,"; }
			if (!dontFade) newObjectStr += "64,1,";
			if (!dontEnter) newObjectStr += "67,1,";
			if (!noGlow) newObjectStr += "96,1,";
			if (!customRotationSpeed) { if (config.sawSpeed != 0) newObjectStr += "97," + std::to_string(config.sawSpeed) + ".000000,"; }
			if (!disableRotation) { if (config.sawSpeed == 0) newObjectStr += "98,1,"; }
			if (!noParticle) newObjectStr += "507,1,";

			if (!newObjectStr.empty()) {
				newObjectStr.pop_back(); // Remove the last comma
				objectStr = newObjectStr;
			} else {
				objectStr = "";
			}

			//log::info("Object after: {}", objectStr);
		}

		first ? levelStringSplit1 = levelStringSplit : levelStringSplit2 = levelStringSplit;
		first = !first;
	}

	std::vector<std::string> firstElementSplit = splitString(firstElement, ",", false);
	std::vector<std::vector<std::string>> firstElementPairs;
	for (int i = 0; i < firstElementSplit.size() - 1; i += 2) {
		firstElementPairs.push_back({ firstElementSplit[i], firstElementSplit[i + 1] });
	}
	//log::info("{}", firstElement);
	//log::info("{}", firstElementPairs);

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
	return modifiedLevelString;
}

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
