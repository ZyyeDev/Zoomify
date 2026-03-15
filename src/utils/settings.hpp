#pragma once

class SettingsManager {
public:
	static SettingsManager* get();
	void init();

	#ifdef GEODE_IS_DESKTOP
	float zoomSensitivity;
	#endif
};