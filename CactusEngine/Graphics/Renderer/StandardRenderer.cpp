#include "StandardRenderer.h"
#include "DrawingSystem.h"
#include "AllComponents.h"
#include "Timer.h"
#include "AllRenderNodes.h"

#include <iostream>

using namespace Engine;

StandardRenderer::StandardRenderer(const std::shared_ptr<DrawingDevice> pDevice, DrawingSystem* pSystem)
	: BaseRenderer(ERendererType::Standard, pDevice, pSystem), m_newCommandRecorded(false)
{
	m_pGraphResources = std::make_shared<RenderGraphResource>();
}

void StandardRenderer::BuildRenderGraph()
{
	m_pRenderGraph = std::make_shared<RenderGraph>(m_pDevice, 4);

	// Create required nodes

	auto pShadowMapNode = std::make_shared<ShadowMapRenderNode>(m_pGraphResources, this);
	auto pGBufferNode = std::make_shared<GBufferRenderNode>(m_pGraphResources, this);
	auto pOpaqueNode = std::make_shared<OpaqueContentRenderNode>(m_pGraphResources, this);
	auto pDeferredLightingNode = std::make_shared<DeferredLightingRenderNode>(m_pGraphResources, this);
	auto pBlurNode = std::make_shared<BlurRenderNode>(m_pGraphResources, this);
	auto pLineDrawingNode = std::make_shared<LineDrawingRenderNode>(m_pGraphResources, this);
	auto pTransparencyNode = std::make_shared<TransparentContentRenderNode>(m_pGraphResources, this);
	auto pBlendNode = std::make_shared<TransparencyBlendRenderNode>(m_pGraphResources, this);
	auto pDOFNode = std::make_shared<DepthOfFieldRenderNode>(m_pGraphResources, this);

	m_pRenderGraph->AddRenderNode("ShadowMapNode", pShadowMapNode);
	m_pRenderGraph->AddRenderNode("GBufferNode", pGBufferNode);
	m_pRenderGraph->AddRenderNode("OpaqueNode", pOpaqueNode);
	m_pRenderGraph->AddRenderNode("DeferredLightingNode", pDeferredLightingNode);
	m_pRenderGraph->AddRenderNode("BlurNode", pBlurNode);
	m_pRenderGraph->AddRenderNode("LineDrawingNode", pLineDrawingNode);
	m_pRenderGraph->AddRenderNode("TransparencyNode", pTransparencyNode);
	m_pRenderGraph->AddRenderNode("BlendNode", pBlendNode);
	m_pRenderGraph->AddRenderNode("DOFNode", pDOFNode);

	// TODO: make the connection automatic
	pShadowMapNode->ConnectNext(pOpaqueNode);
	pGBufferNode->ConnectNext(pOpaqueNode);
	pOpaqueNode->ConnectNext(pDeferredLightingNode);
	pDeferredLightingNode->ConnectNext(pBlurNode);
	pBlurNode->ConnectNext(pLineDrawingNode);
	pLineDrawingNode->ConnectNext(pTransparencyNode);
	pTransparencyNode->ConnectNext(pBlendNode);
	pBlendNode->ConnectNext(pDOFNode);

	// Define resource dependencies

	pOpaqueNode->SetInputResource(OpaqueContentRenderNode::INPUT_SHADOW_MAP, ShadowMapRenderNode::OUTPUT_DEPTH_TEXTURE);
	pOpaqueNode->SetInputResource(OpaqueContentRenderNode::INPUT_GBUFFER_NORMAL, GBufferRenderNode::OUTPUT_NORMAL_GBUFFER);

	pDeferredLightingNode->SetInputResource(DeferredLightingRenderNode::INPUT_GBUFFER_COLOR, OpaqueContentRenderNode::OUTPUT_COLOR_TEXTURE);
	pDeferredLightingNode->SetInputResource(DeferredLightingRenderNode::INPUT_GBUFFER_NORMAL, GBufferRenderNode::OUTPUT_NORMAL_GBUFFER);
	pDeferredLightingNode->SetInputResource(DeferredLightingRenderNode::INPUT_GBUFFER_POSITION, GBufferRenderNode::OUTPUT_POSITION_GBUFFER);
	pDeferredLightingNode->SetInputResource(DeferredLightingRenderNode::INPUT_DEPTH_TEXTURE, OpaqueContentRenderNode::OUTPUT_DEPTH_TEXTURE);

	pBlurNode->SetInputResource(BlurRenderNode::INPUT_COLOR_TEXTURE, OpaqueContentRenderNode::OUTPUT_LINE_SPACE_TEXTURE);

	pLineDrawingNode->SetInputResource(LineDrawingRenderNode::INPUT_COLOR_TEXTURE, DeferredLightingRenderNode::OUTPUT_COLOR_TEXTURE);
	pLineDrawingNode->SetInputResource(LineDrawingRenderNode::INPUT_LINE_SPACE_TEXTURE, BlurRenderNode::OUTPUT_COLOR_TEXTURE);

	pTransparencyNode->SetInputResource(TransparentContentRenderNode::INPUT_COLOR_TEXTURE, LineDrawingRenderNode::OUTPUT_COLOR_TEXTURE);
	pTransparencyNode->SetInputResource(TransparentContentRenderNode::INPUT_BACKGROUND_DEPTH, OpaqueContentRenderNode::OUTPUT_DEPTH_TEXTURE);

	pBlendNode->SetInputResource(TransparencyBlendRenderNode::INPUT_OPQAUE_COLOR_TEXTURE, LineDrawingRenderNode::OUTPUT_COLOR_TEXTURE);
	pBlendNode->SetInputResource(TransparencyBlendRenderNode::INPUT_OPQAUE_DEPTH_TEXTURE, OpaqueContentRenderNode::OUTPUT_DEPTH_TEXTURE);
	pBlendNode->SetInputResource(TransparencyBlendRenderNode::INPUT_TRANSPARENCY_COLOR_TEXTURE, TransparentContentRenderNode::OUTPUT_COLOR_TEXTURE);
	pBlendNode->SetInputResource(TransparencyBlendRenderNode::INPUT_TRANSPARENCY_DEPTH_TEXTURE, TransparentContentRenderNode::OUTPUT_DEPTH_TEXTURE);

	pDOFNode->SetInputResource(DepthOfFieldRenderNode::INPUT_COLOR_TEXTURE, TransparencyBlendRenderNode::OUTPUT_COLOR_TEXTURE);
	pDOFNode->SetInputResource(DepthOfFieldRenderNode::INPUT_GBUFFER_POSITION, GBufferRenderNode::OUTPUT_POSITION_GBUFFER);
	pDOFNode->SetInputResource(DepthOfFieldRenderNode::INPUT_SHADOW_MARK_TEXTURE, OpaqueContentRenderNode::OUTPUT_COLOR_TEXTURE);

	// Initialize render graph

	m_pRenderGraph->SetupRenderNodes();
	m_pRenderGraph->BuildRenderNodePriorities();

	for (uint32_t i = 0; i < m_pRenderGraph->GetRenderNodeCount(); i++)
	{
		m_commandRecordReadyList.emplace(i, nullptr);
		m_commandRecordReadyListFlag.emplace(i, false);
	}
}

