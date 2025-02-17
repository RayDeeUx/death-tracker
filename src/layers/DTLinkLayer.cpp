#include "../layers/DTLinkLayer.hpp"
#include "../layers/LinkLevelCell.hpp"

DTLinkLayer* DTLinkLayer::create(DTLayer* const& layer) {
    auto ret = new DTLinkLayer();
    if (ret && ret->init(520, 280, layer, "square01_001.png", {0.f, 0.f, 94.f, 94.f})) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool DTLinkLayer::setup(DTLayer* const& layer) {
    //create trackerLayer
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    this->setOpacity(0);

    m_DTLayer = layer;

    m_AllLevels = StatsManager::getAllLevels();

    refreshLists();

    seartchInput = InputNode::create(225, "Filter");
    seartchInput->getInput()->setDelegate(this);
    seartchInput->setPosition({winSize.width / 2, 277});
    seartchInput->setScale(0.6f);
    this->addChild(seartchInput);

    scheduleUpdate();

    return true;
}

void DTLinkLayer::SpacialEditList(GJListLayer* list, CCPoint titlePos, float titleSize){
    CCObject* child;

    CCARRAY_FOREACH(list->m_listView->m_tableView->m_cellArray, child){
        auto childCell = dynamic_cast<GenericListCell*>(child);
        if (childCell)
            childCell->m_backgroundLayer->setOpacity(30);
    }

    std::vector<CCSprite*> spritesToRemove;
    CCLabelBMFont* title;

    CCARRAY_FOREACH(list->getChildren(), child){
        auto childSprite = dynamic_cast<CCSprite*>(child);
        auto childLabel = dynamic_cast<CCLabelBMFont*>(child);
        if (childSprite)
            spritesToRemove.push_back(childSprite);
        if (childLabel)
            title = childLabel;
    }

    for (int i = 0; i < spritesToRemove.size(); i++)
    {
        spritesToRemove[i]->removeMeAndCleanup();
    }

    title->setScale(titleSize);
    if (titlePos.x != -2000)
        title->setPosition({titlePos.x, titlePos.y});
}

void DTLinkLayer::refreshLists(){
    if (m_LevelsList != nullptr) m_LevelsList->removeMeAndCleanup();
    if (m_LinkedLevelsList != nullptr) m_LinkedLevelsList->removeMeAndCleanup();

    auto levelsListItems = CCArray::create();

    auto linkedLevelsListItems = CCArray::create();

    auto myKey = StatsManager::getLevelKey(m_DTLayer->m_Level);

    for (int i = 0; i < m_AllLevels.size(); i++)
    {
        std::string filterTextLower = "";
        for (char ch : m_filterText) { 
            filterTextLower += std::tolower(ch); 
        }

        std::string levelNameLower = "";
        for (char ch : m_AllLevels[i].second.levelName) { 
            levelNameLower += std::tolower(ch); 
        }
        if (m_AllLevels[i].second.levelName == "-1")
            levelNameLower = "unknow name";

        if (m_AllLevels[i].first != myKey && StatsManager::ContainsAtIndex(0, m_filterText, levelNameLower)){
            bool isValidForList = true;
            for (int s = 0; s < m_DTLayer->m_MyLevelStats.LinkedLevels.size(); s++)
            {
                if (m_DTLayer->m_MyLevelStats.LinkedLevels[s] == m_AllLevels[i].first)
                    isValidForList = false;
            }
            
            if (isValidForList){
                levelsListItems->addObject(LinkLevelCell::create(this, m_AllLevels[i].first, m_AllLevels[i].second, false));
            }
            else{
                linkedLevelsListItems->addObject(LinkLevelCell::create(this, m_AllLevels[i].first, m_AllLevels[i].second, true));
            }
        }
    }

    auto levelsListView = ListView::create(levelsListItems, 40, CellsWidth, 220);

    m_LevelsList = GJListLayer::create(levelsListView, "Levels", {0,0,0,75}, CellsWidth, 220, 1);
    m_LevelsList->setPosition({52, 45});
    this->addChild(m_LevelsList);

    SpacialEditList(m_LevelsList, {m_LevelsList->getContentSize().width / 2, 234}, 0.7f);

    auto linkedLevelsListView = ListView::create(linkedLevelsListItems, 40, CellsWidth, 220);

    m_LinkedLevelsList = GJListLayer::create(linkedLevelsListView, "Linked", {0,0,0,75}, CellsWidth, 220, 1);
    m_LinkedLevelsList->setPosition({286, 45});
    this->addChild(m_LinkedLevelsList);

    SpacialEditList(m_LinkedLevelsList, {m_LinkedLevelsList->getContentSize().width / 2, 234}, 0.7f);
}

void DTLinkLayer::ChangeLevelLinked(std::string levelKey, LevelStats stats, bool erase){
    if (erase){
        for (int i = 0; i < m_DTLayer->m_MyLevelStats.LinkedLevels.size(); i++)
        {
            if (m_DTLayer->m_MyLevelStats.LinkedLevels[i] == levelKey)
            {
                m_DTLayer->m_MyLevelStats.LinkedLevels.erase(std::next(m_DTLayer->m_MyLevelStats.LinkedLevels.begin(), i));
                break;
            }
        }

        for (int i = 0; i < stats.LinkedLevels.size(); i++)
        {
            if (stats.LinkedLevels[i] == StatsManager::getLevelKey(m_DTLayer->m_Level))
            {
                stats.LinkedLevels.erase(std::next(stats.LinkedLevels.begin(), i));
                break;
            }
        }
    }
    else{
        m_DTLayer->m_MyLevelStats.LinkedLevels.push_back(levelKey);

        stats.LinkedLevels.push_back(StatsManager::getLevelKey(m_DTLayer->m_Level));
    }

    refreshLists();

    if (!erase){
        m_LevelsList->retain();
        m_LevelsList->removeFromParent();
        this->addChild(m_LevelsList);
        m_LevelsList->release();
    }
    else{
        m_LinkedLevelsList->retain();
        m_LinkedLevelsList->removeFromParent();
        this->addChild(m_LinkedLevelsList);
        m_LinkedLevelsList->release();
    }

    StatsManager::saveData(m_DTLayer->m_MyLevelStats, m_DTLayer->m_Level);

    StatsManager::saveData(stats, levelKey);
}

void DTLinkLayer::update(float delta){

    CCRect LevelsListRect = {m_LevelsList->getPositionX(), m_LevelsList->getPositionY(), m_LevelsList->getScaledContentSize().width, m_LevelsList->getScaledContentSize().height};

    CCRect LinkedLevelsListRect = {m_LinkedLevelsList->getPositionX(), m_LinkedLevelsList->getPositionY(), m_LinkedLevelsList->getScaledContentSize().width, m_LinkedLevelsList->getScaledContentSize().height};

    if (LevelsListRect.containsPoint(convertToNodeSpace(getMousePos())) && !scrollSwitchLock){
        scrollSwitchLock = true;

        m_LevelsList->retain();
        m_LevelsList->removeFromParent();
        this->addChild(m_LevelsList);
        m_LevelsList->release();
    }

    if (LinkedLevelsListRect.containsPoint(convertToNodeSpace(getMousePos())) && scrollSwitchLock){
        scrollSwitchLock = false;

        m_LinkedLevelsList->retain();
        m_LinkedLevelsList->removeFromParent();
        this->addChild(m_LinkedLevelsList);
        m_LinkedLevelsList->release();
    }
}

void DTLinkLayer::textChanged(CCTextInputNode* input){
    if (seartchInput->getInput() == input){
        std::string filterText = "";
        if (input->getString() != "")
            filterText = input->getString();
        
        m_filterText = filterText;
        refreshLists();
    }
}

void DTLinkLayer::onClose(CCObject*) {
    m_DTLayer->UpdateSharedStats();
    m_DTLayer->refreshStrings();
    m_DTLayer->RefreshText();
    m_DTLayer->refreshRunAllowedListView();
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
}
