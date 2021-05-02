#pragma once

#include <WolfEngine.h>

class LoadingScene
{
public:
	LoadingScene(Wolf::WolfInstance* wolfInstance);
	
	void update() const;
	Wolf::Scene* getScene() const { return m_scene; }

private:
	Wolf::WolfInstance* m_wolfInstance = nullptr;

	Wolf::Scene* m_scene = nullptr;
	int m_renderPassID = -1;
	int m_fullScreenImageRendererID = -1;
	int m_loadingIconRendererID = -1;
	Wolf::UniformBuffer* m_iconUniformBuffer;
	std::chrono::steady_clock::time_point startTimer = std::chrono::steady_clock::now();
};