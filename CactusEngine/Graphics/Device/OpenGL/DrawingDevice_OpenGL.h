#pragma once
#include "DrawingDevice.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Engine
{
	class DrawingDevice_OpenGL : public DrawingDevice
	{
	public:
		~DrawingDevice_OpenGL();

		void Initialize() override;
		void ShutDown() override;

		std::shared_ptr<ShaderProgram> CreateShaderProgramFromFile(const char* vertexShaderFilePath, const char* fragmentShaderFilePath) override;
		std::shared_ptr<ShaderProgram> CreateShaderProgramFromFile(const char* vertexShaderFilePath, const char* fragmentShaderFilePath, EGPUType gpuType) override;

		bool CreateVertexBuffer(const VertexBufferCreateInfo& createInfo, std::shared_ptr<VertexBuffer>& pOutput) override;
		bool CreateTexture2D(const Texture2DCreateInfo& createInfo, std::shared_ptr<Texture2D>& pOutput) override;
		bool CreateFrameBuffer(const FrameBufferCreateInfo& createInfo, std::shared_ptr<FrameBuffer>& pOutput) override;	
		bool CreateUniformBuffer(const UniformBufferCreateInfo& createInfo, std::shared_ptr<UniformBuffer>& pOutput) override;

		void UpdateShaderParameter(std::shared_ptr<ShaderProgram> pShaderProgram, const std::shared_ptr<ShaderParameterTable> pTable, std::shared_ptr<DrawingCommandBuffer> pCommandBuffer = nullptr) override;
		void SetVertexBuffer(const std::shared_ptr<VertexBuffer> pVertexBuffer, std::shared_ptr<DrawingCommandBuffer> pCommandBuffer = nullptr) override;
		void DrawPrimitive(uint32_t indicesCount, uint32_t baseIndex, uint32_t baseVertex, std::shared_ptr<DrawingCommandBuffer> pCommandBuffer = nullptr) override;
		void DrawFullScreenQuad(std::shared_ptr<DrawingCommandBuffer> pCommandBuffer = nullptr) override;
		void ResizeViewPort(uint32_t width, uint32_t height) override;

		EGraphicsDeviceType GetDeviceType() const override;

		// Low-level functions that shouldn't be called on OpenGL device
		std::shared_ptr<DrawingCommandPool> RequestExternalCommandPool(EQueueType queueType, EGPUType deviceType = EGPUType::Main) override;
		std::shared_ptr<DrawingCommandBuffer> RequestCommandBuffer(std::shared_ptr<DrawingCommandPool> pCommandPool) override;
		void ReturnExternalCommandBuffer(std::shared_ptr<DrawingCommandBuffer> pCommandBuffer) override;
		std::shared_ptr<DrawingSemaphore> RequestDrawingSemaphore(EGPUType deviceType, ESemaphoreWaitStage waitStage) override;

		bool CreateDataTransferBuffer(const DataTransferBufferCreateInfo& createInfo, std::shared_ptr<DataTransferBuffer>& pOutput) override;
		bool CreateRenderPassObject(const RenderPassCreateInfo& createInfo, std::shared_ptr<RenderPassObject>& pOutput) override;
		bool CreateSampler(const TextureSamplerCreateInfo& createInfo, std::shared_ptr<TextureSampler>& pOutput) override;
		bool CreatePipelineVertexInputState(const PipelineVertexInputStateCreateInfo& createInfo, std::shared_ptr<PipelineVertexInputState>& pOutput) override;
		bool CreatePipelineInputAssemblyState(const PipelineInputAssemblyStateCreateInfo& createInfo, std::shared_ptr<PipelineInputAssemblyState>& pOutput) override;
		bool CreatePipelineColorBlendState(const PipelineColorBlendStateCreateInfo& createInfo, std::shared_ptr<PipelineColorBlendState>& pOutput) override;
		bool CreatePipelineRasterizationState(const PipelineRasterizationStateCreateInfo& createInfo, std::shared_ptr<PipelineRasterizationState>& pOutput) override;
		bool CreatePipelineDepthStencilState(const PipelineDepthStencilStateCreateInfo& createInfo, std::shared_ptr<PipelineDepthStencilState>& pOutput) override;
		bool CreatePipelineMultisampleState(const PipelineMultisampleStateCreateInfo& createInfo, std::shared_ptr<PipelineMultisampleState>& pOutput) override;
		bool CreatePipelineViewportState(const PipelineViewportStateCreateInfo& createInfo, std::shared_ptr<PipelineViewportState>& pOutput) override;
		bool CreateGraphicsPipelineObject(const GraphicsPipelineCreateInfo& createInfo, std::shared_ptr<GraphicsPipelineObject>& pOutput) override;

		void TransitionImageLayout(std::shared_ptr<Texture2D> pImage, EImageLayout newLayout, uint32_t appliedStages) override;
		void TransitionImageLayout_Immediate(std::shared_ptr<Texture2D> pImage, EImageLayout newLayout, uint32_t appliedStages) override;
		void ResizeSwapchain(uint32_t width, uint32_t height) override;

		void BindGraphicsPipeline(const std::shared_ptr<GraphicsPipelineObject> pPipeline, std::shared_ptr<DrawingCommandBuffer> pCommandBuffer) override;
		void BeginRenderPass(const std::shared_ptr<RenderPassObject> pRenderPass, const std::shared_ptr<FrameBuffer> pFrameBuffer, std::shared_ptr<DrawingCommandBuffer> pCommandBuffer) override;
		void EndRenderPass(std::shared_ptr<DrawingCommandBuffer> pCommandBuffer) override;
		void EndCommandBuffer(std::shared_ptr<DrawingCommandBuffer> pCommandBuffer) override;
		void CommandWaitSemaphore(std::shared_ptr<DrawingCommandBuffer> pCommandBuffer, std::shared_ptr<DrawingSemaphore> pSemaphore) override;
		void CommandSignalSemaphore(std::shared_ptr<DrawingCommandBuffer> pCommandBuffer, std::shared_ptr<DrawingSemaphore> pSemaphore) override;

		void Present() override;
		void FlushCommands(bool waitExecution, bool flushImplicitCommands, uint32_t deviceTypeFlags = (uint32_t)EGPUType::Main | (uint32_t)EGPUType::Secondary) override;
		void FlushTransferCommands(bool waitExecution) override;
		void WaitSemaphore(std::shared_ptr<DrawingSemaphore> pSemaphore) override;

		std::shared_ptr<TextureSampler> GetDefaultTextureSampler(EGPUType deviceType = EGPUType::Main, bool withDefaultAF = false) const override;
		void GetSwapchainImages(std::vector<std::shared_ptr<Texture2D>>& outImages) const override;
		uint32_t GetSwapchainPresentImageIndex() const override;

		void CopyTexture2DToDataTransferBuffer(std::shared_ptr<Texture2D> pSrcTexture, std::shared_ptr<DataTransferBuffer> pDstBuffer, std::shared_ptr<DrawingCommandBuffer> pCommandBuffer) override;
		void CopyDataTransferBufferToTexture2D(std::shared_ptr<DataTransferBuffer> pSrcBuffer, std::shared_ptr<Texture2D> pDstTexture, std::shared_ptr<DrawingCommandBuffer> pCommandBuffer) override;
		void CopyDataTransferBufferCrossDevice(std::shared_ptr<DataTransferBuffer> pSrcBuffer, std::shared_ptr<DataTransferBuffer> pDstBuffer) override;
		void CopyDataTransferBufferWithinDevice(std::shared_ptr<DataTransferBuffer> pSrcBuffer, std::shared_ptr<DataTransferBuffer> pDstBuffer, std::shared_ptr<DrawingCommandBuffer> pCommandBuffer) override;
		void CopyHostDataToDataTransferBuffer(void* pData, std::shared_ptr<DataTransferBuffer> pDstBuffer, size_t size) override;
		void CopyDataTransferBufferToHostDataLocation(std::shared_ptr<DataTransferBuffer> pSrcBuffer, void* pDataLoc) override;

		void SetPrimitiveTopology(GLenum mode);

	private:
		void SetRenderTarget(const std::shared_ptr<FrameBuffer> pFrameBuffer, const std::vector<uint32_t>& attachments);
		void SetRenderTarget(const std::shared_ptr<FrameBuffer> pFrameBuffer);

	private:
		GLuint m_attributeless_vao = -1;
		GLenum m_primitiveTopologyMode = GL_TRIANGLES;
	};

	template<>
	static std::shared_ptr<DrawingDevice> CreateDrawingDevice<EGraphicsDeviceType::OpenGL>()
	{
		auto pDevice = std::make_shared<DrawingDevice_OpenGL>();

		if (!gpGlobal->QueryGlobalState(EGlobalStateQueryType::GLFWInit))
		{		
			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			{
				throw std::runtime_error("OpenGL: Failed to initialize GLAD");
			}
		}

		return pDevice;
	}
}