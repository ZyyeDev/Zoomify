#ifdef GEODE_IS_DESKTOP

#include "desktop.hpp"

#include "../utils/utils.hpp"
#include "../utils/settings.hpp"

#include <Geode/loader/SettingV3.hpp>
#include <Geode/loader/GameEvent.hpp>

#include <Geode/Geode.hpp>
#include <Geode/cocos/cocoa/CCArray.h>

#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>
#include <Geode/modify/PlayLayer.hpp>
#ifdef GEODE_IS_WINDOWS
#include <windows.h>
#include <Geode/modify/CCEGLView.hpp>
#else
#include <objc/message.h>
#endif // GEODE_IS_WINDOWS
#include <Geode/modify/CCScheduler.hpp>

using namespace geode::prelude;

bool pauseMenu = true;

WindowsZoomManager* WindowsZoomManager::get() {
	static auto inst = new WindowsZoomManager;
	return inst;
}

void WindowsZoomManager::setPauseMenuVisible(bool visible) {
    auto* pauseLayer = typeinfo_cast<PauseLayer*>(CCScene::get()->getChildByID("PauseLayer"));
    if (!pauseLayer) return;

	pauseMenu = visible;

    for (auto* node : CCArrayExt<CCNode>(pauseLayer->getChildren())) {
        node->setVisible(visible);
    }
	int opacity = 75;
	if (!visible) opacity = 0;

    pauseLayer->setOpacity(opacity);
}

void WindowsZoomManager::togglePauseMenu() {
	if (!isPaused) return;
	pauseMenu = !pauseMenu;

	setPauseMenuVisible(pauseMenu);
}

void WindowsZoomManager::setZoom(float zoom) {
	CCNode* playLayer = CCScene::get()->getChildByID("PlayLayer");
	if (!playLayer) return;

	playLayer->setScale(zoom);
	onScreenModified();
}

void WindowsZoomManager::zoom(float delta) {
	CCNode* playLayer = CCScene::get()->getChildByID("PlayLayer");
	if (!playLayer) return;

	CCPoint mousePos = getMousePos();

	zoomPlayLayer(playLayer, delta, mousePos);
	onScreenModified();
}

void WindowsZoomManager::move(CCPoint delta) {
	CCNode* playLayer = CCScene::get()->getChildByID("PlayLayer");
	if (!playLayer) return;

	CCPoint pos = playLayer->getPosition();
	playLayer->setPosition(pos + delta);

	onScreenModified();
}

void WindowsZoomManager::setPos(float x, float y) {
	CCNode* playLayer = CCScene::get()->getChildByID("PlayLayer");
	if (!playLayer) return;

	playLayer->setPosition(CCPoint{ x, y });

	onScreenModified();
}

float WindowsZoomManager::getZoom() {
	CCNode* playLayer = CCScene::get()->getChildByID("PlayLayer");
	if (!playLayer) return 1.0f;

	return playLayer->getScale();
}

CCPoint WindowsZoomManager::screenToWorld(CCPoint pos) {
	CCSize screenSize = getScreenSize();
	CCSize winSize = CCEGLView::get()->getFrameSize();

	return CCPoint{ pos.x * (screenSize.width / winSize.width), pos.y * (screenSize.height / winSize.height) };
}

CCPoint WindowsZoomManager::getMousePosOnNode(CCNode* node) {
	return node->convertToNodeSpace(getMousePos());
}

void WindowsZoomManager::update(float dt) {
	auto mousePos = getMousePos();
	auto lastMousePos = WindowsZoomManager::get()->lastMousePos;

	WindowsZoomManager::get()->deltaMousePos = CCPoint{ mousePos.x - lastMousePos.x, mousePos.y - lastMousePos.y };
	WindowsZoomManager::get()->lastMousePos = mousePos;

	if (!isPaused) return;

	if (isPanning) {
		CCPoint delta = WindowsZoomManager::get()->deltaMousePos;
		move(delta);
	}
}

void WindowsZoomManager::onResume() {
	setZoom(1.0f);
	setPos(0.0f, 0.0f);
	setPauseMenuVisible(true);

	isPaused = false;
	WindowsZoomManager::get()->isPanning = false;
}

void WindowsZoomManager::onPause() {
	isPaused = true;
	WindowsZoomManager::get()->isPanning = false;
    pauseMenu = true;
}

