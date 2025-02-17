#include "../layers/DTGraphLayer.hpp"
#include "../layers/ChooseRunCell.hpp"

DTGraphLayer* DTGraphLayer::create(DTLayer* const& layer) {
    auto ret = new DTGraphLayer();
    if (ret && ret->init(520, 280, layer, "square01_001.png", {0.f, 0.f, 94.f, 94.f})) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool DTGraphLayer::setup(DTLayer* const& layer) {

    m_DTLayer = layer;

    this->setOpacity(0);

    noGraphLabel = CCLabelBMFont::create("No Progress\nFor Graph", "bigFont.fnt");
    noGraphLabel->setZOrder(1);
    noGraphLabel->setVisible(false);
    noGraphLabel->setPosition({318, 161});
    this->addChild(noGraphLabel);

    refreshGraph();

    CCScale9Sprite* FontTextDisplayBG = CCScale9Sprite::create("square02b_001.png", {0,0, 80, 80});
    FontTextDisplayBG->setPosition({318, 161});
    FontTextDisplayBG->setContentSize({430, 256});
    FontTextDisplayBG->setColor({0,0,0});
    FontTextDisplayBG->setOpacity(125);
    this->addChild(FontTextDisplayBG);

    CCScale9Sprite* InfoBG = CCScale9Sprite::create("square02b_001.png", {0,0, 80, 80});
    InfoBG->setPosition({69, 75});
    InfoBG->setContentSize({65, 83});
    InfoBG->setColor({ 113, 167, 255 });
    InfoBG->setOpacity(78);
    this->addChild(InfoBG);

    auto InfoLabel = CCLabelBMFont::create("Point Info", "bigFont.fnt");
    InfoLabel->setPosition({69, 124});
    InfoLabel->setScale(0.35f);
    this->addChild(InfoLabel);

    npsLabel = CCLabelBMFont::create("No\nPoint\nSelected", "bigFont.fnt");
    npsLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    npsLabel->setScale(0.35f);
    npsLabel->setPosition({69, 75});
    npsLabel->setZOrder(1);
    npsLabel->setVisible(false);
    npsLabel->setPositionY(npsLabel->getPositionY());
    this->addChild(npsLabel);

    PointInfoLabel = SimpleTextArea::create("Precent\n \nPassrate:\npassrate", "bigFont.fnt");
    PointInfoLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    PointInfoLabel->setPosition({69, 75});
    PointInfoLabel->setZOrder(1);
    PointInfoLabel->setVisible(false);
    PointInfoLabel->setScale(0.35f);
    this->addChild(PointInfoLabel);

    auto SessionSelectCont = CCNode::create();
    SessionSelectCont->setID("Session-Select-Container");
    SessionSelectCont->setPosition({69, 210});
    SessionSelectCont->setScale(0.85f);
    m_mainLayer->addChild(SessionSelectCont);

    auto m_SessionSelectMenu = CCMenu::create();
    m_SessionSelectMenu->setPosition({0, 0});
    SessionSelectCont->addChild(m_SessionSelectMenu);

    std::ranges::sort(m_DTLayer->m_SharedLevelStats.sessions, [](const Session a, const Session b) {
        return a.lastPlayed > b.lastPlayed;
    });

    m_SessionSelectionInput = InputNode::create(120, "Session");
    if (m_DTLayer->m_SessionsAmount == 0)
        m_SessionSelectionInput->setString("No sessions.");
    else
        m_SessionSelectionInput->setString(fmt::format("{}/{}", m_DTLayer->m_SessionSelected, m_DTLayer->m_SessionsAmount));
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
        menu_selector(DTGraphLayer::switchedSessionRight)
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
        menu_selector(DTGraphLayer::switchedSessionLeft)
    );
    SessionSelectionLeft->setPosition({-34, 0});
    SessionSelectionLeft->setRotation(180);
    m_SessionSelectMenu->addChild(SessionSelectionLeft);

    auto SessionSelectionLabel = CCLabelBMFont::create("Session", "bigFont.fnt");
    SessionSelectionLabel->setPosition({0, 16});
    SessionSelectionLabel->setScale(0.45f);
    SessionSelectCont->addChild(SessionSelectionLabel);

    auto viewModeLabel = CCLabelBMFont::create("View Mode", "bigFont.fnt");
    viewModeLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    viewModeLabel->setScale(0.35f);
    viewModeLabel->setPosition({69, 275});
    this->addChild(viewModeLabel);

    viewModeButtonS = ButtonSprite::create("Normal");
    viewModeButtonS->setScale(0.475f);
    auto viewModeButton = CCMenuItemSpriteExtra::create(
        viewModeButtonS,
        nullptr,
        this,
        menu_selector(DTGraphLayer::onViewModeButton)
    );
    viewModeButton->setPosition({-215, 99});
    this->m_buttonMenu->addChild(viewModeButton);

    runViewModeButtonS = ButtonSprite::create("From 0");
    runViewModeButtonS->setScale(0.475f);
    auto runViewModeButton = CCMenuItemSpriteExtra::create(
        runViewModeButtonS,
        nullptr,
        this,
        menu_selector(DTGraphLayer::onRunViewModeButton)
    );
    runViewModeButton->setPosition({-215, 79});
    this->m_buttonMenu->addChild(runViewModeButton);

    m_RunSelectInput = InputNode::create(120, "Run %");
    m_RunSelectInput->getInput()->setDelegate(this);
    m_RunSelectInput->getInput()->setAllowedChars("1234567890");
    m_RunSelectInput->setScale(0.45f);
    m_RunSelectInput->setPosition({69, 195});
    this->addChild(m_RunSelectInput);

    CCArray* runsAllowed = CCArray::create();

    for (int i = 0; i < m_DTLayer->m_MyLevelStats.RunsToSave.size(); i++)
    {
        runsAllowed->addObject(ChooseRunCell::create(m_DTLayer->m_MyLevelStats.RunsToSave[i], this));
    }
    
    auto runsAllowedView = ListView::create(runsAllowed, 20, 65, 55);

    m_RunsList = GJListLayer::create(runsAllowedView, "", {0,0,0,75}, 65, 55, 1);
    m_RunsList->setPosition({36, 130});
    m_mainLayer->addChild(m_RunsList);

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
        if (childSprite)
            spritesToRemove.push_back(childSprite);
    }

    for (int i = 0; i < spritesToRemove.size(); i++)
    {
        spritesToRemove[i]->removeMeAndCleanup();
    }

    scheduleUpdate();

    return true;
}

