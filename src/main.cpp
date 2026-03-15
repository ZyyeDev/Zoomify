#include <charconv>

#include <Geode/Geode.hpp>

#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>

#include "utils/settings.hpp"

#ifdef GEODE_IS_MOBILE
#include "platforms/mobile.hpp"
#endif

using namespace geode::prelude;

float clamp(float d, float min, float max) {
	const float t = d < min ? min : d;
	return t > max ? max : t;
}

$execute {
	geode::log::info("Zoom mod loaded!");
	geode::log::info("Platform: " GEODE_PLATFORM_NAME);

	SettingsManager::get()->init();
}