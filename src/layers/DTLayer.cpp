#include "../layers/DTLayer.hpp"
#include "../managers/DTPopupManager.hpp"
#include "../utils/Settings.hpp"
#include "../layers/LabelLayoutWindow.hpp"
#include "../layers/RunAllowedCell.hpp"
#include "../layers/DTGraphLayer.hpp"
#include "../layers/DTLinkLayer.hpp"

DTLayer* DTLayer::create(GJGameLevel* const& Level) {
    auto ret = new DTLayer();
    if (ret && ret->init(520, 280, Level, "square01_001.png", {0.f, 0.f, 94.f, 94.f})) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool DTLayer::setup(GJGameLevel* const& level) {
    //create trackerLayer
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    m_Level = level;

    // ================================== //
    // loading data
    m_MyLevelStats = StatsManager::getLevelStats(m_Level);

    UpdateSharedStats();
    // ================================== //

    /*
     * main page
    */

    this->setZOrder(100);
    m_buttonMenu->setZOrder(1);

    //texts bg
    m_TextBG = CCScale9Sprite::create("GJ_square05.png", {0,0, 80, 80});
    m_TextBG->setContentSize({m_size.width / 1.6f, m_size.height / 1.15f});
    m_TextBG->setPosition({winSize.width / 2, winSize.height / 2});
    m_TextBG->setOpacity(100);
    m_TextBG->setZOrder(10);
    m_mainLayer->addChild(m_TextBG);

    //create a scroll layer for the text
    m_ScrollLayer = ScrollLayer::create({m_size.width / 1.6f, m_size.height / 1.15f});
    m_ScrollLayer->setPosition(CCSize{winSize.width / 2, winSize.height / 2} - m_TextBG->getContentSize() / 2);
    m_ScrollLayer->setZOrder(11);
    m_mainLayer->addChild(m_ScrollLayer);

    m_ScrollBar = Scrollbar::create(m_ScrollLayer);
    m_ScrollBar->setScale(0.95f);
    m_ScrollBar->setZOrder(11);
    m_ScrollBar->setPosition({439, 160});
    m_mainLayer->addChild(m_ScrollBar);

    auto editLayoutBtnS = ButtonSprite::create("Edit Layout");
    editLayoutBtnS->setScale(0.475f);
    m_EditLayoutBtn = CCMenuItemSpriteExtra::create(
        editLayoutBtnS,
        nullptr,
        this,
        menu_selector(DTLayer::onEditLayout)
    );
    m_EditLayoutBtn->setPosition({207, 114});
    this->m_buttonMenu->addChild(m_EditLayoutBtn);

    /*
     * edit layout
    */

    m_EditLayoutMenu = CCMenu::create();
    m_EditLayoutMenu->setVisible(false);
    m_EditLayoutMenu->setZOrder(10);
    m_mainLayer->addChild(m_EditLayoutMenu);

    m_BlackSquare = CCSprite::create("square02_001.png");
    m_BlackSquare->setZOrder(9);
    m_BlackSquare->setVisible(false);
    m_BlackSquare->setPosition(winSize / 2);
    m_BlackSquare->setScale(100);
    m_BlackSquare->setOpacity(150);
    m_mainLayer->addChild(m_BlackSquare);

    auto editLayoutCancelBtnS = ButtonSprite::create("Cancel");
    editLayoutCancelBtnS->setColor({ 255, 0, 0 });
    editLayoutCancelBtnS->setScale(0.375f);
    auto editLayoutCancelBtn = CCMenuItemSpriteExtra::create(
        editLayoutCancelBtnS,
        nullptr,
        this,
        menu_selector(DTLayer::onEditLayoutCancel)
    );
    editLayoutCancelBtn->setPosition({226, 114});
    m_EditLayoutMenu->addChild(editLayoutCancelBtn);

    auto editLayoutApplyBtnS = ButtonSprite::create("Apply");
    editLayoutApplyBtnS->setScale(0.375f);
    auto editLayoutApplyBtn = CCMenuItemSpriteExtra::create(
        editLayoutApplyBtnS,
        nullptr,
        this,
        menu_selector(DTLayer::onEditLayoutApply)
    );
    editLayoutApplyBtn->setPosition({186, 114});
    m_EditLayoutMenu->addChild(editLayoutApplyBtn);

    auto addWindowButtonS = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    auto addWindowButton = CCMenuItemSpriteExtra::create(
        addWindowButtonS,
        nullptr,
        this,
        menu_selector(DTLayer::addBox)
    );
    addWindowButton->setPosition({205, 96});
    m_EditLayoutMenu->addChild(addWindowButton);

    //session selection

    auto SessionSelectCont = CCNode::create();
    SessionSelectCont->setID("Session-Select-Container");
    SessionSelectCont->setPosition({77, 253});
    m_mainLayer->addChild(SessionSelectCont);

    m_SessionSelectMenu = CCMenu::create();
    m_SessionSelectMenu->setPosition({0, 0});
    SessionSelectCont->addChild(m_SessionSelectMenu);

    m_SessionsAmount = m_SharedLevelStats.sessions.size();
    m_SessionSelected = 1;

    m_SessionSelectionInput = InputNode::create(120, "Session");
    if (m_SessionsAmount == 0)
        m_SessionSelectionInput->setString("No sessions.");
    else
        m_SessionSelectionInput->setString(fmt::format("{}/{}", m_SessionSelected, m_SessionsAmount));
    m_SessionSelectionInput->getInput()->setDelegate(this);
    m_SessionSelectionInput->getInput()->setAllowedChars("1234567890");
    m_SessionSelectionInput->setScale(0.45f);
    SessionSelectCont->addChild(m_SessionSelectionInput);

    auto SessionSelectionRightS = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    SessionSelectionRightS->setScaleX(0.35f);
    SessionSelectionRightS->setScaleY(0.2f);
    auto SessionSelectionRight = CCMenuItemSpriteExtra::create(
        SessionSelectionRightS,
        nullptr,
        this,
        menu_selector(DTLayer::SwitchSessionRight)
    );
    SessionSelectionRight->setPosition({34, 0});
    m_SessionSelectMenu->addChild(SessionSelectionRight);

    auto SessionSelectionLeftS = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    SessionSelectionLeftS->setScaleX(0.35f);
    SessionSelectionLeftS->setScaleY(0.2f);
    auto SessionSelectionLeft = CCMenuItemSpriteExtra::create(
        SessionSelectionLeftS,
        nullptr,
        this,
        menu_selector(DTLayer::SwitchSessionLeft)
    );
    SessionSelectionLeft->setPosition({-34, 0});
    SessionSelectionLeft->setRotation(180);
    m_SessionSelectMenu->addChild(SessionSelectionLeft);

    auto SessionSelectionLabel = CCLabelBMFont::create("Session", "bigFont.fnt");
    SessionSelectionLabel->setPosition({0, 16});
    SessionSelectionLabel->setScale(0.45f);
    SessionSelectCont->addChild(SessionSelectionLabel);

    refreshRunAllowedListView();

    m_AddRunAllowedInput = InputNode::create(90, "ST%");
    m_AddRunAllowedInput->getInput()->setMaxLabelLength(3);
    m_AddRunAllowedInput->getInput()->setAllowedChars("1234567890");
    m_AddRunAllowedInput->getInput()->setDelegate(this);
    m_AddRunAllowedInput->setPosition({67, 221});
    m_AddRunAllowedInput->setScale(0.5f);
    m_mainLayer->addChild(m_AddRunAllowedInput);

    m_RunStuffMenu = CCMenu::create();
    m_mainLayer->addChild(m_RunStuffMenu);

    auto AddRunAllowedButtonS = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    AddRunAllowedButtonS->setScale(0.75f);
    auto AddRunAllowedButton = CCMenuItemSpriteExtra::create(
        AddRunAllowedButtonS,
        nullptr,
        this,
        menu_selector(DTLayer::addRunAllowed)
    );
    AddRunAllowedButton->setPosition({-180, 61});
    m_RunStuffMenu->addChild(AddRunAllowedButton);

    auto checkOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    auto checkOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    allRunsToggle = CCMenuItemToggler::create(
        checkOff,
        checkOn,
        this,
        menu_selector(DTLayer::OnToggleAllRuns)
    );

    if (m_MyLevelStats.RunsToSave.size())
        if (m_MyLevelStats.RunsToSave[0] == -1){
            allRunsToggle->toggle(true);
        }

    allRunsToggle->setPosition({-238, -26});
    allRunsToggle->setScale(0.35f);
    m_RunStuffMenu->addChild(allRunsToggle);

    auto allRunsToggleLabel = CCLabelBMFont::create("Track any run", "bigFont.fnt");
    allRunsToggleLabel->setPosition({-198, -26});
    allRunsToggleLabel->setScale(0.25f);
    m_RunStuffMenu->addChild(allRunsToggleLabel);

    auto DeleteUnusedButtonS = ButtonSprite::create("Delete Unused", "bigFont.fnt", "GJ_button_06.png");
    DeleteUnusedButtonS->setScale(0.3f);
    auto DeleteUnusedButton = CCMenuItemSpriteExtra::create(
        DeleteUnusedButtonS,
        nullptr,
        this,
        menu_selector(DTLayer::deleteUnused)
    );
    DeleteUnusedButton->setPosition({-207, -40});
    m_RunStuffMenu->addChild(DeleteUnusedButton);

    //graph

    auto GraphButtonS = ButtonSprite::create("Graph", "bigFont.fnt", "GJ_button_01.png");
    GraphButtonS->setScale(0.3f);
    auto GraphButton = CCMenuItemSpriteExtra::create(
        GraphButtonS,
        nullptr,
        this,
        menu_selector(DTLayer::openGraphMenu)
    );
    GraphButton->setPosition({-207, -60});
    m_buttonMenu->addChild(GraphButton);

    //linking

    auto LinkLevelsButtonS = CCSprite::createWithSpriteFrameName("gj_linkBtn_001.png");
    auto LinkLevelsButton = CCMenuItemSpriteExtra::create(
        LinkLevelsButtonS,
        nullptr,
        this,
        menu_selector(DTLayer::OnLinkButtonClicked)
    );
    LinkLevelsButton->setPosition({-208, -108});
    m_buttonMenu->addChild(LinkLevelsButton);

    createLayoutBlocks();
    refreshStrings();
    RefreshText(true);

    scheduleUpdate();

    return true;
}

void DTLayer::update(float delta){
    m_LayoutStuffCont->setVisible(m_EditLayoutMenu->isVisible());
    m_TextCont->setVisible(!m_EditLayoutMenu->isVisible());
}

void DTLayer::onEditLayout(CCObject* sender){
    EditLayoutEnabled(true);
}

void DTLayer::onEditLayoutCancel(CCObject*){
    EditLayoutEnabled(false);
    m_LayoutStuffCont->removeMeAndCleanup();
    createLayoutBlocks();
    RefreshText();
}

void DTLayer::textChanged(CCTextInputNode* input){
    if (input == m_SessionSelectionInput->getInput() && m_SessionsAmount > 0){
        int selected = 1;
        if (input->getString() != "")
            selected = std::stoi(input->getString());
        
        if (selected > m_SessionsAmount){
            selected = m_SessionsAmount;
            input->setString(fmt::format("{}", m_SessionsAmount));
        }

        if (selected < 1){
            selected = 1;
            input->setString("1");
        }

        m_SessionSelected = selected;
        updateSessionString(m_SessionSelected);
        RefreshText();
    }

    if (input == m_AddRunAllowedInput->getInput()){

        int res = 0;
        if (input->getString() != "")
            res = std::stoi(input->getString());

        if (res > 100){
            res = 100;
            input->setString("100");
        }

    }
}

void DTLayer::textInputOpened(CCTextInputNode* input){
    if (input == m_SessionSelectionInput->getInput() && m_SessionsAmount > 0){
        input->setString(fmt::format("{}", m_SessionSelected));
        m_SessionSelectionInputSelected = true;
    }
}

void DTLayer::textInputClosed(CCTextInputNode* input){
    if (input == m_SessionSelectionInput->getInput() && m_SessionsAmount > 0){
        input->setString(fmt::format("{}/{}", m_SessionSelected, m_SessionsAmount));
        m_SessionSelectionInputSelected = false;
    }
}

void DTLayer::updateSessionString(int session){
    if (session - 1 < 0 || session - 1 >= m_SharedLevelStats.sessions.size()) return;

    Session currentSession = m_SharedLevelStats.sessions[session - 1];

    selectedSessionInfo = CreateDeathsString(currentSession.deaths, currentSession.newBests, "<sbc>");
    m_SelectedSessionRunInfo = CreateRunsString(currentSession.runs);

    std::string mergedString = "";
    for (int i = 0; i < selectedSessionInfo.size(); i++)
    {
        if (std::get<0>(selectedSessionInfo[0]) != "-1" && std::get<0>(selectedSessionInfo[0]) != "No Saved Progress"){
            mergedString += fmt::format("{}% x{}", std::get<0>(selectedSessionInfo[i]), std::get<1>(selectedSessionInfo[i]));
            if (i != selectedSessionInfo.size() - 1) mergedString += '\n';
        }
    }
    selectedSessionString = mergedString;

    mergedString = "";
    for (int i = 0; i < m_SelectedSessionRunInfo.size(); i++)
    {
        if (std::get<0>(m_SelectedSessionRunInfo[0]) != "-1" && std::get<0>(m_SelectedSessionRunInfo[0]) != "No Saved Progress"){
            Run SplittedKey = StatsManager::splitRunKey(std::get<0>(m_SelectedSessionRunInfo[i]));

            mergedString += fmt::format("{}% - {}% x{}", SplittedKey.start, SplittedKey.end, std::get<1>(m_SelectedSessionRunInfo[i]));
            if (i != m_SelectedSessionRunInfo.size() - 1) mergedString += '\n';
        }
    }
    selectedSessionRunString = mergedString;
}

void DTLayer::onEditLayoutApply(CCObject*){
    EditLayoutEnabled(false);

    std::vector<LabelLayout> layout;
    for (int i = 0; i < m_LayoutLines.size(); i++)
    {
        auto IWindow = static_cast<LabelLayoutWindow*>(m_LayoutLines[i]);
        layout.push_back(IWindow->m_MyLayout);
    }

    Save::setLayout(layout);

    RefreshText(true);
}

bool DTLayer::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent){
    m_IsClicking = true;
    ClickPos = pTouch;
    return true;
}

