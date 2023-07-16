#pragma once

#include <Windows.h>
#include <chrono>
#include <cstdlib>
#include <d3d12.h>
#include "externals/DirectXTex/d3dx12.h"
#include <dxgi1_6.h>
#include <wrl.h>

#include "WinApp.h"

/// <summary>
/// DirectX�ėp
/// </summary>
class DirectXCommon
{
public:
	/// <summary>
	/// �V���O���g���C���X�^���X�̎擾
	/// </summary>
	/// <returns></returns>
	static DirectXCommon* GetInstance();

	/// <summary>
	/// ������
	/// </summary>
	/// <param name="winApp">WinApp</param>
	/// <param name="backBufferWidth"></param>
	/// <param name="backBufferHeight"></param>
	void Initialize(
		WinApp* winApp, int32_t backBufferWidth = WinApp::kWindowWidth,
		int32_t backBufferHeight = WinApp::kWindowHeight);

	/// <summary>
	/// �`��O����
	/// </summary>
	void PreDraw();

	/// <summary>
	/// �`��㏈��
	/// </summary>
	void PostDraw();

	/// <summary>
	/// �����_�[�^�[�Q�b�g�̃N���A
	/// </summary>
	void ClearRenderTarget();

	/// <summary>
	/// �[�x�o�b�t�@�̃N���A
	/// </summary>
	void ClearDepthBuffer();

	ID3D12Device* GetDevice() const { return device_.Get(); }

	ID3D12GraphicsCommandList* GetCommadList() const { return commandList_.Get(); }

	int32_t GetBackBufferWidth() const { return backBufferWidth_; }

	int32_t GetBackBufferHeight() const { return backBufferHeight_; }

private:

	WinApp* winApp_;

	//Direct3D�֘A
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2];
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	UINT64 fenceVal_ = 0;
	int32_t backBufferWidth_ = 0;
	int32_t backBufferHeight_ = 0;

private:
	DirectXCommon() = default;
	~DirectXCommon() = default;
	DirectXCommon(const DirectXCommon&) = delete;
	const DirectXCommon& operator=(const DirectXCommon&) = delete;

	/// <summary>
	/// ���O
	/// </summary>
	/// <param name="message"></param>
	void Log(const std::string& message);
	
	/// <summary>
	/// �R���o�[�g�X�g�����O
	/// </summary>
	/// <param name="message"></param>
	std::wstring ConvertString(const std::string& str);

	/// <summary>
	/// �R���o�[�g�X�g�����O
	/// </summary>
	/// <param name="message"></param>
	std::string ConvertString(const std::wstring& str);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		Microsoft::WRL::ComPtr<ID3D12Device> device, const D3D12_DESCRIPTOR_HEAP_TYPE& heapType, UINT numDescriptors, bool shaderVisible);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);

	/// <summary>
	/// DXGI�f�o�C�X������
	/// </summary>
	void InitializeDXGIDevice();

	/// <summary>
	/// �X���b�v�`�F�[���̐���
	/// </summary>
	void CreateSwapChain();

	/// <summary>
	/// �R�}���h�֘A������
	/// </summary>
	void Initializecommand();

	/// <summary>
	/// �����_�[�^�[�Q�b�g����
	/// </summary>
	void CreateFinalRenderTarget();

	/// <summary>
	/// �[�x�o�b�t�@����
	/// </summary>
	void CreateDepthBuffer();

	/// <summary>
	/// �t�F���X����
	/// </summary>
	void CreateFence();


};