void DTGraphLayer::update(float delta){
    if (pointToDisplay.size() == 0){
        npsLabel->setVisible(true);
        PointInfoLabel->setVisible(false);
    }
    else{
        PointInfoLabel->setVisible(true);
        npsLabel->setVisible(false);

        std::string runFixed = pointToDisplay[0]->m_Run;

        if (runFixed != "<" && runFixed.length() > 1){
            //new best color
            if (m_DTLayer->isKeyInIndex(runFixed, 1, "nbc>")){
                runFixed.erase(0, 5);
            }
            //sessions best color
            if (m_DTLayer->isKeyInIndex(runFixed, 1, "sbc>")){
                runFixed.erase(0, 5);
            }
        }

        PointInfoLabel->setText(fmt::format("Run:\n{}\n \nPassrate:\n{:.2f}%", runFixed, pointToDisplay[0]->m_Passrate));
    }
}

void DTGraphLayer::onViewModeButton(CCObject*){
    if (ViewModeNormal){
        ViewModeNormal = false;
        viewModeButtonS->m_label->setString("Session");
    }
    else{
        ViewModeNormal = true;
        viewModeButtonS->m_label->setString("Normal");
    }
    refreshGraph();
}

void DTGraphLayer::onRunViewModeButton(CCObject*){
    if (RunViewModeFromZero){
        RunViewModeFromZero = false;
        runViewModeButtonS->m_label->setString("Runs");
    }
    else{
        RunViewModeFromZero = true;
        runViewModeButtonS->m_label->setString("From 0");
    }
    refreshGraph();
}

void DTGraphLayer::textChanged(CCTextInputNode* input){
    if (input == m_SessionSelectionInput->getInput() && m_DTLayer->m_SessionsAmount > 0){
        int selected = 1;
        if (input->getString() != "")
            selected = std::stoi(input->getString());
        
        if (selected > m_DTLayer->m_SessionsAmount){
            selected = m_DTLayer->m_SessionsAmount;
            input->setString(fmt::format("{}", m_DTLayer->m_SessionsAmount));
        }

        if (selected < 1){
            selected = 1;
            input->setString("1");
        }

        m_DTLayer->m_SessionSelectionInput->setString(fmt::format("{}/{}", selected, m_DTLayer->m_SessionsAmount));

        m_DTLayer->m_SessionSelected = selected;
        m_DTLayer->updateSessionString(m_DTLayer->m_SessionSelected);
        if (!ViewModeNormal)
            refreshGraph();
    }

    if (input == m_RunSelectInput->getInput()){
        int selected = 0;
        if (input->getString() != "")
            selected = std::stoi(input->getString());

        if (selected > 100){
            selected = 100;
            input->setString("100");
        }

        m_SelectedRunPrecent = selected;

        if (!RunViewModeFromZero)
            refreshGraph();
    }
}

