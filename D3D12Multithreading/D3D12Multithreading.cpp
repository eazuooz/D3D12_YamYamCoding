//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "D3D12Multithreading.h"
#include "FrameResource.h"

D3D12Multithreading* D3D12Multithreading::s_app = nullptr;

D3D12Multithreading::D3D12Multithreading(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_keyboardInput(),
    m_titleCount(0),
    m_cpuTime(0),
    m_fenceValue(0),
    m_rtvDescriptorSize(0),
    m_currentFrameResourceIndex(0),
    m_pCurrentFrameResource(nullptr)
{
    s_app = this;

    m_keyboardInput.animate = true;

    ThrowIfFailed(DXGIDeclareAdapterRemovalSupport());
}

D3D12Multithreading::~D3D12Multithreading()
{
    s_app = nullptr;
}

void D3D12Multithreading::OnInit()
{
    LoadPipeline();
    LoadAssets();
    LoadContexts();
}

// Load the rendering pipeline dependencies.
void D3D12Multithreading::LoadPipeline()
{
   
}

// Load the sample assets.
void D3D12Multithreading::LoadAssets()
{
    
}

// Initialize threads and events.
void D3D12Multithreading::LoadContexts()
{

}

// Update frame-based values.
void D3D12Multithreading::OnUpdate()
{
    
}

// Render the scene.
void D3D12Multithreading::OnRender()
{
    try
    {
    
    }
    catch (HrException& e)
    {
        
    }
}

// Release sample's D3D objects.
void D3D12Multithreading::ReleaseD3DResources()
{
    m_fence.Reset();
    ResetComPtrArray(&m_renderTargets);
    m_commandQueue.Reset();
    m_swapChain.Reset();
    m_device.Reset();
}

// Tears down D3D resources and reinitializes them.
void D3D12Multithreading::RestoreD3DResources()
{
    // Give GPU a chance to finish its execution in progress.
    try
    {
        WaitForGpu();
    }
    catch (HrException&)
    {
        // Do nothing, currently attached adapter is unresponsive.
    }
    ReleaseD3DResources();
    OnInit();
}

// Wait for pending GPU work to complete.
void D3D12Multithreading::WaitForGpu()
{

}

void D3D12Multithreading::OnDestroy()
{
    
}

void D3D12Multithreading::OnKeyDown(UINT8 key)
{
    switch (key)
    {
    case VK_LEFT:
        m_keyboardInput.leftArrowPressed = true;
        break;
    case VK_RIGHT:
        m_keyboardInput.rightArrowPressed = true;
        break;
    case VK_UP:
        m_keyboardInput.upArrowPressed = true;
        break;
    case VK_DOWN:
        m_keyboardInput.downArrowPressed = true;
        break;
    case VK_SPACE:
        m_keyboardInput.animate = !m_keyboardInput.animate;
        break;
    }
}

void D3D12Multithreading::OnKeyUp(UINT8 key)
{
    switch (key)
    {
    case VK_LEFT:
        m_keyboardInput.leftArrowPressed = false;
        break;
    case VK_RIGHT:
        m_keyboardInput.rightArrowPressed = false;
        break;
    case VK_UP:
        m_keyboardInput.upArrowPressed = false;
        break;
    case VK_DOWN:
        m_keyboardInput.downArrowPressed = false;
        break;
    }
}

// Assemble the CommandListPre command list.
void D3D12Multithreading::BeginFrame()
{
    m_pCurrentFrameResource->Init();

    // Indicate that the back buffer will be used as a render target.
    m_pCurrentFrameResource->m_commandLists[CommandListPre]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the render target and depth stencil.
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    m_pCurrentFrameResource->m_commandLists[CommandListPre]->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_pCurrentFrameResource->m_commandLists[CommandListPre]->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    ThrowIfFailed(m_pCurrentFrameResource->m_commandLists[CommandListPre]->Close());
}

// Assemble the CommandListMid command list.
void D3D12Multithreading::MidFrame()
{
    // Transition our shadow map from the shadow pass to readable in the scene pass.
    m_pCurrentFrameResource->SwapBarriers();

    ThrowIfFailed(m_pCurrentFrameResource->m_commandLists[CommandListMid]->Close());
}

// Assemble the CommandListPost command list.
void D3D12Multithreading::EndFrame()
{
    m_pCurrentFrameResource->Finish();

    // Indicate that the back buffer will now be used to present.
    m_pCurrentFrameResource->m_commandLists[CommandListPost]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_pCurrentFrameResource->m_commandLists[CommandListPost]->Close());
}

