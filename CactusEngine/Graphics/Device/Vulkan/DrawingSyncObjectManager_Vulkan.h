#pragma once
#include <vulkan.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace Engine
{
	class DrawingSemaphore_Vulkan
	{
	public:
		DrawingSemaphore_Vulkan(const VkSemaphore& semaphoreHandle, uint32_t assignedID);

	public:
		VkSemaphore semaphore;

	private:
		uint32_t id;
		friend class DrawingSyncObjectManager_Vulkan;
	};

	class DrawingFence_Vulkan
	{
	public:
		DrawingFence_Vulkan(const VkFence& fenceHandle, uint32_t assignedID);

	public:
		VkFence fence;

	private:
		uint32_t id;
		friend class DrawingSyncObjectManager_Vulkan;
	};

	class DrawingDevice_Vulkan;
	class DrawingSyncObjectManager_Vulkan
	{
	public:
		DrawingSyncObjectManager_Vulkan(const std::shared_ptr<DrawingDevice_Vulkan> pDevice);

		std::shared_ptr<DrawingSemaphore_Vulkan> RequestSemaphore();
		std::shared_ptr<DrawingFence_Vulkan> RequestFence();

		void ReturnSemaphore(std::shared_ptr<DrawingSemaphore_Vulkan> pSemaphore);
		void ReturnFence(std::shared_ptr<DrawingFence_Vulkan> pFence);

	private:
		bool CreateNewSemaphore(uint32_t count);
		bool CreateNewFence(uint32_t count, bool signaled = false);

	public:
		const uint32_t MAX_SEMAPHORE_COUNT = 128;
		const uint32_t MAX_FENCE_COUNT = 64;

	private:
		std::shared_ptr<DrawingDevice_Vulkan> m_pDevice;
		mutable std::mutex m_mutex;

		std::vector<std::shared_ptr<DrawingSemaphore_Vulkan>> m_semaphorePool;
		std::vector<std::shared_ptr<DrawingFence_Vulkan>> m_fencePool;

		std::unordered_map<uint32_t, bool> m_semaphoreAvailability;
		std::unordered_map<uint32_t, bool> m_fenceAvailability;
	};
}