void DTLayer::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent){
    //m_IsClicking = false;
    ClickPos = pTouch;
}

void DTLayer::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent){
    m_IsClicking = false;
    ClickPos = pTouch;
}

void DTLayer::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent){
    m_IsClicking = false;
    ClickPos = pTouch;
}

void DTLayer::EditLayoutEnabled(bool b){
    this->m_buttonMenu->setEnabled(!b);
    m_SessionSelectMenu->setEnabled(!b);
    m_RunStuffMenu->setEnabled(!b);
    m_EditLayoutMenu->setVisible(b);
    m_EditLayoutBtn->setVisible(!b);
    m_BlackSquare->setVisible(b);
    m_LayoutStuffCont->setVisible(b);
    if (b)
    {
        changeScrollSizeByBoxes(true);
        m_TextBG->setOpacity(200);
    }
    else{
        m_TextBG->setOpacity(100);
    }
}

void DTLayer::changeScrollSizeByBoxes(bool moveToTop){
    float hight = 0;
    float lineH = 0;
    LabelLayoutWindow* PrevLine = nullptr;

    std::vector<LabelLayoutWindow*> sortedLayoutLines;
    for (int i = 0; i < m_LayoutLines.size(); i++)
        sortedLayoutLines.push_back(static_cast<LabelLayoutWindow*>(m_LayoutLines[i]));
    
    std::ranges::sort(sortedLayoutLines, [](const LabelLayoutWindow* a, const LabelLayoutWindow* b) {
        return a->m_MyLayout.line < b->m_MyLayout.line;
    });

    for (int i = 0; i < sortedLayoutLines.size(); i++)
    {
        //hight += m_LayoutLines[i]->getContentSize().height;

        if (PrevLine){
            float calH = abs(sortedLayoutLines[i]->getPositionY() - PrevLine->getPositionY());
            hight += calH;
        }
        else{
            lineH = m_LayoutLines[i]->getContentSize().height;
        }

        PrevLine = sortedLayoutLines[i];
    }
    hight += lineH * 2;

    if (hight < m_ScrollLayer->getContentSize().height){
        m_ScrollLayer->m_contentLayer->setContentSize({m_ScrollLayer->m_contentLayer->getContentSize().width, m_ScrollLayer->getContentSize().height});
    }
    else{
        m_ScrollLayer->m_contentLayer->setContentSize({m_ScrollLayer->m_contentLayer->getContentSize().width, hight});
    }
    
    m_LayoutStuffCont->setPositionX(m_ScrollLayer->getContentSize().width / 2);
    m_LayoutStuffCont->setPositionY(m_ScrollLayer->m_contentLayer->getContentSize().height);

    if (moveToTop)
        m_ScrollLayer->moveToTop();
}

