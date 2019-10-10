#include "ForwardRenderer.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MaterialComponent.h"
#include "CameraComponent.h"
#include "DrawingSystem.h"
#include "Timer.h"

using namespace Engine;

ForwardRenderer::ForwardRenderer(const std::shared_ptr<DrawingDevice> pDevice, DrawingSystem* pSystem)
	: BaseRenderer(eRenderer_Forward, pDevice, pSystem)
{
}

void ForwardRenderer::BuildRenderGraph()
{
	m_pRenderGraph = std::make_shared<RenderGraph>();

	RenderGraphResource opaquePassInput;
	RenderGraphResource opaquePassOutput;

	auto pOpaquePass = std::make_shared<RenderNode>(m_pRenderGraph, 
		[](const RenderGraphResource& input, RenderGraphResource& output, const std::shared_ptr<RenderContext> pContext)
	{
		auto pCameraTransform = std::static_pointer_cast<TransformComponent>(pContext->pCamera->GetComponent(eCompType_Transform));
		auto pCameraComp = std::static_pointer_cast<CameraComponent>(pContext->pCamera->GetComponent(eCompType_Camera));
		if (!pCameraComp || !pCameraTransform)
		{
			return;
		}
		Vector3 cameraPos = pCameraTransform->GetPosition();
		Matrix4x4 viewMat = glm::lookAt(cameraPos, cameraPos + pCameraTransform->GetForwardDirection(), UP);
		Matrix4x4 projectionMat = glm::perspective(pCameraComp->GetFOV(),
			float(gpGlobal->GetConfiguration<GraphicsConfiguration>(eConfiguration_Graphics)->GetWindowWidth()) / float(gpGlobal->GetConfiguration<GraphicsConfiguration>(eConfiguration_Graphics)->GetWindowHeight()),
			pCameraComp->GetNearClip(), pCameraComp->GetFarClip());

		for (auto& entity : *pContext->pDrawList)
		{
			auto pTransformComp = std::static_pointer_cast<TransformComponent>(entity->GetComponent(eCompType_Transform));
			auto pMeshFilterComp = std::static_pointer_cast<MeshFilterComponent>(entity->GetComponent(eCompType_MeshFilter));
			auto pMaterialComp = std::static_pointer_cast<MaterialComponent>(entity->GetComponent(eCompType_Material));

			if (!pTransformComp || !pMeshFilterComp || !pMaterialComp)
			{
				continue;
			}

			auto pMesh = pMeshFilterComp->GetMesh();

			if (!pMesh)
			{
				continue;
			}

			Matrix4x4 modelMat = pTransformComp->GetModelMatrix();

			auto pShaderProgram = (pContext->pRenderer->GetDrawingSystem())->GetShaderProgramByType(pMaterialComp->GetShaderProgramType());
			auto pShaderParamTable = std::make_shared<ShaderParameterTable>();

			float currTime = Timer::Now();

			pShaderParamTable->AddEntry(pShaderProgram->GetParamLocation(ShaderParamNames::MODEL_MATRIX), eShaderParam_Mat4, glm::value_ptr(modelMat));
			pShaderParamTable->AddEntry(pShaderProgram->GetParamLocation(ShaderParamNames::VIEW_MATRIX), eShaderParam_Mat4, glm::value_ptr(viewMat));
			pShaderParamTable->AddEntry(pShaderProgram->GetParamLocation(ShaderParamNames::PROJECTION_MATRIX), eShaderParam_Mat4, glm::value_ptr(projectionMat));
			pShaderParamTable->AddEntry(pShaderProgram->GetParamLocation(ShaderParamNames::CAMERA_POSITION), eShaderParam_Vec3, glm::value_ptr(cameraPos));
			pShaderParamTable->AddEntry(pShaderProgram->GetParamLocation(ShaderParamNames::TIME), eShaderParam_Float1, &currTime);
			pShaderParamTable->AddEntry(pShaderProgram->GetParamLocation(ShaderParamNames::ALBEDO_COLOR), eShaderParam_Vec4, glm::value_ptr(pMaterialComp->GetAlbedoColor()));

			auto pAlbedoTexture = pMaterialComp->GetAlbedoTexture();
			if (pAlbedoTexture)
			{
				// Alert: this only works for OpenGL; Vulkan requires doing this through descriptor sets
				uint32_t albedoTexID = pAlbedoTexture->GetTextureID();
				pShaderParamTable->AddEntry(pShaderProgram->GetParamLocation(ShaderParamNames::ALBEDO_TEXTURE), eShaderParam_Texture2D, &albedoTexID);
			}

			auto pDevice = pContext->pRenderer->GetDrawingDevice();
			pDevice->UpdateShaderParameter(pShaderProgram, pShaderParamTable);
			pDevice->SetVertexBuffer(pMesh->GetVertexBuffer());
			pDevice->DrawPrimitive(pMesh->GetVertexBuffer()->GetNumberOfIndices());

			pShaderProgram->Reset();
		}
	},
		opaquePassInput,
		opaquePassOutput);

	m_pRenderGraph->AddRenderNode(pOpaquePass);
}

void ForwardRenderer::Draw(const std::vector<std::shared_ptr<IEntity>>& drawList, const std::shared_ptr<IEntity> pCamera)
{
	if (!pCamera)
	{
		return;
	}

	auto pContext = std::make_shared<RenderContext>();
	pContext->pCamera = pCamera;
	pContext->pDrawList = &drawList;
	pContext->pRenderer = this;

	m_pRenderGraph->BeginRenderPasses(pContext);
}