void DTGraphLayer::textInputOpened(CCTextInputNode* input){
    if (input == m_SessionSelectionInput->getInput() && m_DTLayer->m_SessionsAmount > 0){
        input->setString(fmt::format("{}", m_DTLayer->m_SessionSelected));
        m_DTLayer->m_SessionSelectionInputSelected = true;
    }
}

void DTGraphLayer::textInputClosed(CCTextInputNode* input){
    if (input == m_SessionSelectionInput->getInput() && m_DTLayer->m_SessionsAmount > 0){
        input->setString(fmt::format("{}/{}", m_DTLayer->m_SessionSelected, m_DTLayer->m_SessionsAmount));
        m_DTLayer->m_SessionSelectionInputSelected = false;
    }
}

/*
    //
    creates a graph with a given deaths string
    change the scaling to change the space between the points on the x and y
    //
*/
CCNode* DTGraphLayer::CreateGraph(std::vector<std::tuple<std::string, int, float>> deathsString, int bestRun, ccColor3B color, CCPoint Scaling, ccColor4B graphBoxOutlineColor, ccColor4B graphBoxFillColor, float graphBoxOutlineThickness, ccColor4B labelLineColor, ccColor4B labelColor, int labelEvery, ccColor4B gridColor, int gridLineEvery){
    if (std::get<0>(deathsString[0]) == "-1" || std::get<0>(deathsString[0]) == "No Saved Progress") return nullptr;

    auto toReturnNode = CCNode::create();

    auto LabelsNode = CCNode::create();
    toReturnNode->addChild(LabelsNode);

    CCPoint MaskShape[4] = {
        ccp(0, 0),
        ccp(100 * Scaling.x, 0),
        ccp(100 * Scaling.x, 100 * Scaling.y),
        ccp(0, 100 * Scaling.y)
    };

    auto clippingNode = CCClippingNode::create();
    toReturnNode->addChild(clippingNode);

    auto mask = CCDrawNode::create();
    mask->drawPolygon(MaskShape, 4, ccc4FFromccc4B(graphBoxFillColor), graphBoxOutlineThickness, ccc4FFromccc4B(graphBoxOutlineColor));
    clippingNode->setStencil(mask);
    clippingNode->addChild(mask);

    auto MenuForGP = CCMenu::create();
    MenuForGP->setPosition({0,0});
    MenuForGP->setZOrder(1);
    toReturnNode->addChild(MenuForGP);

    std::vector<CCPoint> lines;

    for (int i = 0; i < deathsString.size(); i++)
    {
        //remove extra coloring
        std::string editedDString = std::get<0>(deathsString[i]);

        if (StatsManager::ContainsAtIndex(0, "<nbc>", editedDString) || StatsManager::ContainsAtIndex(0, "<sbc>", editedDString)){
            std::get<0>(deathsString[i]) = editedDString.erase(0, 5);
        }
    }
    
    //sort
    std::ranges::sort(deathsString, [](const std::tuple<std::string, int, float> a, const std::tuple<std::string, int, float> b) {
        //log::info("a:{}, b:{}", a, b);
        auto percentA = std::stoi(std::get<0>(a));
        auto percentB = std::stoi(std::get<0>(b));
        return percentA < percentB; // true --> A before B
    });

    //log::info("sorting done");

    //add the min and max points if needed
    if (std::stoi(std::get<0>(deathsString[0])) > 0)
        deathsString.insert(deathsString.begin(), std::tuple<std::string, int, float>{"0", 0, 100});
    
    if (std::stoi(std::get<0>(deathsString[deathsString.size() - 1])) < 100)
        deathsString.push_back(std::tuple<std::string, int, float>{"100", 0, 0});
    else
        std::get<2>(deathsString[deathsString.size() - 1]) = 100;
    

    //log::info("added extras");

    CCPoint previousPoint = ccp(-1, -1);

    for (int i = 0; i < deathsString.size(); i++)
    {
        //save point
        CCPoint myPoint = ccp(std::stoi(std::get<0>(deathsString[i])), std::get<2>(deathsString[i]));

        
        //add extra points
        if (previousPoint.x != -1){

            //add a before point if needed
            if (previousPoint.x != myPoint.x - 1){

                if (previousPoint.x != myPoint.x - 2 && previousPoint.y != 100 && previousPoint.x + 1 <= bestRun){
                    lines.push_back(ccp(previousPoint.x + 1, 100) * Scaling);
                }

                if (myPoint.x - 1 <= bestRun && myPoint.y != 100)
                    lines.push_back(ccp(myPoint.x - 1, 100) * Scaling);
            }
        }

        lines.push_back(myPoint * Scaling);
        previousPoint = myPoint;
    }

    //log::info("added lines");

    ccColor3B colorOfPoints;

    //add points
    if ((color.r + color.g + color.b) / 3 > 200)
        colorOfPoints = {255, 255, 255};
    else
        colorOfPoints = { 136, 136, 136};

    for (int i = 0; i < lines.size(); i++)
    {
        auto GP = GraphPoint::create(fmt::format("{}%", lines[i].x / Scaling.x), lines[i].y / Scaling.y, colorOfPoints);
        GP->setDelegate(this);
        GP->setPosition(lines[i]);
        GP->setScale(0.05f);
        MenuForGP->addChild(GP);
    }

    //add wrapping
    lines.push_back(ccp(lines[lines.size() - 1].x + 100, lines[lines.size() - 1].y));
    lines.push_back(ccp(lines[lines.size() - 1].x + 100, -100));
    lines.push_back(ccp(-100, -100));
    lines.push_back(ccp(-100, lines[0].y));

    //create graph
    auto line = CCDrawNode::create();
    line->drawPolygon(&lines[0], lines.size(), ccc4FFromccc4B({ 0, 0, 0, 0}), 1, ccc4FFromccc3B(color));
    clippingNode->addChild(line);

    //create measuring labels
    auto tempT = CCLabelBMFont::create("100", "chatFont.fnt");
    tempT->setScale(0.4f);
    float XForPr = tempT->getScaledContentSize().width;

    auto gridNode = CCDrawNode::create();
    gridNode->setZOrder(-1);
    clippingNode->addChild(gridNode);

    for (int i = 0; i <= 100; i++)
    {
        auto labelPr = CCSprite::createWithSpriteFrameName("gridLine01_001.png");
        labelPr->setPositionX(i * Scaling.x);
        labelPr->setRotation(90);
        labelPr->setColor({labelLineColor.r, labelLineColor.g, labelLineColor.b});
        labelPr->setOpacity(labelLineColor.a);
        LabelsNode->addChild(labelPr);
        
        if (floor(static_cast<float>(i) / labelEvery) == static_cast<float>(i) / labelEvery){
            labelPr->setScaleX(0.2f);

            auto labelPrText = CCLabelBMFont::create(std::to_string(i).c_str(), "chatFont.fnt");
            labelPrText->setPositionX(i * Scaling.x);
            labelPrText->setScale(0.4f);
            labelPrText->setPositionY(-labelPr->getScaledContentSize().width - labelPrText->getScaledContentSize().height);
            labelPrText->setColor({labelColor.r, labelColor.g, labelColor.b});
            labelPrText->setOpacity(labelColor.a);
            LabelsNode->addChild(labelPrText);
        }
        else{
            labelPr->setScaleX(0.1f);
            labelPr->setScaleY(0.8f);
        }
        labelPr->setPositionY(-labelPr->getScaledContentSize().width);
        

        //

        auto labelPS = CCSprite::createWithSpriteFrameName("gridLine01_001.png");
        labelPS->setPositionY(i * Scaling.y);
        labelPS->setColor({labelLineColor.r, labelLineColor.g, labelLineColor.b});
        labelPS->setOpacity(labelLineColor.a);
        LabelsNode->addChild(labelPS);

        if (floor(static_cast<float>(i) / labelEvery) == static_cast<float>(i) / labelEvery){
            labelPS->setScaleX(0.2f);

            auto labelPSText = CCLabelBMFont::create(std::to_string(i).c_str(), "chatFont.fnt");
            labelPSText->setPositionY(i * Scaling.y);
            labelPSText->setScale(0.4f);
            labelPSText->setPositionX(-labelPS->getScaledContentSize().width - XForPr);
            labelPSText->setColor({labelColor.r, labelColor.g, labelColor.b});
            labelPSText->setOpacity(labelColor.a);
            LabelsNode->addChild(labelPSText);
        }
        else{
            labelPS->setScaleX(0.1f);
            labelPS->setScaleY(0.8f);
        }

        labelPS->setPositionX(-labelPS->getScaledContentSize().width);

        //add grid

        if (floor(static_cast<float>(i) / gridLineEvery) == static_cast<float>(i) / gridLineEvery){
            CCPoint gridLineH[4]{
                ccp(0, i * Scaling.y),
                ccp(0, i * Scaling.y),
                ccp(100 * Scaling.x, i * Scaling.y),
                ccp(100 * Scaling.x, i * Scaling.y)
            };

            CCPoint gridLineS[4]{
                ccp(i * Scaling.x, 0),
                ccp(i * Scaling.x, 0),
                ccp(i * Scaling.x, 100 * Scaling.y),
                ccp(i * Scaling.x, 100 * Scaling.y)
            };

            gridNode->drawPolygon(gridLineH, 4, ccc4FFromccc4B(gridColor), 0.2f, ccc4FFromccc4B(gridColor));
            gridNode->drawPolygon(gridLineS, 4, ccc4FFromccc4B(gridColor), 0.2f, ccc4FFromccc4B(gridColor));
        }
    }
    

    return toReturnNode;
}