void DTLayer::createLayoutBlocks(){
    auto m_CurretLayout = Save::getLayout();

    m_LayoutStuffCont = CCNode::create();
    m_LayoutStuffCont->setPositionX(m_ScrollLayer->getContentSize().width / 2);
    m_LayoutStuffCont->setPositionY(m_ScrollLayer->m_contentLayer->getContentSize().height);
    m_LayoutStuffCont->setVisible(false);

    m_ScrollLayer->m_contentLayer->addChild(m_LayoutStuffCont);

    m_LayoutLines.clear();

    for (int i = 0; i < m_CurretLayout.size(); i++)
    {
        auto currentWindow = LabelLayoutWindow::create(m_CurretLayout[i], this);
        m_LayoutStuffCont->addChild(currentWindow);
        handleTouchPriority(this);
        m_LayoutLines.push_back(currentWindow);
    }

    for (int i = 0; i < m_LayoutLines.size(); i++)
    {
        static_cast<LabelLayoutWindow*>(m_LayoutLines[i])->setPositionBasedOnLayout(static_cast<LabelLayoutWindow*>(m_LayoutLines[i])->m_MyLayout);
    }
}

void DTLayer::RefreshText(bool moveToTop){

    if (m_TextCont) m_TextCont->removeMeAndCleanup();

    m_TextCont = CCNode::create();
    m_ScrollLayer->m_contentLayer->addChild(m_TextCont);

    float overallHight = 0;

    auto layout = Save::getLayout();

    std::vector<std::tuple<SimpleTextArea*, int, int>> lables;

    std::vector<std::pair<int, float>> positioning;
    std::map<int, bool> twoLabelLines;

    for (const auto& labelSettings : layout)
    {
        auto modifiedString = modifyString(labelSettings.text);

        auto label = SimpleTextArea::create(modifiedString.c_str(), StatsManager::getFont(labelSettings.font).c_str());
        label->setAlignment(labelSettings.alignment);
        label->setAnchorPoint({0.5f, 1});
        label->setWrappingMode(WrappingMode::WORD_WRAP);
        label->setColor(labelSettings.color);
        label->setScale(labelSettings.fontSize);
        lables.push_back(std::tuple<SimpleTextArea*, int, int>{label, labelSettings.line, labelSettings.position});
        m_TextCont->addChild(label);

        float contentSize = 0;

        for (const auto& line : label->getLines())
        {
            contentSize += line->getScaledContentSize().height + label->getLinePadding();
        }

        bool notAdded = true;

        for (int i = 0; i < positioning.size(); i++)
        {
            if (positioning[i].first == labelSettings.line){
                notAdded = false;
                twoLabelLines.insert(std::pair<int, bool>{labelSettings.line, true});
                if (positioning[i].second < contentSize){
                    positioning[i].second = contentSize;
                }
            }
        }

        if (notAdded)
            positioning.push_back(std::pair<int, float>{labelSettings.line, contentSize});
    }

    std::ranges::sort(positioning, [](const std::pair<int, float> a, const std::pair<int, float> b) {
        auto posA = std::get<0>(a);
        auto posB = std::get<0>(b);
        return posA < posB;
    });

    for (int i = 0; i < lables.size(); i++)
    {
        if (twoLabelLines[std::get<1>(lables[i])]){
            SimpleTextArea* currentArea = std::get<0>(lables[i]);

            currentArea->setWidth({currentArea->getWidth() / 2});
            if (std::get<2>(lables[i]) == 1){
                currentArea->setPositionX(currentArea->getPositionX() + currentArea->getWidth() / 2);
            }
            else{
                currentArea->setPositionX(currentArea->getPositionX() - currentArea->getWidth() / 2);
            }
            currentArea->setFont(currentArea->getFont());
        }
        for (int p = 0; p < positioning.size(); p++)
        {
            if (std::get<1>(lables[i]) > positioning[p].first){
                std::get<0>(lables[i])->setPositionY(std::get<0>(lables[i])->getPositionY() - positioning[p].second);
            }
            else{
                break;
            }
        }
        
    }

    for (const auto& labelWExtra : lables)
    {
        auto label = std::get<0>(labelWExtra);
        for (const auto& line : label->getLines())
        {
            std::string s = line->getString();
            if (s != "<" && s.length() > 1){
                //new best color
                if (isKeyInIndex(s, 1, "nbc>")){
                    s.erase(0, 5);
                    line->setString(s.c_str());
                    line->setColor(Save::getNewBestColor());
                }
                //sessions best color
                if (isKeyInIndex(s, 1, "sbc>")){
                    s.erase(0, 5);
                    line->setString(s.c_str());
                    line->setColor(Save::getSessionBestColor()); 
                }
            }
        }
    }
    

    for (int i = 0; i < positioning.size(); i++)
    {
        overallHight += positioning[i].second;
    }

    if (overallHight < m_ScrollLayer->getContentSize().height){
        m_ScrollLayer->m_contentLayer->setContentSize({m_ScrollLayer->m_contentLayer->getContentSize().width, m_ScrollLayer->getContentSize().height});
    }
    else{
        m_ScrollLayer->m_contentLayer->setContentSize({m_ScrollLayer->m_contentLayer->getContentSize().width, overallHight});
    }

    m_TextCont->setPositionX(m_ScrollLayer->getContentSize().width / 2);
    m_TextCont->setPositionY(m_ScrollLayer->m_contentLayer->getContentSize().height);

    if (moveToTop)
        m_ScrollLayer->moveToTop();
    /*
    create labels by settings

    know how high every line
        get the highest if the who in the same line
    
    place the labels at the bottom of the previewse line postions added up
    
    place the pos 1 on the right and pos 0 on the left
    */
}

