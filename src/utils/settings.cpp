#include <Geode/Geode.hpp>

#include <Geode/loader/SettingV3.hpp>
#include <Geode/loader/GameEvent.hpp>

#include "settings.hpp"

using namespace geode::prelude;

SettingsManager* SettingsManager::get() {
	static auto inst = new SettingsManager;
	return inst;
}

void SettingsManager::init() {
	#ifdef GEODE_IS_DESKTOP

	zoomSensitivity = Mod::get()->getSettingValue<float>("zoom-sensitivity");
	listenForSettingChanges<float>("zoom-sensitivity", [&](float sensitivity) {
		zoomSensitivity = sensitivity;
	});
	
	#endif
}