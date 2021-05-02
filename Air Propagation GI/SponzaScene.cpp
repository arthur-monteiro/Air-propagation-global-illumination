#include "SponzaScene.h"

using namespace Wolf;

SponzaScene::SponzaScene(Wolf::WolfInstance* wolfInstance)
{
	m_window = wolfInstance->getWindowPtr();

	// Scene creation
	Scene::SceneCreateInfo sceneCreateInfo;
	sceneCreateInfo.swapChainCommandType = Scene::CommandType::COMPUTE;

	m_scene = wolfInstance->createScene(sceneCreateInfo);

	// Model creation
	Model::ModelCreateInfo modelCreateInfo{};
	modelCreateInfo.inputVertexTemplate = InputVertexTemplate::FULL_3D_MATERIAL;
	Model* model = wolfInstance->createModel<>(modelCreateInfo);

	Model::ModelLoadingInfo modelLoadingInfo;
	modelLoadingInfo.filename = std::move("Models/sponza/sponza.obj");
	modelLoadingInfo.mtlFolder = std::move("Models/sponza");
	model->loadObj(modelLoadingInfo);

	// Data
	float near = 0.1f;
	float far = 100.0f;
	{
		m_projectionMatrix = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, near, far);
		m_projectionMatrix[1][1] *= -1;
		m_viewMatrix = glm::lookAt(glm::vec3(-2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		m_modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));
	}

	{
		// GBuffer
		Scene::CommandBufferCreateInfo commandBufferCreateInfo;
		commandBufferCreateInfo.finalPipelineStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		commandBufferCreateInfo.commandType = Scene::CommandType::GRAPHICS;
		m_gBufferCommandBufferID = m_scene->addCommandBuffer(commandBufferCreateInfo);

		m_GBuffer = std::make_unique<GBuffer>(wolfInstance, m_scene, m_gBufferCommandBufferID, wolfInstance->getWindowSize(), VK_SAMPLE_COUNT_1_BIT, model, glm::mat4(1.0f), true);

		Image* depth = m_GBuffer->getDepth();
		Image* albedo = m_GBuffer->getAlbedo();
		Image* normalRoughnessMetal = m_GBuffer->getNormalRoughnessMetal();

		// CSM
		m_cascadedShadowMapping = std::make_unique<CascadedShadowMapping>(wolfInstance, m_scene, model, 0.1f, 100.0f, 32.f, glm::radians(45.0f), wolfInstance->getWindowSize(),
			depth, m_projectionMatrix);

		// Radiosity probes
		std::array<std::vector<float>, 2> probesIntensityPingPong;
		probesIntensityPingPong[0].resize(1000);
		probesIntensityPingPong[1].resize(1000);

		bool useDataInFileShadow = true;
		if (!useDataInFileShadow)
		{
			for (int i(0); i < 1000; ++i)
			{
				glm::vec3 probePos = glm::vec3(20.0f - ((int)i / 100) * 4.0f, 0.1f + (((int)i / 10) % 10) * 1.7f, -10.0f + (i % 10) * 2.5f);
				probePos /= 0.01f;

				if (!model->checkIntersection(probePos, probePos - (30.0f * m_lightDir) / 0.01f))
					probesIntensityPingPong[0][i] = 1.0f;
				else
					probesIntensityPingPong[0][i] = 0.0f;
			}

			std::ofstream probeDirectLightFile;
			probeDirectLightFile.open("Data/probesDirectLight.dat");
			for (int i(0); i < 1000; ++i)
			{
				probeDirectLightFile << (probesIntensityPingPong[0][i] == 1.0f) << "\n";
			}
			probeDirectLightFile.close();
		}
		else
		{
			std::ifstream probeDirectLightFile("Data/probesDirectLight.dat");
			if (probeDirectLightFile.is_open())
			{
				std::string line; int i(0);
				while (std::getline(probeDirectLightFile, line))
				{
					probesIntensityPingPong[0][i] = line[0] == '1' ? 1.0f : 0.0f;
					i++;
				}
			}
		}

		bool useDataInFile = true;
		struct ProbeCollision
		{
			bool pX;
			bool lX;

			bool pY;
			bool lY;

			bool pZ;
			bool lZ;
		};
		std::vector<ProbeCollision> probeCollisions(1000);
		if (!useDataInFile)
		{
			for (int i(0); i < 1000; ++i)
			{
				if (i % 20 == 0)
					std::cout << i / 20 << " %\n";

				int probeI = (int)i / 100, probeJ = ((int)i / 10) % 10, probeK = i % 10;
				glm::vec3 probePos = glm::vec3(20.0f - probeI * 4.0f, 0.1f + probeJ * 1.7f, -10.0f + probeK * 2.5f);
				probePos /= 0.01f;

				if (probeI == 0)
					probeCollisions[i].lX = true;
				else
					probeCollisions[i].lX = model->checkIntersection(probePos, probePos + glm::vec3(4.0f / 0.01f, 0.0f, 0.0f));
				if (probeI == 9)
					probeCollisions[i].pX = true;
				else
					probeCollisions[i].pX = model->checkIntersection(probePos, probePos - glm::vec3(4.0f / 0.01f, 0.0f, 0.0f));

				if (probeJ == 0)
					probeCollisions[i].lY = true;
				else
					probeCollisions[i].lY = model->checkIntersection(probePos, probePos - glm::vec3(0.0f, 1.7f / 0.01f, 0.0f));
				if (probeJ == 9)
					probeCollisions[i].pY = true;
				else
					probeCollisions[i].pY = model->checkIntersection(probePos, probePos + glm::vec3(0.0f, 1.7f / 0.01f, 0.0f));

				if (probeK == 0)
					probeCollisions[i].lZ = true;
				else
					probeCollisions[i].lZ = model->checkIntersection(probePos, probePos - glm::vec3(0.0f, 0.0f, 2.5f / 0.01f));
				if (probeK == 9)
					probeCollisions[i].pZ = true;
				else
					probeCollisions[i].pZ = model->checkIntersection(probePos, probePos + glm::vec3(0.0f, 0.0f, 2.5f / 0.01f));
			}

			std::ofstream probeCollisionsFile;
			probeCollisionsFile.open("Data/probesCollision.dat");
			for (int i(0); i < 1000; ++i)
			{
				probeCollisionsFile << probeCollisions[i].lX << "," << probeCollisions[i].pX << "," << probeCollisions[i].lY << "," <<
					probeCollisions[i].pY << "," << probeCollisions[i].lZ << "," << probeCollisions[i].pZ << "\n";
			}
			probeCollisionsFile.close();
		}
		else
		{
			std::ifstream probeCollisionsFile("Data/probesCollision.dat");
			if (probeCollisionsFile.is_open())
			{
				std::string line; int i(0);
				while (std::getline(probeCollisionsFile, line))
				{
					int valueNum = 0;
					for (int c(0); c < line.size(); ++c)
					{
						if (line[c] == ',')
							valueNum++;
						else
						{
							if (valueNum == 0)
								probeCollisions[i].lX = line[c] == '1';
							else if (valueNum == 1)
								probeCollisions[i].pX = line[c] == '1';
							else if (valueNum == 2)
								probeCollisions[i].lY = line[c] == '1';
							else if (valueNum == 3)
								probeCollisions[i].pY = line[c] == '1';
							else if (valueNum == 4)
								probeCollisions[i].lZ = line[c] == '1';
							else if (valueNum == 5)
								probeCollisions[i].pZ = line[c] == '1';
						}
					}

					i++;
				}
				probeCollisionsFile.close();
			}
		}

		float probeIntensityCoeffs[] = { 0.25f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f };
		for (int probePassCalculation(0); probePassCalculation < 10; ++probePassCalculation)
		{
			for (int i(0); i < 1000; ++i)
			{
				if (i % 20 == 0)
					std::cout << 50 + i / 20 << " %\n";

				probesIntensityPingPong[(probePassCalculation + 1) % 2][i] = probesIntensityPingPong[probePassCalculation % 2][i];

				if (probesIntensityPingPong[probePassCalculation % 2][i] > 0.0f)
					continue;

				int probeI = (int)i / 100, probeJ = ((int)i / 10) % 10, probeK = i % 10;
				glm::vec3 probePos = glm::vec3(20.0f - probeI * 4.0f, 0.1f + probeJ * 1.7f, -10.0f + probeK * 2.5f);

				// X
				if (!probeCollisions[i].lX)
					probesIntensityPingPong[(probePassCalculation + 1) % 2][i] += probesIntensityPingPong[probePassCalculation % 2][100 * (probeI - 1) + 10 * probeJ + probeK] * probeIntensityCoeffs[probePassCalculation];
				if (!probeCollisions[i].pX)
					probesIntensityPingPong[(probePassCalculation + 1) % 2][i] += probesIntensityPingPong[probePassCalculation % 2][100 * (probeI + 1) + 10 * probeJ + probeK] * probeIntensityCoeffs[probePassCalculation];

				// Y
				if (!probeCollisions[i].lY)
					probesIntensityPingPong[(probePassCalculation + 1) % 2][i] += probesIntensityPingPong[probePassCalculation % 2][100 * probeI + 10 * (probeJ - 1) + probeK] * probeIntensityCoeffs[probePassCalculation];
				if (!probeCollisions[i].pY)
					probesIntensityPingPong[(probePassCalculation + 1) % 2][i] += probesIntensityPingPong[probePassCalculation % 2][100 * probeI + 10 * (probeJ + 1) + probeK] * probeIntensityCoeffs[probePassCalculation];

				// Z
				if (!probeCollisions[i].lZ)
					probesIntensityPingPong[(probePassCalculation + 1) % 2][i] += probesIntensityPingPong[probePassCalculation % 2][100 * probeI + 10 * probeJ + probeK - 1] * probeIntensityCoeffs[probePassCalculation];
				if (!probeCollisions[i].pZ)
					probesIntensityPingPong[(probePassCalculation + 1) % 2][i] += probesIntensityPingPong[probePassCalculation % 2][100 * probeI + 10 * probeJ + probeK + 1] * probeIntensityCoeffs[probePassCalculation];
			}
		}

		m_ubRadiosityProbesData.resize(1000);
		for (int i(0); i < 1000; ++i)
			m_ubRadiosityProbesData[i].x = probesIntensityPingPong[1][i];
		m_uboRadiosityProbes = wolfInstance->createUniformBufferObject(m_ubRadiosityProbesData.data(), m_ubRadiosityProbesData.size() * sizeof(glm::vec4));

		// Direct lighting
#pragma warning(disable: 4996)
		m_directLightingOutputTexture = wolfInstance->createTexture();
		m_directLightingOutputTexture->create({ wolfInstance->getWindowSize().width, wolfInstance->getWindowSize().height, 1 }, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
		m_directLightingOutputTexture->setImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		m_directLightingUBData.directionDirectionalLight = glm::vec4(m_lightDir, 1.0f);
		m_directLightingUBData.colorDirectionalLight = glm::vec4(10.0f, 9.0f, 6.0f, 1.0f);
		m_directLightingUBData.invProjection = glm::inverse(m_projectionMatrix);
		m_directLightingUBData.invView = glm::inverse(m_viewMatrix);
		m_directLightingUBData.projParams.x = far / (far - near);
		m_directLightingUBData.projParams.y = (-far * near) / (far - near);
		m_directLightingUniformBuffer = wolfInstance->createUniformBufferObject(&m_directLightingUBData, sizeof(m_directLightingUBData));

		Scene::ComputePassCreateInfo directLigtingComputePassCreateInfo;
		directLigtingComputePassCreateInfo.extent = wolfInstance->getWindowSize();
		directLigtingComputePassCreateInfo.dispatchGroups = { 16, 16, 1 };
		directLigtingComputePassCreateInfo.computeShaderPath = "Shaders/directLighting/comp.spv";

		commandBufferCreateInfo.finalPipelineStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		commandBufferCreateInfo.commandType = Scene::CommandType::COMPUTE;
		m_directLightingCommandBufferID = m_scene->addCommandBuffer(commandBufferCreateInfo);
		directLigtingComputePassCreateInfo.commandBufferID = m_directLightingCommandBufferID;

		{
			DescriptorSetGenerator descriptorSetGenerator;
			descriptorSetGenerator.addImages({ depth }, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0);
			descriptorSetGenerator.addImages({ albedo }, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1);
			descriptorSetGenerator.addImages({ normalRoughnessMetal }, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 2);
			descriptorSetGenerator.addImages({ m_cascadedShadowMapping->getOutputShadowMaskTexture()->getImage() }, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 3);
			descriptorSetGenerator.addImages({ m_cascadedShadowMapping->getOutputVolumetricLightMaskTexture()->getImage() }, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 4);
			descriptorSetGenerator.addImages({ m_reflectionQuantityImage }, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 5);
			descriptorSetGenerator.addImages({ m_directLightingOutputTexture->getImage() }, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 6);
			descriptorSetGenerator.addUniformBuffer(m_directLightingUniformBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 7);
			descriptorSetGenerator.addUniformBuffer(m_uboRadiosityProbes, VK_SHADER_STAGE_COMPUTE_BIT, 8);

			directLigtingComputePassCreateInfo.descriptorSetCreateInfo = descriptorSetGenerator.getDescritorSetCreateInfo();
		}

		m_scene->addComputePass(directLigtingComputePassCreateInfo);

		// Tone mapping
		Scene::ComputePassCreateInfo toneMappingComputePassCreateInfo;
		toneMappingComputePassCreateInfo.computeShaderPath = "Shaders/toneMapping/comp.spv";
		toneMappingComputePassCreateInfo.outputIsSwapChain = true;
		toneMappingComputePassCreateInfo.commandBufferID = -1;
		toneMappingComputePassCreateInfo.dispatchGroups = { 16, 16, 1 };

		DescriptorSetGenerator toneMappingDescriptorSetGenerator;
		toneMappingDescriptorSetGenerator.addImages({ m_directLightingOutputTexture->getImage() }, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0);

		toneMappingComputePassCreateInfo.descriptorSetCreateInfo = toneMappingDescriptorSetGenerator.getDescritorSetCreateInfo();
		toneMappingComputePassCreateInfo.outputBinding = 1;

		m_scene->addComputePass(toneMappingComputePassCreateInfo);
	}

	m_scene->record();

	m_camera.initialize(glm::vec3(1.4f, 1.2f, 0.3f), glm::vec3(2.0f, 0.9f, -0.3f), glm::vec3(0.0f, 1.0f, 0.0f), 0.01f, 5.0f,
		16.0f / 9.0f);
}