std::string DTLayer::modifyString(std::string ToModify){
    /*
    keys to check for

    {f0} - runs from 0

    {runs} - runs

    {lvln} - levels name
    
    {att} - level attempts (with linked levels attempts)

    {s0} - selected session runs from 0
    
    {sruns} - selected session runs

    {nl} - new line
    */
    
    for (int i = 0; i < ToModify.length(); i++)
    {
        if (i + 1 < ToModify.length() - 1){
            if (ToModify[i] == '{'){
                if (isKeyInIndex(ToModify, i + 1, "f0}")){
                    ToModify.erase(i, 4);
                    ToModify.insert(i, deathsString);
                }
                if (isKeyInIndex(ToModify, i + 1, "runs}")){
                    ToModify.erase(i, 6);
                    ToModify.insert(i, RunString);
                }
                if (isKeyInIndex(ToModify, i + 1, "lvln}")){
                    ToModify.erase(i, 6);
                    ToModify.insert(i, m_Level->m_levelName);
                }
                if (isKeyInIndex(ToModify, i + 1, "att}")){
                    ToModify.erase(i, 5);
                    ToModify.insert(i, std::to_string(m_SharedLevelStats.attempts));
                }
                if (isKeyInIndex(ToModify, i + 1, "s0}")){
                    ToModify.erase(i, 4);
                    ToModify.insert(i, selectedSessionString);
                }
                if (isKeyInIndex(ToModify, i + 1, "sruns}")){
                    ToModify.erase(i, 7);
                    ToModify.insert(i, selectedSessionRunString);
                }
                if (isKeyInIndex(ToModify, i + 1, "nl}")){
                    ToModify.erase(i, 4);
                    ToModify.insert(i, "\n");
                }
            }
        }
    }
    
    return ToModify;
}

