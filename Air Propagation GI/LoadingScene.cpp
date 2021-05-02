#include "LoadingScene.h"

using namespace Wolf;

LoadingScene::LoadingScene(Wolf::WolfInstance* wolfInstance) : m_wolfInstance(wolfInstance)
{
	// Scene creation
	Scene::SceneCreateInfo sceneCreateInfo;
	sceneCreateInfo.swapChainCommandType = Scene::CommandType::GRAPHICS;

	m_scene = m_wolfInstance->createScene(sceneCreateInfo);

	// Render Pass Creation
	Scene::RenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.commandBufferID = -1; // default command buffer
	renderPassCreateInfo.outputIsSwapChain = true;
	m_renderPassID = m_scene->addRenderPass(renderPassCreateInfo);

	// Loading Image
	std::vector<Vertex2DTextured> vertices =
	{
		{ glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 0.0f) }, // top left
		{ glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 1.0f) }, // bot left
		{ glm::vec2(1.0f, -1.0f), glm::vec2(1.0f, 0.0f) }, // top right
		{ glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f) } // bot right
	};

	std::vector<uint32_t> indices =
	{
		0, 1, 2,
		1, 3, 2
	};

	Model::ModelCreateInfo modelCreateInfo{};
	modelCreateInfo.inputVertexTemplate = InputVertexTemplate::POSITION_TEXTURECOORD_2D;
	Model* model = m_wolfInstance->createModel(modelCreateInfo);

	model->addMeshFromVertices(vertices.data(), 4, sizeof(Vertex2DTextured), indices); // the resources are uploaded to the GPU here

	// Renderers
	{
		// Data
		Image* texture = wolfInstance->createImageFromFile("Textures/loadingScreen.jpg");
		Sampler* sampler = wolfInstance->createSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);
		
		RendererCreateInfo rendererCreateInfo;

		ShaderCreateInfo vertexShaderCreateInfo{};
		vertexShaderCreateInfo.filename = "Shaders/loading/fullScreenLoadingImageVert.spv";
		vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		rendererCreateInfo.pipelineCreateInfo.shaderCreateInfos.push_back(vertexShaderCreateInfo);

		ShaderCreateInfo fragmentShaderCreateInfo{};
		fragmentShaderCreateInfo.filename = "Shaders/loading/fullScreenLoadingImageFrag.spv";
		fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		rendererCreateInfo.pipelineCreateInfo.shaderCreateInfos.push_back(fragmentShaderCreateInfo);
		
		rendererCreateInfo.inputVerticesTemplate = InputVertexTemplate::POSITION_TEXTURECOORD_2D;
		rendererCreateInfo.instanceTemplate = InstanceTemplate::NO;
		rendererCreateInfo.renderPassID = m_renderPassID;

		rendererCreateInfo.pipelineCreateInfo.alphaBlending = { true };

		DescriptorSetGenerator descriptorSetGenerator;
		descriptorSetGenerator.addCombinedImageSampler(texture, sampler, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

		rendererCreateInfo.descriptorLayouts = descriptorSetGenerator.getDescriptorLayouts();

		m_fullScreenImageRendererID = m_scene->addRenderer(rendererCreateInfo);

		// Link the model to the renderer
		Renderer::AddMeshInfo addMeshInfo{};
		addMeshInfo.vertexBuffer = model->getVertexBuffers()[0];
		addMeshInfo.renderPassID = m_renderPassID;
		addMeshInfo.rendererID = m_fullScreenImageRendererID;

		addMeshInfo.descriptorSetCreateInfo = descriptorSetGenerator.getDescritorSetCreateInfo();

		m_scene->addMesh(addMeshInfo);
	}
	{
		// Data
		glm::mat4 transform = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.8f, 0.75f, 0.0f)), glm::vec3(0.2f, 0.2f, 1.0f));
		m_iconUniformBuffer = wolfInstance->createUniformBufferObject(&transform, sizeof(glm::mat4));

		Image* texture = wolfInstance->createImageFromFile("Textures/loadingIcon.png");
		Sampler* sampler = wolfInstance->createSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 1.0f, VK_FILTER_LINEAR);

		// Renderer
		RendererCreateInfo rendererCreateInfo;

		ShaderCreateInfo vertexShaderCreateInfo{};
		vertexShaderCreateInfo.filename = "Shaders/loading/loadingIconVert.spv";
		vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		rendererCreateInfo.pipelineCreateInfo.shaderCreateInfos.push_back(vertexShaderCreateInfo);

		ShaderCreateInfo fragmentShaderCreateInfo{};
		fragmentShaderCreateInfo.filename = "Shaders/loading/loadingIconFrag.spv";
		fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		rendererCreateInfo.pipelineCreateInfo.shaderCreateInfos.push_back(fragmentShaderCreateInfo);

		rendererCreateInfo.pipelineCreateInfo.alphaBlending = { true };
		
		rendererCreateInfo.inputVerticesTemplate = InputVertexTemplate::POSITION_TEXTURECOORD_2D;
		rendererCreateInfo.instanceTemplate = InstanceTemplate::NO;
		rendererCreateInfo.renderPassID = m_renderPassID;

		DescriptorSetGenerator descriptorSetGenerator;
		descriptorSetGenerator.addUniformBuffer(m_iconUniformBuffer, VK_SHADER_STAGE_VERTEX_BIT, 0);
		descriptorSetGenerator.addCombinedImageSampler(texture, sampler, VK_SHADER_STAGE_FRAGMENT_BIT, 1);

		rendererCreateInfo.descriptorLayouts = descriptorSetGenerator.getDescriptorLayouts();

		m_loadingIconRendererID = m_scene->addRenderer(rendererCreateInfo);

		// Link the model to the renderer
		Renderer::AddMeshInfo addMeshInfo{};
		addMeshInfo.vertexBuffer = model->getVertexBuffers()[0];
		addMeshInfo.renderPassID = m_renderPassID;
		addMeshInfo.rendererID = m_loadingIconRendererID;

		addMeshInfo.descriptorSetCreateInfo = descriptorSetGenerator.getDescritorSetCreateInfo();

		m_scene->addMesh(addMeshInfo);
	}

	// Record
	m_scene->record();
}

void LoadingScene::update() const
{
	std::chrono::steady_clock::time_point currentTimer = std::chrono::steady_clock::now();
	float timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(currentTimer - startTimer).count() / 1'000.0f;
	
	glm::mat4 transform = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.8f, 0.75f, 0.0f)), timeDiff * 2.0f, glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(0.15f, 0.15f, 1.0f));
	m_iconUniformBuffer->updateData(&transform);
}
