#pragma once
#include "ISystem.h"
#include "Global.h"
#include "ECSWorld.h"
#include "DrawingDevice.h"
#include <chrono>

namespace Engine
{
	class AnimationSystem : public ISystem
	{
	public:
		AnimationSystem(ECSWorld* pWorld);
		~AnimationSystem() = default;

		void SetSystemID(uint32_t id);
		uint32_t GetSystemID() const;

		void Initialize();
		void ShutDown();

		void FrameBegin();
		void Tick();
		void FrameEnd();

	private:
		uint32_t m_systemID;
		ECSWorld* m_pECSWorld;
	};
}