bool DTLayer::isKeyInIndex(std::string s, int Index, std::string key){
    if (Index + key.length() > s.length()) false;

    bool toReturn = true;

    for (int i = 0; i < key.length(); i++)
    {
        if (s[Index + i] != key[i])
            toReturn = false;
    }
    
    return toReturn;
}

void DTLayer::refreshStrings(){
    m_DeathsInfo = CreateDeathsString(m_SharedLevelStats.deaths, m_SharedLevelStats.newBests, "<nbc>");
    m_RunInfo = CreateRunsString(m_SharedLevelStats.runs);

    std::string mergedString = "";
    for (int i = 0; i < m_DeathsInfo.size(); i++)
    {
        if (std::get<0>(m_DeathsInfo[0]) != "-1" && std::get<0>(m_DeathsInfo[0]) != "No Saved Progress"){
            mergedString += fmt::format("{}% x{}", std::get<0>(m_DeathsInfo[i]), std::get<1>(m_DeathsInfo[i]));
            if (i != m_DeathsInfo.size() - 1) mergedString += '\n';
        }
    }
    deathsString = mergedString;

    mergedString = "";
    for (int i = 0; i < m_RunInfo.size(); i++)
    {
        if (std::get<0>(m_RunInfo[0]) != "-1" && std::get<0>(m_RunInfo[0]) != "No Saved Progress"){
            Run SplittedKey = StatsManager::splitRunKey(std::get<0>(m_RunInfo[i]));

            mergedString += fmt::format("{}% - {}% x{}", SplittedKey.start, SplittedKey.end, std::get<1>(m_RunInfo[i]));
            if (i != m_RunInfo.size() - 1) mergedString += '\n';
        }
    }
    RunString = mergedString;

    updateSessionString(m_SessionSelected);
}

