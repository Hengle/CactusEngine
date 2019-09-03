#include "GraphicsApplication.h"

using namespace Engine;

void GraphicsApplication::Initialize()
{
	m_pECSWorld = std::make_shared<ECSWorld>();

	InitWindow(); // Alert: since we are binding the init of GLAD with GLFW, this has to be done before InitECS()

	if (m_pSetupFunc)
	{
		m_pSetupFunc(this);
	}

	InitECS();
}

void GraphicsApplication::Tick()
{
	m_pECSWorld->Tick();
	m_pWindow->Tick();
}

void GraphicsApplication::ShutDown()
{
	m_pECSWorld->ShutDown();
	m_pWindow->ShutDown();
}

bool GraphicsApplication::ShouldQuit() const
{
	return (m_shouldQuit || m_pWindow->ShouldQuit());
}

std::shared_ptr<ECSWorld> GraphicsApplication::GetECSWorld() const
{
	return m_pECSWorld;
}

std::shared_ptr<DrawingDevice> GraphicsApplication::GetDrawingDevice() const
{
	return m_pDevice;
}

void GraphicsApplication::SetDrawingDevice(const std::shared_ptr<DrawingDevice> pDevice)
{
	m_pDevice = pDevice;
}

void GraphicsApplication::AddSetupFunction(void(*pSetupFunc)(GraphicsApplication* pApp))
{
	m_pSetupFunc = pSetupFunc;
}

void GraphicsApplication::InitWindow()
{
	m_pWindow = std::make_shared<GLFWWindow>(
		gpGlobal->GetConfiguration<AppConfiguration>(eConfiguration_App)->GetAppName(),
		gpGlobal->GetConfiguration<GraphicsConfiguration>(eConfiguration_Graphics)->GetWindowWidth(),
		gpGlobal->GetConfiguration<GraphicsConfiguration>(eConfiguration_Graphics)->GetWindowHeight()
		);

	if (!m_pWindow)
	{
		throw std::runtime_error("Failed to create GFLW window.");
	}
	m_pWindow->Initialize();
}

void GraphicsApplication::InitECS()
{
	m_pECSWorld->Initialize();
}