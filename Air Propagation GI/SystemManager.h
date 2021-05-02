#pragma once

#include "LoadingScene.h"
#include "SponzaScene.h"

enum class GAME_STATE
{
	LOADING,
	RUNNING
};

class SystemManager
{
public:
	void run();

private:
	void createWolfInstance();
	void loadSponzaScene();
	
	static void debugCallback(Wolf::Debug::Severity severity, std::string message);

private:
	std::unique_ptr<Wolf::WolfInstance> m_wolfInstance;

	std::unique_ptr<LoadingScene> m_loadingScene;
	std::unique_ptr<SponzaScene> m_sponzaScene;

	GAME_STATE m_gameState = GAME_STATE::LOADING;
	std::thread m_sceneLoadingThread;
	bool m_needJoinLoadingThread = false;
};