std::vector<std::tuple<std::string, int, float>> DTLayer::CreateDeathsString(Deaths deaths, NewBests newBests, std::string NewBestsColorString){
    if (!m_Level) return std::vector<std::tuple<std::string, int, float>>{std::tuple<std::string, int, float>("-1", 0, 0)};
    if (deaths.size() == 0) return std::vector<std::tuple<std::string, int, float>>{std::tuple<std::string, int, float>("No Saved Progress", 0, 0)};

    int totalDeaths = 0;
    std::vector<std::tuple<std::string, int>> sortedDeaths{};

    // sort the deaths
    for (const auto [percentKey, count] : deaths) {
        sortedDeaths.push_back(std::make_tuple(percentKey, count));
        totalDeaths += count;
    }

    std::ranges::sort(sortedDeaths, [](const std::tuple<std::string, int> a, const std::tuple<std::string, int> b) {
        auto percentA = std::stof(std::get<0>(a));
        auto percentB = std::stof(std::get<0>(b));
        return percentA < percentB; // true --> A before B
    });

    // create output
	int offset = m_Level->m_normalPercent.value() == 100
		? 1
		: 0;

    std::vector<std::tuple<std::string, int, float>> output{};

    int bestRun = 0;

    for (const auto& best : newBests)
    {
        if (best > bestRun) bestRun = best;
    }

    for (const auto [percentKey, count] : sortedDeaths) {
        // calculate pass rate
        totalDeaths -= count;

        float passCount = totalDeaths;
        float passRate = (passCount + offset) / (passCount + count + offset) * 100;

        // format output
        auto labelStr = percentKey;

        if (newBests.contains(std::stoi(percentKey)))
            labelStr.insert(0, NewBestsColorString);
            
        if (std::stoi(percentKey) == bestRun)
            if (bestRun != 100)
                passRate = 0;
            else
                passRate = 100;

        output.push_back(std::tuple<std::string, int, float>(labelStr, count, passRate));
    }

    if (output.size() == 0) output.push_back(std::tuple<std::string, int, float>("No Saved Progress", 0, 0));
    return output;
}

std::vector<std::tuple<std::string, int, float>> DTLayer::CreateRunsString(Runs runs){
    if (!m_Level) return std::vector<std::tuple<std::string, int, float>>{std::tuple<std::string, int, float>("-1", 0, 0)};
    if (!runs.size()) return std::vector<std::tuple<std::string, int, float>>{std::tuple<std::string, int, float>("No Saved Progress", 0, 0)};

    std::map<int, int> totalDeaths;
    std::vector<std::tuple<std::string, int>> sortedRuns{};

    for (const auto [runKey, count] : runs){
        if (m_MyLevelStats.RunsToSave.size())
            if (m_MyLevelStats.RunsToSave[0] == -1){
                sortedRuns.push_back(std::make_tuple(runKey, count));
                        
                totalDeaths[StatsManager::splitRunKey(runKey).start] += count;
            }
            else{
                for (int i = 0; i < m_MyLevelStats.RunsToSave.size(); i++)
                {
                    if (m_MyLevelStats.RunsToSave[i] == StatsManager::splitRunKey(runKey).start){
                        sortedRuns.push_back(std::make_tuple(runKey, count));
                        
                        totalDeaths[StatsManager::splitRunKey(runKey).start] += count;
                        
                        break;
                    }
                }
            }
        
    }

    // sort the runs
    std::ranges::sort(sortedRuns, [](const std::tuple<std::string, int> a, const std::tuple<std::string, int> b) {
        auto runA = StatsManager::splitRunKey(std::get<0>(a));
        auto runB = StatsManager::splitRunKey(std::get<0>(b));

        // start is equal, compare end
        if (runA.start == runB.start) return runA.end < runB.end;
        return runA.start < runB.start;
    });

    // create output
    std::vector<std::tuple<std::string, int, float>> output{};


    for (const auto [runKey, count] : sortedRuns) {

        totalDeaths[StatsManager::splitRunKey(runKey).start] -= count;

        float passCount = totalDeaths[StatsManager::splitRunKey(runKey).start];
        float passRate = (passCount) / (passCount + count) * 100;

        output.push_back(std::tuple<std::string, int, float>(runKey, count, passRate));
    }

    return output;
}