void WindowsZoomManager::onScroll(float y, float x) {
	if (!isPaused) return;

	CCNode* playLayer = CCScene::get()->getChildByID("PlayLayer");
	if (!playLayer) return;
	
	float zoomDelta = SettingsManager::get()->zoomSensitivity * 0.1f;
	
	if (Loader::get()->isModLoaded("prevter.smooth-scroll")) {
		zoom(-y * zoomDelta * 0.1f);
	} else if (y > 0) {
		zoom(-zoomDelta);
	} else {
		zoom(zoomDelta);
	}

	if (y > 0) {
		if (playLayer->getScale() <= 1.01f) {
			setPauseMenuVisible(true);
		}
	} else {
		setPauseMenuVisible(false);
	}
}

void WindowsZoomManager::onScreenModified() {
	CCNode* playLayer = CCScene::get()->getChildByID("PlayLayer");
	if (!playLayer) return;

	clampPlayLayerPos(playLayer);
	if (!isPaused) return;
}


$on_game(Loaded) {
	// i should change it to be a for loop but for now
	// its gonna be like that
	std::string toggleMenuKey = "toggle_menu";
	listenForKeybindSettingPresses(toggleMenuKey, [toggleMenuKey](Keybind const&, bool down, bool repeat, double) {
		if (down) {
			WindowsZoomManager::get()->togglePauseMenu();
		}
	});
}

class $modify(PauseLayer) {
	void onResume(CCObject* sender) {
		WindowsZoomManager::get()->onResume();
		PauseLayer::onResume(sender);
	}

	void onRestart(CCObject* sender) {
		WindowsZoomManager::get()->onResume();
		PauseLayer::onRestart(sender);
	}

	void onRestartFull(CCObject* sender) {
		WindowsZoomManager::get()->onResume();
		PauseLayer::onRestartFull(sender);
	}

	void onNormalMode(CCObject* sender) {
		WindowsZoomManager::get()->onResume();
		PauseLayer::onNormalMode(sender);
	}

	void onPracticeMode(CCObject* sender) {
		WindowsZoomManager::get()->onResume();
		PauseLayer::onPracticeMode(sender);
	}
};

class $modify(PlayLayer) {
	void pauseGame(bool p0) {
		WindowsZoomManager::get()->onPause();
		PlayLayer::pauseGame(p0);
	}

	void startGame() {
		WindowsZoomManager::get()->onResume();
		PlayLayer::startGame();
	}

	bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
		WindowsZoomManager::get()->onResume();
		return PlayLayer::init(level, useReplay, dontCreateObjects);
	}
};

class $modify(CCScheduler) {
	virtual void update(float dt) {
		WindowsZoomManager::get()->update(dt);
		CCScheduler::update(dt);
	}
};

#ifdef GEODE_IS_WINDOWS
$execute {
    auto* window = CCEGLView::sharedOpenGLView()->getWindow();

    using glfwSetMouseButtonCallback_t = GLFWmousebuttonfun(*)(GLFWwindow*, GLFWmousebuttonfun);
    auto setCallback = reinterpret_cast<glfwSetMouseButtonCallback_t>(
        GetProcAddress(GetModuleHandleA("glfw3.dll"), "glfwSetMouseButtonCallback")
    );

    if (setCallback) {
        static GLFWmousebuttonfun prev = nullptr;
        prev = setCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
            if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
                WindowsZoomManager::get()->isPanning = (action == GLFW_PRESS);
            }
            if (prev) prev(w, button, action, mods);
        });
    }
}
#else
void otherMouseDownHook(void* self, SEL sel, void* event) {
	WindowsZoomManager::get()->isPanning = true;
	reinterpret_cast<void(*)(void*, SEL, void*)>(objc_msgSend)(self, sel, event);
}

void otherMouseUpHook(void* self, SEL sel, void* event) {
	WindowsZoomManager::get()->isPanning = false;
	reinterpret_cast<void(*)(void*, SEL, void*)>(objc_msgSend)(self, sel, event);
}

$execute {
	if (auto hook = ObjcHook::create("EAGLView", "otherMouseDown:", &otherMouseDownHook)) {
		(void) Mod::get()->claimHook(hook.unwrap());
	}
	
	if (auto hook = ObjcHook::create("EAGLView", "otherMouseUp:", &otherMouseUpHook)) {
		(void) Mod::get()->claimHook(hook.unwrap());
	}
}
#endif // GEODE_IS_WINDOWS

class $modify(CCMouseDispatcher) {
	bool dispatchScrollMSG(float y, float x) {
		WindowsZoomManager::get()->onScroll(y, x);
		return CCMouseDispatcher::dispatchScrollMSG(y, x);
	}
};
#endif // GEODE_IS_DESKTOP