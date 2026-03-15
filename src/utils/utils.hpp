#pragma once

#include <Geode/Geode.hpp>
using namespace geode::prelude;

CCSize getScreenSize();
void clampPlayLayerPos(CCNode* playLayer);
void zoomPlayLayer(CCNode* playLayer, float zoom, CCPoint screenAnchor);