void DTLayer::SwitchSessionRight(CCObject*){
    if (m_SessionSelected >= m_SessionsAmount) return;

    m_SessionSelected += 1;
    if (m_SessionSelectionInputSelected)
        m_SessionSelectionInput->setString(fmt::format("{}", m_SessionSelected));
    else
        m_SessionSelectionInput->setString(fmt::format("{}/{}", m_SessionSelected, m_SessionsAmount));
    updateSessionString(m_SessionSelected);
    RefreshText();
}   

void DTLayer::SwitchSessionLeft(CCObject*){
    if (m_SessionSelected - 1 < 1) return;

    m_SessionSelected -= 1;
    if (m_SessionSelectionInputSelected)
        m_SessionSelectionInput->setString(fmt::format("{}", m_SessionSelected));
    else
        m_SessionSelectionInput->setString(fmt::format("{}/{}", m_SessionSelected, m_SessionsAmount));
    updateSessionString(m_SessionSelected);
    RefreshText();
}

void DTLayer::addBox(CCObject*){
    int lastLine = 0;

    for (int i = 0; i < m_LayoutLines.size(); i++)
    {
        if (static_cast<LabelLayoutWindow*>(m_LayoutLines[i])->m_MyLayout.line > lastLine){
            lastLine = static_cast<LabelLayoutWindow*>(m_LayoutLines[i])->m_MyLayout.line;
        }
    }
    
    LabelLayout currentLayout
    {
        .labelName = "Label",
        .text = "",
        .line = lastLine + 1,
        .position = 0,
        .color = {255, 255, 255, 255},
        .alignment = CCTextAlignment::kCCTextAlignmentCenter,
        .font = 0,
        .fontSize = 0.75f
    };

    auto currentWindow = LabelLayoutWindow::create(currentLayout, this);
    m_LayoutStuffCont->addChild(currentWindow);
    m_LayoutLines.push_back(currentWindow);

    currentWindow->setPositionBasedOnLayout(currentLayout);

    changeScrollSizeByBoxes();
}

void DTLayer::addRunAllowed(CCObject*){

    int startPrecent = -1;
    if (m_AddRunAllowedInput->getString() != "")
        startPrecent = std::stoi(m_AddRunAllowedInput->getString());
    if (startPrecent == -1) return;

    bool doesExist = false;
    for (int i = 0; i < m_MyLevelStats.RunsToSave.size(); i++)
    {
        if (m_MyLevelStats.RunsToSave[i] == startPrecent)
            doesExist = true;
    }
    
    if (!doesExist){
        m_MyLevelStats.RunsToSave.push_back(startPrecent);

        std::ranges::sort(m_MyLevelStats.RunsToSave, [](const int a, const int b) {
            return a < b; // true --> A before B
        });

        refreshRunAllowedListView();
        updateRunsAllowed();
    }
}

void DTLayer::updateRunsAllowed(){
    StatsManager::saveData(m_MyLevelStats, m_Level);
    refreshStrings();
    RefreshText();
}

