#include "SystemManager.h"

using namespace Wolf;

void SystemManager::run()
{
	createWolfInstance();

	m_loadingScene = std::make_unique<LoadingScene>(m_wolfInstance.get());
	m_sceneLoadingThread = std::thread(&SystemManager::loadSponzaScene, this);
	
	while (!m_wolfInstance->windowShouldClose() /* check if the window should close (for example if the user pressed alt+f4)*/)
	{
		if (m_needJoinLoadingThread)
		{
			m_sceneLoadingThread.join();
			m_needJoinLoadingThread = false;
		}
		
		if (m_gameState == GAME_STATE::LOADING)
		{
			//graphicsQueueMutex->lock();
			m_loadingScene->update();
			m_wolfInstance->frame(m_loadingScene->getScene(), {}, {});
			//graphicsQueueMutex->unlock();
		}
		else if (m_gameState == GAME_STATE::RUNNING)
		{
			m_sponzaScene->update();
			m_wolfInstance->frame(m_sponzaScene->getScene(), m_sponzaScene->getCommandBufferToSubmit(), m_sponzaScene->getCommandBufferSynchronisation());
		}
	}

	m_wolfInstance->waitIdle();
}

void SystemManager::createWolfInstance()
{
	WolfInstanceCreateInfo instanceCreateInfo;

	// Application
	instanceCreateInfo.applicationName = "Sponza Example";
	instanceCreateInfo.majorVersion = 1;
	instanceCreateInfo.minorVersion = 0;

	// Window
	instanceCreateInfo.windowHeight = 720;
	instanceCreateInfo.windowWidth = 1280;

	// Debug
	instanceCreateInfo.debugCallback = debugCallback;

	instanceCreateInfo.useOVR = false;

	m_wolfInstance = std::make_unique<WolfInstance>(instanceCreateInfo);
}

void SystemManager::loadSponzaScene()
{
	m_sponzaScene = std::make_unique<SponzaScene>(m_wolfInstance.get());
	m_gameState = GAME_STATE::RUNNING;
	m_needJoinLoadingThread = true;
}

void SystemManager::debugCallback(Wolf::Debug::Severity severity, std::string message)
{
	switch (severity)
	{
	case Debug::Severity::ERROR:
		std::cout << "Error : ";
	case Debug::Severity::WARNING:
		std::cout << "Warning : ";
	case Debug::Severity::INFO:
		std::cout << "Info : ";
	}

	std::cout << message << std::endl;
}