void StandardRenderer::Draw(const std::vector<std::shared_ptr<IEntity>>& drawList, const std::shared_ptr<IEntity> pCamera)
{
	if (!pCamera)
	{
		return;
	}

	auto pContext = std::make_shared<RenderContext>();
	pContext->pCamera = pCamera;
	pContext->pDrawList = &drawList;

	if (m_eGraphicsDeviceType == EGraphicsDeviceType::Vulkan)
	{
		for (auto& item : m_commandRecordReadyList)
		{
			item.second = nullptr;
			m_commandRecordReadyListFlag[item.first] = false;
		}

		m_pRenderGraph->BeginRenderPassesParallel(pContext);

		// Submit async recorded command buffers by correct sequence

		unsigned int finishedNodeCount = 0;

		while (finishedNodeCount < m_pRenderGraph->GetRenderNodeCount())
		{
			std::vector<std::pair<uint32_t, std::shared_ptr<DrawingCommandBuffer>>> buffersToReturn;

			{
				std::unique_lock<std::mutex> lock(m_commandRecordListWriteMutex);
				m_commandRecordListCv.wait(lock, [this]() { return m_newCommandRecorded; });
				m_newCommandRecorded = false;

				std::vector<uint32_t> sortedQueueContents;
				std::queue<uint32_t>  continueWaitQueue; // Command buffers that shouldn't be submitted as prior dependencies have not finished

				// Eliminate dependency jump
				while (!m_writtenCommandPriorities.empty())
				{
					sortedQueueContents.emplace_back(m_writtenCommandPriorities.front());
					m_writtenCommandPriorities.pop();
				}
				std::sort(sortedQueueContents.begin(), sortedQueueContents.end());

				for (unsigned int i = 0; i < sortedQueueContents.size(); i++)
				{
					bool proceed = true;
					uint32_t currPriority = sortedQueueContents[i];
					for (uint32_t& id : m_pRenderGraph->m_nodePriorityDependencies[currPriority]) // Check if dependent nodes have finished
					{
						if (!m_commandRecordReadyListFlag[id])
						{
							continueWaitQueue.push(currPriority);
							proceed = false;
							break;
						}
					}
					if (proceed)
					{
						m_commandRecordReadyListFlag[currPriority] = true;
						m_commandRecordReadyList[currPriority]->m_debugID = currPriority;
						buffersToReturn.emplace_back(currPriority, m_commandRecordReadyList[currPriority]);
					}
				}

				while (!continueWaitQueue.empty())
				{
					m_writtenCommandPriorities.push(continueWaitQueue.front());
					continueWaitQueue.pop();
				}
			}

			if (buffersToReturn.size() > 0)
			{
				std::sort(buffersToReturn.begin(), buffersToReturn.end(),
					[](const std::pair<uint32_t, std::shared_ptr<DrawingCommandBuffer>>& lhs, std::pair<uint32_t, std::shared_ptr<DrawingCommandBuffer>>& rhs)
					{
						return lhs.first < rhs.first;
					});

				for (unsigned int i = 0; i < buffersToReturn.size(); i++)
				{
					m_pDevice->ReturnExternalCommandBuffer(buffersToReturn[i].second);
				}

				finishedNodeCount += buffersToReturn.size();
				m_pDevice->FlushCommands(false, false);
			}
		}
	}
	else // OpenGL
	{
		m_pRenderGraph->BeginRenderPassesSequential(pContext);
	}
}

void StandardRenderer::WriteCommandRecordList(const char* pNodeName, const std::shared_ptr<DrawingCommandBuffer>& pCommandBuffer)
{
	{
		std::lock_guard<std::mutex> guard(m_commandRecordListWriteMutex);

		m_commandRecordReadyList[m_pRenderGraph->m_renderNodePriorities[pNodeName]] = pCommandBuffer;		
		m_writtenCommandPriorities.push(m_pRenderGraph->m_renderNodePriorities[pNodeName]);
		m_newCommandRecorded = true;
	}

	m_commandRecordListCv.notify_all();
}