CCNode* DTGraphLayer::CreateRunGraph(std::vector<std::tuple<std::string, int, float>> deathsString, int bestRun, ccColor3B color, CCPoint Scaling, ccColor4B graphBoxOutlineColor, ccColor4B graphBoxFillColor, float graphBoxOutlineThickness, ccColor4B labelLineColor, ccColor4B labelColor, int labelEvery, ccColor4B gridColor, int gridLineEvery){
    if (std::get<0>(deathsString[0]) == "-1" || std::get<0>(deathsString[0]) == "No Saved Progress") return nullptr;

    auto toReturnNode = CCNode::create();

    auto LabelsNode = CCNode::create();
    toReturnNode->addChild(LabelsNode);

    CCPoint MaskShape[4] = {
        ccp(0, 0),
        ccp(100 * Scaling.x, 0),
        ccp(100 * Scaling.x, 100 * Scaling.y),
        ccp(0, 100 * Scaling.y)
    };

    auto clippingNode = CCClippingNode::create();
    toReturnNode->addChild(clippingNode);

    auto mask = CCDrawNode::create();
    mask->drawPolygon(MaskShape, 4, ccc4FFromccc4B(graphBoxFillColor), graphBoxOutlineThickness, ccc4FFromccc4B(graphBoxOutlineColor));
    clippingNode->setStencil(mask);
    clippingNode->addChild(mask);

    auto MenuForGP = CCMenu::create();
    MenuForGP->setPosition({0,0});
    MenuForGP->setZOrder(1);
    toReturnNode->addChild(MenuForGP);

    float RunStartPrecent = StatsManager::splitRunKey(std::get<0>(deathsString[0])).start;

    std::vector<CCPoint> lines;
    
    //sort
    std::ranges::sort(deathsString, [](const std::tuple<std::string, int, float> a, const std::tuple<std::string, int, float> b) {
        auto runA = StatsManager::splitRunKey(std::get<0>(a));
        auto runB = StatsManager::splitRunKey(std::get<0>(b));

        // start is equal, compare end
        if (runA.start == runB.start) return runA.end < runB.end;
        return runA.start < runB.start;
    });

    //log::info("sorting done");

    std::get<2>(deathsString[deathsString.size() - 1]) = 0;

    //add the min and max points if needed
    if (StatsManager::splitRunKey(std::get<0>(deathsString[0])).end > RunStartPrecent)
        deathsString.insert(deathsString.begin(), std::tuple<std::string, int, float>{fmt::format("{}-{}", RunStartPrecent, RunStartPrecent), 0, 100});
    
    if (StatsManager::splitRunKey(std::get<0>(deathsString[deathsString.size() - 1])).end < 100)
        deathsString.push_back(std::tuple<std::string, int, float>{fmt::format("{}-100", RunStartPrecent), 100, 0});
    else
        std::get<2>(deathsString[deathsString.size() - 1]) = 100;
    

    //log::info("added extras");

    CCPoint previousPoint = ccp(-1, -1);

    for (int i = 0; i < deathsString.size(); i++)
    {
        //save point
        CCPoint myPoint = ccp(StatsManager::splitRunKey(std::get<0>(deathsString[i])).end, std::get<2>(deathsString[i]));

        
        //add extra points
        if (previousPoint.x != -1){

            //add a before point if needed
            if (previousPoint.x != myPoint.x - 1){

                if (previousPoint.x != myPoint.x - 2 && previousPoint.y != 100 && previousPoint.x + 1 <= bestRun){
                    lines.push_back(ccp(previousPoint.x + 1, 100) * Scaling);
                }

                if (myPoint.x - 1 <= bestRun && myPoint.y != 100)
                    lines.push_back(ccp(myPoint.x - 1, 100) * Scaling);
            }
        }

        lines.push_back(myPoint * Scaling);
        previousPoint = myPoint;
    }

    //log::info("added lines");

    ccColor3B colorOfPoints;

    if ((color.r + color.g + color.b) / 3 > 200)
        colorOfPoints = {255, 255, 255};
    else
        colorOfPoints = { 136, 136, 136};

    for (int i = 0; i < lines.size(); i++)
    {
        auto GP = GraphPoint::create(fmt::format("{}% - {}%", RunStartPrecent, lines[i].x / Scaling.x), lines[i].y / Scaling.y, colorOfPoints);
        GP->setDelegate(this);
        GP->setPosition(lines[i]);
        GP->setScale(0.05f);
        MenuForGP->addChild(GP);
    }

    //add wrapping
    lines.push_back(ccp(lines[lines.size() - 1].x + 100, lines[lines.size() - 1].y));
    lines.push_back(ccp(lines[lines.size() - 1].x + 100, -100));
    lines.push_back(ccp(lines[0].x, -100));

    auto line = CCDrawNode::create();
    line->drawPolygon(&lines[0], lines.size(), ccc4FFromccc4B({ 0, 0, 0, 0}), 1, ccc4FFromccc3B(color));
    clippingNode->addChild(line);

    auto tempT = CCLabelBMFont::create("100", "chatFont.fnt");
    tempT->setScale(0.4f);
    float XForPr = tempT->getScaledContentSize().width;

    auto gridNode = CCDrawNode::create();
    gridNode->setZOrder(-1);
    clippingNode->addChild(gridNode);

    for (int i = 0; i <= 100; i++)
    {
        auto labelPr = CCSprite::createWithSpriteFrameName("gridLine01_001.png");
        labelPr->setPositionX(i * Scaling.x);
        labelPr->setRotation(90);
        labelPr->setColor({labelLineColor.r, labelLineColor.g, labelLineColor.b});
        labelPr->setOpacity(labelLineColor.a);
        LabelsNode->addChild(labelPr);
        
        if (floor(static_cast<float>(i) / labelEvery) == static_cast<float>(i) / labelEvery){
            labelPr->setScaleX(0.2f);

            auto labelPrText = CCLabelBMFont::create(std::to_string(i).c_str(), "chatFont.fnt");
            labelPrText->setPositionX(i * Scaling.x);
            labelPrText->setScale(0.4f);
            labelPrText->setPositionY(-labelPr->getScaledContentSize().width - labelPrText->getScaledContentSize().height);
            labelPrText->setColor({labelColor.r, labelColor.g, labelColor.b});
            labelPrText->setOpacity(labelColor.a);
            LabelsNode->addChild(labelPrText);
        }
        else{
            labelPr->setScaleX(0.1f);
            labelPr->setScaleY(0.8f);
        }
        labelPr->setPositionY(-labelPr->getScaledContentSize().width);
        

        //

        auto labelPS = CCSprite::createWithSpriteFrameName("gridLine01_001.png");
        labelPS->setPositionY(i * Scaling.y);
        labelPS->setColor({labelLineColor.r, labelLineColor.g, labelLineColor.b});
        labelPS->setOpacity(labelLineColor.a);
        LabelsNode->addChild(labelPS);

        if (floor(static_cast<float>(i) / labelEvery) == static_cast<float>(i) / labelEvery){
            labelPS->setScaleX(0.2f);

            auto labelPSText = CCLabelBMFont::create(std::to_string(i).c_str(), "chatFont.fnt");
            labelPSText->setPositionY(i * Scaling.y);
            labelPSText->setScale(0.4f);
            labelPSText->setPositionX(-labelPS->getScaledContentSize().width - XForPr);
            labelPSText->setColor({labelColor.r, labelColor.g, labelColor.b});
            labelPSText->setOpacity(labelColor.a);
            LabelsNode->addChild(labelPSText);
        }
        else{
            labelPS->setScaleX(0.1f);
            labelPS->setScaleY(0.8f);
        }

        labelPS->setPositionX(-labelPS->getScaledContentSize().width);

        //grid

        if (floor(static_cast<float>(i) / gridLineEvery) == static_cast<float>(i) / gridLineEvery){
            CCPoint gridLineH[4]{
                ccp(0, i * Scaling.y),
                ccp(0, i * Scaling.y),
                ccp(100 * Scaling.x, i * Scaling.y),
                ccp(100 * Scaling.x, i * Scaling.y)
            };

            CCPoint gridLineS[4]{
                ccp(i * Scaling.x, 0),
                ccp(i * Scaling.x, 0),
                ccp(i * Scaling.x, 100 * Scaling.y),
                ccp(i * Scaling.x, 100 * Scaling.y)
            };

            gridNode->drawPolygon(gridLineH, 4, ccc4FFromccc4B(gridColor), 0.2f, ccc4FFromccc4B(gridColor));
            gridNode->drawPolygon(gridLineS, 4, ccc4FFromccc4B(gridColor), 0.2f, ccc4FFromccc4B(gridColor));
        }
    }
    

    return toReturnNode;
}