void SponzaScene::update()
{
	m_camera.update(m_window);
	m_viewMatrix = m_camera.getViewMatrix();

	m_GBuffer->updateMVPMatrix(m_modelMatrix, m_viewMatrix, m_projectionMatrix);

	m_cascadedShadowMapping->updateMatrices(m_lightDir, m_camera.getPosition(), m_camera.getOrientation(), m_modelMatrix, glm::inverse(m_viewMatrix * m_modelMatrix));

	m_directLightingUBData.invView = glm::inverse(m_viewMatrix);
	m_directLightingUBData.directionDirectionalLight = glm::transpose(m_directLightingUBData.invView) * glm::vec4(m_lightDir, 1.0f);
	m_directLightingUniformBuffer->updateData(&m_directLightingUBData);
}

std::vector<int> SponzaScene::getCommandBufferToSubmit()
{
	std::vector<int> r;
	r.push_back(m_gBufferCommandBufferID);
	std::vector<int> csmCommandBuffer = m_cascadedShadowMapping->getCascadeCommandBuffers();
	for (auto& commandBuffer : csmCommandBuffer)
		r.push_back(commandBuffer);
	r.push_back(m_directLightingCommandBufferID);

	return r;
}

std::vector<std::pair<int, int>> SponzaScene::getCommandBufferSynchronisation()
{
	std::vector<std::pair<int, int>> r =
	{ { m_gBufferCommandBufferID, m_directLightingCommandBufferID}, { m_directLightingCommandBufferID, -1 } };

	std::vector<std::pair<int, int>> csmSynchronisation = m_cascadedShadowMapping->getCommandBufferSynchronisation();
	for (auto& sync : csmSynchronisation)
	{
		r.push_back(sync);
	}
	r.emplace_back(m_cascadedShadowMapping->getCascadeCommandBuffers().back(), m_directLightingCommandBufferID);

	return r;
}