void DTLayer::refreshRunAllowedListView(){
    if (m_RunsList) m_RunsList->removeMeAndCleanup();
    CCArray* runsAllowed = CCArray::create();

    for (int i = 0; i < m_MyLevelStats.RunsToSave.size(); i++)
    {
        if (m_MyLevelStats.RunsToSave[i] != -1)
            runsAllowed->addObject(RunAllowedCell::create(m_MyLevelStats.RunsToSave[i], this));
    }
    
    auto runsAllowedView = ListView::create(runsAllowed, 20, 75, 70);

    m_RunsList = GJListLayer::create(runsAllowedView, "Runs", {0,0,0,75}, 75, 70, 1);
    m_RunsList->setPosition({41, 142});
    m_mainLayer->addChild(m_RunsList);

    m_ScrollLayer->retain();
    m_ScrollLayer->removeFromParent();
    m_mainLayer->addChild(m_ScrollLayer);
    m_ScrollLayer->release();

    CCObject* child;

    CCARRAY_FOREACH(m_RunsList->m_listView->m_tableView->m_cellArray, child){
        auto childCell = dynamic_cast<GenericListCell*>(child);
        if (childCell)
            childCell->m_backgroundLayer->setOpacity(30);
    }

    std::vector<CCSprite*> spritesToRemove;
    CCLabelBMFont* title;

    CCARRAY_FOREACH(m_RunsList->getChildren(), child){
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

    title->setScale(0.4f);
    title->setPosition({39, 96});
}

void DTLayer::deleteUnused(CCObject*){
    m_RunDeleteAlert = FLAlertLayer::create(this, "Warning!", "this will delete all saved runs that were not added to the list of runs to track, please make sure you have all of the precents you want on the tracked runs list before doing this.", "Delete", "Cancel");
    m_RunDeleteAlert->setZOrder(101);
    this->addChild(m_RunDeleteAlert);
}

void DTLayer::FLAlert_Clicked(FLAlertLayer* layer, bool selected){
    if (m_RunDeleteAlert == layer && !selected){
        for (auto it = m_MyLevelStats.runs.cbegin(); it != m_MyLevelStats.runs.cend();)
        {
            bool erase = true;
            for (int i = 0; i < m_MyLevelStats.RunsToSave.size(); i++)
            {
                
                if (StatsManager::splitRunKey(it->first).start == m_MyLevelStats.RunsToSave[i]){
                    erase = false;
                }
            }
            if (erase){
                m_MyLevelStats.runs.erase(it++);
            }
            else{
                ++it;
            }
        }

        for (int i = 0; i < m_MyLevelStats.sessions.size(); i++)
        {
            for (auto it = m_MyLevelStats.sessions[i].runs.cbegin(); it != m_MyLevelStats.sessions[i].runs.cend();)
            {
                bool erase = true;
                for (int i = 0; i < m_MyLevelStats.RunsToSave.size(); i++)
                {
                    
                    if (StatsManager::splitRunKey(it->first).start == m_MyLevelStats.RunsToSave[i]){
                        erase = false;
                    }
                }
                if (erase){
                    m_MyLevelStats.sessions[i].runs.erase(it++);
                }
                else{
                    ++it;
                }
            }
        }
        

        StatsManager::saveData(m_MyLevelStats, m_Level);
    }

}

void DTLayer::onClose(cocos2d::CCObject*) {
    if (m_EditLayoutMenu->isVisible()){
        EditLayoutEnabled(false);
    }
    else{
        this->setKeypadEnabled(false);
        this->setTouchEnabled(false);
        this->removeFromParentAndCleanup(true);
    }
}

void DTLayer::openGraphMenu(CCObject*){
    auto graph = DTGraphLayer::create(this);
    graph->setZOrder(100);
    this->addChild(graph);
}

void DTLayer::OnToggleAllRuns(CCObject*){
    if (allRunsToggle->isOn()){
        
        if (m_MyLevelStats.RunsToSave.size()){
            if (m_MyLevelStats.RunsToSave[0] == -1){
                m_MyLevelStats.RunsToSave.erase(m_MyLevelStats.RunsToSave.begin());

                updateRunsAllowed();
            }
        }
    }
    else{
        m_MyLevelStats.RunsToSave.insert(m_MyLevelStats.RunsToSave.begin(), -1);
        updateRunsAllowed();
    }
}

void DTLayer::OnLinkButtonClicked(CCObject*){
    auto lLayer = DTLinkLayer::create(this);
    lLayer->setZOrder(100);
    m_mainLayer->addChild(lLayer);
}

void DTLayer::UpdateSharedStats(){
    m_SharedLevelStats = m_MyLevelStats;

    for (int i = 0; i < m_MyLevelStats.LinkedLevels.size(); i++)
    {
        auto currStats = StatsManager::getLevelStats(m_MyLevelStats.LinkedLevels[i]);

        m_SharedLevelStats.attempts += currStats.attempts;

        m_SharedLevelStats.sessions.reserve(m_SharedLevelStats.sessions.size() + distance(currStats.sessions.begin(),currStats.sessions.end()));
        m_SharedLevelStats.sessions.insert(m_SharedLevelStats.sessions.end(),currStats.sessions.begin(),currStats.sessions.end());

        for (int r = 0; r < m_MyLevelStats.RunsToSave.size(); r++)
        {
            bool addMeRun = true;
            for (int r2 = 0; r2 < currStats.RunsToSave.size(); r2++)
            {
                if (currStats.RunsToSave[r2] == m_MyLevelStats.RunsToSave[r])
                    addMeRun = false;
            }
            
            if (addMeRun)
                currStats.RunsToSave.push_back(m_MyLevelStats.RunsToSave[r]);
        }
        for (int r = 0; r < currStats.RunsToSave.size(); r++)
        {
            bool addMeRun = true;
            for (int r2 = 0; r2 < m_MyLevelStats.RunsToSave.size(); r2++)
            {
                if (m_MyLevelStats.RunsToSave[r2] == currStats.RunsToSave[r])
                    addMeRun = false;
            }
            
            if (addMeRun)
                m_MyLevelStats.RunsToSave.push_back(currStats.RunsToSave[r]);
        }

        std::ranges::sort(m_MyLevelStats.RunsToSave, [](const int a, const int b) {
            return a < b;
        });

        std::ranges::sort(currStats.RunsToSave, [](const int a, const int b) {
            return a < b;
        });

        StatsManager::saveData(m_MyLevelStats, m_Level);
        StatsManager::saveData(currStats, m_MyLevelStats.LinkedLevels[i]);
        
        for (const auto& [death, count] : currStats.deaths)
        {
            m_SharedLevelStats.deaths[death] += count;
        }
        
        for (const auto& [run, count] : currStats.runs)
        {
            m_SharedLevelStats.runs[run] += count;
        }

        if (m_SharedLevelStats.currentBest < currStats.currentBest)
            m_SharedLevelStats.currentBest = currStats.currentBest;

        m_SharedLevelStats.newBests.insert(currStats.newBests.begin(), currStats.newBests.end());
    }

    std::ranges::sort(m_SharedLevelStats.sessions, [](const Session a, const Session b) {
        return a.lastPlayed > b.lastPlayed;
    });

    m_SessionsAmount = m_SharedLevelStats.sessions.size();

    if (m_SessionSelectionInput)
        m_SessionSelectionInput->setString(fmt::format("{}/{}", m_SessionSelected, m_SessionsAmount));
    
}