int DTGraphLayer::GetBestRun(NewBests bests){
    int bestRun = 0;

    for (auto const& best : bests)
    {
        if (best > bestRun) bestRun = best;
    }

    return bestRun;
}

int DTGraphLayer::GetBestRun(std::vector<std::tuple<std::string, int, float>> selectedPrecentRunInfo){
    if (std::get<0>(selectedPrecentRunInfo[0]) == "-1" || std::get<0>(selectedPrecentRunInfo[0]) == "No Saved Progress") return -1;

    int bestRun = 0;

    for (auto const& best : selectedPrecentRunInfo)
    {
        if (StatsManager::splitRunKey(std::get<0>(best)).end > bestRun) bestRun = StatsManager::splitRunKey(std::get<0>(best)).end;
    }

    return bestRun;
}

void DTGraphLayer::OnPointSelected(cocos2d::CCNode* point){
    pointToDisplay.insert(pointToDisplay.end(), static_cast<GraphPoint*>(point));
}

void DTGraphLayer::OnPointDeselected(cocos2d::CCNode* point){
    for (int i = 0; i < pointToDisplay.size(); i++)
    {
        if (pointToDisplay[i] == point){
            pointToDisplay.erase(std::next(pointToDisplay.begin(), i));
            break;
        }
            
    }
    
}

