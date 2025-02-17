#pragma once

#include <Geode/Geode.hpp>
#include "../layers/DTLayer.hpp"
#include "../managers/StatsManager.hpp"
#include "../utils/Save.hpp"

using namespace geode::prelude;

class DTLinkLayer : public Popup<DTLayer* const&>, public TextInputDelegate {
    protected:
        bool setup(DTLayer* const& layer) override;

        void update(float delta);
    public:
        static DTLinkLayer* create(DTLayer* const& layer);

        DTLayer* m_DTLayer;

        void SpacialEditList(GJListLayer* list, CCPoint titlePos, float titleSize);

        void refreshLists();

        void ChangeLevelLinked(std::string levelKey, LevelStats stats, bool erase);

        float CellsWidth = 230;

        bool scrollSwitchLock;

        GJListLayer* m_LevelsList = nullptr;
        GJListLayer* m_LinkedLevelsList = nullptr;

        InputNode* seartchInput;
        std::string m_filterText;
        void textChanged(CCTextInputNode* input);

        std::vector<std::pair<std::string, LevelStats>> m_AllLevels;

        void onClose(CCObject*);
};