// Worker thread body. workerIndex is an integer from 0 to NumContexts 
// describing the worker's thread index.
void D3D12Multithreading::WorkerThread(int threadIndex)
{
    assert(threadIndex >= 0);
    assert(threadIndex < NumContexts);
#if !SINGLETHREADED

    while (threadIndex >= 0 && threadIndex < NumContexts)
    {
        // Wait for main thread to tell us to draw.

        WaitForSingleObject(m_workerBeginRenderFrame[threadIndex], INFINITE);

#endif
        ID3D12GraphicsCommandList* pShadowCommandList = m_pCurrentFrameResource->m_shadowCommandLists[threadIndex].Get();
        ID3D12GraphicsCommandList* pSceneCommandList = m_pCurrentFrameResource->m_sceneCommandLists[threadIndex].Get();

        //
        // Shadow pass
        //

        // Populate the command list.
        SetCommonPipelineState(pShadowCommandList);
        m_pCurrentFrameResource->Bind(pShadowCommandList, FALSE, nullptr, nullptr);    // No need to pass RTV or DSV descriptor heap.

        // Set null SRVs for the diffuse/normal textures.
        pShadowCommandList->SetGraphicsRootDescriptorTable(0, m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());

        // Distribute objects over threads by drawing only 1/NumContexts 
        // objects per worker (i.e. every object such that objectnum % 
        // NumContexts == threadIndex).
        PIXBeginEvent(pShadowCommandList, 0, L"Worker drawing shadow pass...");

        for (int j = threadIndex; j < _countof(SampleAssets::Draws); j += NumContexts)
        {
            SampleAssets::DrawParameters drawArgs = SampleAssets::Draws[j];

            pShadowCommandList->DrawIndexedInstanced(drawArgs.IndexCount, 1, drawArgs.IndexStart, drawArgs.VertexBase, 0);
        }

        PIXEndEvent(pShadowCommandList);

        ThrowIfFailed(pShadowCommandList->Close());

#if !SINGLETHREADED
        // Submit shadow pass.
        SetEvent(m_workerFinishShadowPass[threadIndex]);
#endif

        //
        // Scene pass
        // 

        // Populate the command list.  These can only be sent after the shadow 
        // passes for this frame have been submitted.
        SetCommonPipelineState(pSceneCommandList);
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
        m_pCurrentFrameResource->Bind(pSceneCommandList, TRUE, &rtvHandle, &dsvHandle);

        PIXBeginEvent(pSceneCommandList, 0, L"Worker drawing scene pass...");

        D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvHeapStart = m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
        const UINT cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        const UINT nullSrvCount = 2;
        for (int j = threadIndex; j < _countof(SampleAssets::Draws); j += NumContexts)
        {
            SampleAssets::DrawParameters drawArgs = SampleAssets::Draws[j];

            // Set the diffuse and normal textures for the current object.
            CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvHandle(cbvSrvHeapStart, nullSrvCount + drawArgs.DiffuseTextureIndex, cbvSrvDescriptorSize);
            pSceneCommandList->SetGraphicsRootDescriptorTable(0, cbvSrvHandle);

            pSceneCommandList->DrawIndexedInstanced(drawArgs.IndexCount, 1, drawArgs.IndexStart, drawArgs.VertexBase, 0);
        }

        PIXEndEvent(pSceneCommandList);
        ThrowIfFailed(pSceneCommandList->Close());

#if !SINGLETHREADED
        // Tell main thread that we are done.
        SetEvent(m_workerFinishedRenderFrame[threadIndex]); 
    }
#endif
}

void D3D12Multithreading::SetCommonPipelineState(ID3D12GraphicsCommandList* pCommandList)
{
    pCommandList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvHeap.Get(), m_samplerHeap.Get() };
    pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    pCommandList->RSSetViewports(1, &m_viewport);
    pCommandList->RSSetScissorRects(1, &m_scissorRect);
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    pCommandList->IASetIndexBuffer(&m_indexBufferView);
    pCommandList->SetGraphicsRootDescriptorTable(3, m_samplerHeap->GetGPUDescriptorHandleForHeapStart());
    pCommandList->OMSetStencilRef(0);

    // Render targets and depth stencil are set elsewhere because the 
    // depth stencil depends on the frame resource being used.

    // Constant buffers are set elsewhere because they depend on the 
    // frame resource being used.

    // SRVs are set elsewhere because they change based on the object 
    // being drawn.
}