void DTGraphLayer::switchedSessionRight(CCObject*){
    if (m_DTLayer->m_SessionSelected >= m_DTLayer->m_SessionsAmount) return;

    m_DTLayer->m_SessionSelected += 1;
    if (m_DTLayer->m_SessionSelectionInputSelected){
        m_DTLayer->m_SessionSelectionInput->setString(fmt::format("{}", m_DTLayer->m_SessionSelected));
        m_SessionSelectionInput->setString(fmt::format("{}", m_DTLayer->m_SessionSelected));
    }
    else{
        m_DTLayer->m_SessionSelectionInput->setString(fmt::format("{}/{}",m_DTLayer-> m_SessionSelected, m_DTLayer->m_SessionsAmount));
        m_SessionSelectionInput->setString(fmt::format("{}/{}",m_DTLayer-> m_SessionSelected, m_DTLayer->m_SessionsAmount));
    }
    m_DTLayer->updateSessionString(m_DTLayer->m_SessionSelected);
    if (!ViewModeNormal)
        refreshGraph();
}

void DTGraphLayer::switchedSessionLeft(CCObject*){
    if (m_DTLayer->m_SessionSelected - 1 < 1) return;

    m_DTLayer->m_SessionSelected -= 1;
    if (m_DTLayer->m_SessionSelectionInputSelected){
        m_DTLayer->m_SessionSelectionInput->setString(fmt::format("{}", m_DTLayer->m_SessionSelected));
        m_SessionSelectionInput->setString(fmt::format("{}", m_DTLayer->m_SessionSelected));
    }
    else{
        m_DTLayer->m_SessionSelectionInput->setString(fmt::format("{}/{}", m_DTLayer->m_SessionSelected, m_DTLayer->m_SessionsAmount));
        m_SessionSelectionInput->setString(fmt::format("{}/{}", m_DTLayer->m_SessionSelected, m_DTLayer->m_SessionsAmount));
    }
        
    m_DTLayer->updateSessionString(m_DTLayer->m_SessionSelected);
    if (!ViewModeNormal)
        refreshGraph();
}

