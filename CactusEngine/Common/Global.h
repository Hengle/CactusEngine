#pragma once
#include "Configuration.h"
#include <memory>
#include <stdexcept>
#include <vector>
#include <assert.h>

namespace Engine
{
	class BaseApplication;
	class Global
	{
	public:
		Global();
		~Global() = default;

		void SetApplication(const std::shared_ptr<BaseApplication> pApp);
		std::shared_ptr<BaseApplication> GetCurrentApplication() const;

		template<typename T>
		inline void CreateConfiguration(EConfigurationType configType)
		{
			assert(configType < m_configurations.size());
			m_configurations[configType] = std::make_shared<T>();
		}

		template<typename T>
		inline std::shared_ptr<T> GetConfiguration(EConfigurationType configType) const
		{
			assert(configType < m_configurations.size());
			return std::dynamic_pointer_cast<T>(m_configurations[configType]);
		}

		bool QueryGlobalState(EGlobalStateQueryType type) const;
		void MarkGlobalState(EGlobalStateQueryType type, bool val); // Alert: this does not "change" global state, that's why it's "Mark.."

	private:
		std::shared_ptr<BaseApplication> m_pCurrentApp;
		std::vector<std::shared_ptr<BaseConfiguration>> m_configurations;

		std::vector<bool> m_globalStates;
	};

	extern Global* gpGlobal;
}