void DTGraphLayer::onClose(cocos2d::CCObject*) {
    m_DTLayer->RefreshText();
    m_DTLayer->m_SessionSelectionInputSelected = false;
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
}

void DTGraphLayer::refreshGraph(){
    if (m_graph) m_graph->removeMeAndCleanup();

    if (ViewModeNormal){
        if (RunViewModeFromZero){
            m_graph = CreateGraph(m_DTLayer->m_DeathsInfo, GetBestRun(m_DTLayer->m_SharedLevelStats.newBests), Save::getNewBestColor(), {4, 2.3f}, { 124, 124, 124, 255}, {0, 0, 0, 120}, 0.2f, {115, 115, 115, 255}, { 202, 202, 202, 255}, 5, { 29, 29, 29, 255 }, 5);
        }
        else{
            std::vector<std::tuple<std::string, int, float>> selectedPrecentRunInfo;
            for (int i = 0; i < m_DTLayer->m_RunInfo.size(); i++)
            {
                std::string currentRunString = std::get<0>(m_DTLayer->m_RunInfo[i]);

                if (currentRunString != "-1" && currentRunString != "No Saved Progress")
                    if (StatsManager::splitRunKey(currentRunString).start == m_SelectedRunPrecent){
                        selectedPrecentRunInfo.push_back(m_DTLayer->m_RunInfo[i]);
                    }
                        
            }
            
            if (!selectedPrecentRunInfo.size())
                selectedPrecentRunInfo.push_back(std::tuple<std::string, int, float>{"No Saved Progress", -1, 0});

            m_graph = CreateRunGraph(selectedPrecentRunInfo, GetBestRun(selectedPrecentRunInfo), Save::getNewBestColor(), {4, 2.3f}, { 124, 124, 124, 255}, {0, 0, 0, 120}, 0.2f, {115, 115, 115, 255}, { 202, 202, 202, 255}, 5, { 29, 29, 29, 255 }, 5);
        }
        
        if (m_graph){
            m_graph->setPosition({129, 52});
            m_graph->setZOrder(1);
            this->addChild(m_graph);
            noGraphLabel->setVisible(false);
        }
        else{
            noGraphLabel->setVisible(true);
        }
    }
    else{
        if (RunViewModeFromZero){
            m_graph = CreateGraph(m_DTLayer->selectedSessionInfo, GetBestRun(m_DTLayer->m_SharedLevelStats.sessions[m_DTLayer->m_SessionSelected - 1].newBests), Save::getSessionBestColor(), {4, 2.3f}, { 124, 124, 124, 255}, {0, 0, 0, 120}, 0.2f, {115, 115, 115, 255}, { 202, 202, 202, 255}, 5, { 29, 29, 29, 255 }, 5);
        }
        else{
            std::vector<std::tuple<std::string, int, float>> selectedPrecentRunInfo;
            for (int i = 0; i < m_DTLayer->m_SelectedSessionRunInfo.size(); i++)
            {
                std::string currentRunString = std::get<0>(m_DTLayer->m_SelectedSessionRunInfo[i]);

                if (currentRunString != "-1" && currentRunString != "No Saved Progress")
                    if (StatsManager::splitRunKey(currentRunString).start == m_SelectedRunPrecent){
                        selectedPrecentRunInfo.push_back(m_DTLayer->m_SelectedSessionRunInfo[i]);
                    }
            }
            
            if (!selectedPrecentRunInfo.size())
                selectedPrecentRunInfo.push_back(std::tuple<std::string, int, float>{"No Saved Progress", -1, 0});

            m_graph = CreateRunGraph(selectedPrecentRunInfo, GetBestRun(selectedPrecentRunInfo), Save::getSessionBestColor(), {4, 2.3f}, { 124, 124, 124, 255}, {0, 0, 0, 120}, 0.2f, {115, 115, 115, 255}, { 202, 202, 202, 255}, 5, { 29, 29, 29, 255 }, 5);
        }

        if (m_graph){
            m_graph->setPosition({129, 52});
            m_graph->setZOrder(1);
            this->addChild(m_graph);
            noGraphLabel->setVisible(false);
        }
        else{
            noGraphLabel->setVisible(true);
        }
    }
    
}

void DTGraphLayer::RunChosen(int run){
    m_RunSelectInput->setString(std::to_string(run));
    m_SelectedRunPrecent = run;
    if (!RunViewModeFromZero)
        refreshGraph();
}