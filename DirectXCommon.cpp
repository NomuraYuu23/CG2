#include "DirectXCommon.h"
#include "SafeDelete.h"
#include <algorithm>
#include <cassert>
#include <thread>
#include <timeapi.h>
#include <vector>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Winmm.lib")

using namespace Microsoft::WRL;

DirectXCommon* DirectXCommon::GetInstance() {
	static DirectXCommon instance;
	return &instance;
}

// ������
void DirectXCommon::Initialize(
	WinApp* winApp, int32_t backBufferWidth = WinApp::kWindowWidth,
	int32_t backBufferHeight = WinApp::kWindowHeight) {

	// nullptr�`�F�b�N
	assert(winApp);
	assert(4 <= backBufferWidth && backBufferWidth <= 4096);
	assert(4 <= backBufferHeight && backBufferHeight <= 4096);

	winApp_ = winApp;
	backBufferWidth_ = backBufferWidth;
	backBufferHeight_ = backBufferHeight;

	// DXGI�f�o�C�X������
	InitializeDXGIDevice();

	// �R�}���h�֘A������
	Initializecommand();

	// �X���b�v�`�F�[���̐���
	CreateSwapChain();

	// �����_�[�^�[�Q�b�g����
	CreateFinalRenderTarget();

	// �[�x�o�b�t�@����
	CreateDepthBuffer();

	// �t�F���X����
	CreateFence();

}

// �`��O����
void DirectXCommon::PreDraw() {

	//���ꂩ�珑�����ރo�b�N�o�b�t�@�̃C���f�b�N�X���擾
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	//TransitionBarrier�̐ݒ�
	D3D12_RESOURCE_BARRIER barrier{};
	//����̃o���A��Transition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//None�ɂ��Ă���
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//�o���A�𒣂�Ώۂ̃��\�[�X�B���݂̃o�b�N�o�b�t�@�ɑ΂��čs��
	barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
	//�J�ڑO�i���݁j��ResouceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//�J�ڌ��ResoureState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//TransitionBarrier�𒣂�
	commandList_->ResourceBarrier(1, &barrier);

	//DescriptorSize���擾���Ă���
	const uint32_t desriptorSizeRTV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t desriptorSizeDSV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//RTV�̐ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//�o�͌��ʂ�SRGB�ɕϊ����ď�������
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2d�̃e�N�X�`���Ƃ��ď�������

	//�f�B�X�N���v�^�̐擪���擾����
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = GetCPUDescriptorHandle(rtvHeap_.Get(), desriptorSizeRTV, 0);;
	//RTV��2���̂Ńf�B�X�N���v�^��2�p��
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	//�܂�1�ڂ����B1�ڂ͍ŏ��̂Ƃ���ɍ��hr�B���ꏊ��������Ŏw�肵�Ă�����K�v������
	rtvHandles[0] = rtvStartHandle;
	device_->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
	//2�ڂ̃f�B�X�N���v�^�n���h���𓾂�(���͂�)
	rtvHandles[1].ptr = rtvHandles[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


	//�`����DSV��RTV��ݒ肷��
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetCPUDescriptorHandle(dsvHeap_.Get(), desriptorSizeDSV, 0);
	commandList_->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);

	// �S��ʃN���A
	ClearRenderTarget();

	// �[�x�o�b�t�@�N���A
	ClearDepthBuffer();

	//�r���[�|�[�g
	D3D12_VIEWPORT viewport{};
	//�N���C�A���g�̈�̃T�C�Y�ƈꏏ�ɂ��ĉ�ʑS�̂ɕ\��
	viewport.Width = winApp_->kWindowWidth;
	viewport.Height = winApp_->kWindowHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	commandList_->RSSetViewports(1, &viewport);//Viewport��ݒ�
	
	//�V�U�[��`
	D3D12_RECT scissorRect{};
	//��{�I�Ƀr���[�|�[�g�Ɠ�����`���\�������悤�ɂ���
	scissorRect.left = 0;
	scissorRect.right = winApp_->kWindowWidth;
	scissorRect.top = 0;
	scissorRect.bottom = winApp_->kWindowHeight;
	commandList_->RSSetScissorRects(1, &scissorRect);//Scirssor��ݒ�

}
// �`��㏈��
void DirectXCommon::PostDraw() {

	//���ꂩ�珑�����ރo�b�N�o�b�t�@�̃C���f�b�N�X���擾
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	//TransitionBarrier�̐ݒ�
	D3D12_RESOURCE_BARRIER barrier{};
	//����̃o���A��Transition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//None�ɂ��Ă���
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//�o���A�𒣂�Ώۂ̃��\�[�X�B���݂̃o�b�N�o�b�t�@�ɑ΂��čs��
	barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();

	//��ʂɕ`�������͂��ׂďI���A��ʂɉf���̂ŁA��Ԃ�J��
	//�����RenderTarget����Present�ɂ���
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//TransitionBarrier�𒣂�
	commandList_->ResourceBarrier(1, &barrier);

	//�R�}���h���X�g�̓��e���m�肳����B���ׂẴR�}���h��ς�ł���Close���邱��
	HRESULT hr = commandList_->Close();
	assert(SUCCEEDED(hr));

	//GPU�ɃR�}���h���X�g�̎��s���s�킹��
	ID3D12CommandList* commandLists[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(1, commandLists);
	//GPU��OS�ɉ�ʂ̌������s���悤�ɒʒm����
	swapChain_->Present(1, 0);

	//Fence�̒l���X�V
	fenceVal_++;
	//GPU�������܂ł��ǂ蒅�����Ƃ��ɁAFence�̒l���w�肵���l�ɑ������悤��Signal�𑗂�
	commandQueue_->Signal(fence_.Get(), fenceVal_);

	//Fence�̒l���w�肵��Signal�l�ɂ��ǂ蒅���Ă��邪�m�F����
	//GetCompletedValue�̏����l��Fence�쐬���ɓn���������l
	if (fence_->GetCompletedValue() < fenceVal_) {
		//Frence��Signal�������߂̃C�x���g���쐬����
		HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		assert(fenceEvent != nullptr);
		//�w�肵��Signal�ɂ��ǂ���Ă��Ȃ��̂ŁA���ǂ���܂ő҂悤�ɃC�x���g��ݒ肷��
		fence_->SetEventOnCompletion(fenceVal_, fenceEvent);
		//�C�x���g��҂�
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	//���̃t���[���p�̃R�}���h���X�g������
	hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr));

}

// �����_�[�^�[�Q�b�g�̃N���A
void DirectXCommon::ClearRenderTarget() {

	//���ꂩ�珑�����ރo�b�N�o�b�t�@�̃C���f�b�N�X���擾
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	//DescriptorSize���擾���Ă���
	const uint32_t desriptorSizeRTV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t desriptorSizeDSV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//RTV�̐ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//�o�͌��ʂ�SRGB�ɕϊ����ď�������
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2d�̃e�N�X�`���Ƃ��ď�������

	//�f�B�X�N���v�^�̐擪���擾����
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = GetCPUDescriptorHandle(rtvHeap_.Get(), desriptorSizeRTV, 0);;
	//RTV��2���̂Ńf�B�X�N���v�^��2�p��
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	//�܂�1�ڂ����B1�ڂ͍ŏ��̂Ƃ���ɍ��hr�B���ꏊ��������Ŏw�肵�Ă�����K�v������
	rtvHandles[0] = rtvStartHandle;
	//device_->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
	//2�ڂ̃f�B�X�N���v�^�n���h���𓾂�(���͂�)
	rtvHandles[1].ptr = rtvHandles[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//�w�肵���F�ŉ�ʑS�̂��N���A����
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };//���ۂ��F�BRGBA�̏�
	commandList_->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

}
// �[�x�o�b�t�@�̃N���A
void DirectXCommon::ClearDepthBuffer() {

	//���ꂩ�珑�����ރo�b�N�o�b�t�@�̃C���f�b�N�X���擾
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	//DescriptorSize���擾���Ă���
	const uint32_t desriptorSizeRTV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t desriptorSizeDSV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//RTV�̐ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//�o�͌��ʂ�SRGB�ɕϊ����ď�������
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2d�̃e�N�X�`���Ƃ��ď�������

	//�f�B�X�N���v�^�̐擪���擾����
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = GetCPUDescriptorHandle(rtvHeap_.Get(), desriptorSizeRTV, 0);;
	//RTV��2���̂Ńf�B�X�N���v�^��2�p��
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	//�܂�1�ڂ����B1�ڂ͍ŏ��̂Ƃ���ɍ��hr�B���ꏊ��������Ŏw�肵�Ă�����K�v������
	rtvHandles[0] = rtvStartHandle;
	//device_->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
	//2�ڂ̃f�B�X�N���v�^�n���h���𓾂�(���͂�)
	rtvHandles[1].ptr = rtvHandles[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//�`����DSV��RTV��ݒ肷��
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetCPUDescriptorHandle(dsvHeap_.Get(), desriptorSizeDSV, 0);
	//�w�肵���[�x�ŉ�ʑS�̂��N���A����
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

}

//���O
void DirectXCommon::Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}

//�R���o�[�g�X�g�����O
std::wstring DirectXCommon::ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

//�R���o�[�g�X�g�����O
std::string DirectXCommon::ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;

}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;

}

//�f�B�X�N���v�g�q�[�v�֐�
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(
	Microsoft::WRL::ComPtr<ID3D12Device> device, const D3D12_DESCRIPTOR_HEAP_TYPE& heapType, UINT numDescriptors, bool shaderVisible) {

	//�f�B�X�N���v�^�q�[�v�̐���
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;//�����_�[�^�[�Q�b�g�r���[�p
	descriptorHeapDesc.NumDescriptors = numDescriptors;//�_�u���o�b�t�@�p��2�B�����Ă��\��Ȃ�
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	//�f�B�X�N���v�^�q�[�v�����Ȃ������̂ŋN���ł��Ȃ�
	assert(SUCCEEDED(hr));
	return descriptorHeap;

}

//DepthStencilTexture�����
Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height) {

	//��������Resource�̐ݒ�
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//���p����Heap�̐ݒ�
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	//�[�x�l�̃N���A�ݒ�
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//Resoure�̐���
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;

}

// DXGI�f�o�C�X������
void DirectXCommon::InitializeDXGIDevice() {

	//HRESULT7��Windows�n�̃G���[�R�[�h�ł���A
	//�֐��������������ǂ�����SUCCEDED�}�N���Ŕ���ł���
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	//�������̍��{�G�ȕ����ŃG���[���o���ꍇ�̓v���O�������Ԉ���Ă��邩�A�ǂ��ɂ��ł��Ȃ��ꍇ�������̂�assert�ɂ��Ă���
	assert(SUCCEEDED(hr));

	//�g�p����A�_�v�^�p�̕ϐ��A�ŏ���nullptr�����Ă���
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
	//�ǂ����ɃA�_�v�^�𗊂�
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; ++i) {
		//�A�_�v�^�̏����擾����
		DXGI_ADAPTER_DESC3 adapterDesc{};
		assert(SUCCEEDED(hr));//�擾�ł��Ȃ��͈̂�厖
		//�\�t�g�E�F�A�A�_�v�^�łȂ���΍̗p
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//�̗p�����A�_�v�^�̏������O�ɏo�́Awstring�̕��Ȃ̂Œ���
			Log(ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;//�\�t�g�E�F�A�A�_�v�^�̏ꍇ�͌��Ȃ��������Ƃɂ���
	}
	//�K�؂ȃA�_�v�^��������Ȃ������̂ŋN���ł��Ȃ�
	assert(useAdapter != nullptr);

	//�@�\���x���ƃ��O�o�͗p�̕�����
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };
	//�������ɐ����ł��邩�����Ă���
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		//�̗p�����A�_�v�^�Ńf�o�C�X�𐶐�
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
		//�w�肵���@�\���x���Ńf�o�C�X�������ł��������m�F
		if (SUCCEEDED(hr)) {
			//�����ł����̂Ń��O�o�͂��s���ă��[�v�𔲂���
			Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}

	}
	//�f�o�C�X�̐��������܂������Ȃ������̂ŋN���ł��Ȃ�
	assert(device_ != nullptr);
	Log("complete create D3D12Device!!!\n");//�����������̃��O������

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//���o���G���[���Ɏ~�܂�
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//�G���[���Ɏ~�܂�
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//�x�����Ɏ~�܂�
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		//�}�����郁�b�Z�[�W��ID
		D3D12_MESSAGE_ID denyIds[] = {
			//Windows11�ł�DXGI�f�o�b�O���C���[��DX12�f�o�b�O���C���[�̑��ݍ�p�ɂ��G���[���b�Z�[�W
			//http://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
				D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//�}�����郌�x��
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//�w�肵�����b�Z�[�W�̕\����}������
		infoQueue->PushStorageFilter(&filter);

	}
#endif


}

// �X���b�v�`�F�[���̐���
void DirectXCommon::CreateSwapChain() {

	//�X���b�v�`�F�[���𐶐�����
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = backBufferWidth_;//��ʂ̕��B�E�B���h�E�̃N���C�A���g�̈�𓯂����̂ɂ��Ă���
	swapChainDesc.Height = backBufferHeight_;//��ʂ̍����B�E�B���h�E�̃N���C�A���g�̈�𓯂����̂ɂ��Ă���
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//�F�̌`��
	swapChainDesc.SampleDesc.Count = 1;//�}���`�T���v�����Ȃ�
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//�`��̃^�[�Q�b�g�Ƃ��ė��p����
	swapChainDesc.BufferCount = 2;//�_�u���o�b�t�@
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//���j�^�ɂ�������A���g��j��
	//�R�}���h�L���[�A�E�B���h�E�n���h���A�ݒ��n���Đ�������
	HRESULT hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), winApp_->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));

}

// �R�}���h�֘A������
void DirectXCommon::Initializecommand() {

	//�R�}���h�L���[�𐶐�����
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	HRESULT hr = device_->CreateCommandQueue(&commandQueueDesc,
		IID_PPV_ARGS(&commandQueue_));
	//�R�}���h�L���[�̐��������܂������Ȃ������̂ŋN���ł��Ȃ�
	assert(SUCCEEDED(hr));

	//�R�}���h�A���P�[�^�𐶐�����
	hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	//�R�}���h�A���P�[�^�̐��������܂������Ȃ������̂ŋN���ł��Ȃ�
	assert(SUCCEEDED(hr));

	//�R�}���h���X�g�𐶐�����
	hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr,
		IID_PPV_ARGS(&commandList_));
	//�R�}���h���X�g�̐��������܂������Ȃ������̂ŋN���ł��Ȃ�
	assert(SUCCEEDED(hr));

}

// �����_�[�^�[�Q�b�g����
void DirectXCommon::CreateFinalRenderTarget() {

	//�f�B�X�N���v�^�q�[�v�̐���
	//RTV�p�̃q�[�v�f�B�X�N���v�^�̐���2�BRTV��Shader���ŐG����̂ł͂Ȃ��̂ŁAShaderVisible��false
	rtvHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	//SRV�p�̃q�[�v�f�B�X�N���v�^�̐���128�BSRV��Shader���ŐG����̂Ȃ̂ŁAShaderVisible��true
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);




	//SwapChain����Resource�����������Ă���
	swapChainResources[0] = { nullptr };
	swapChainResources[1] = { nullptr };
	HRESULT hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//���܂��擾�ł��Ȃ���΋N���ł��Ȃ�
	assert(SUCCEEDED(hr));
	hr = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	//���܂��擾�ł��Ȃ���΋N���ł��Ȃ�
	assert(SUCCEEDED(hr));

	//DescriptorSize���擾���Ă���
	//const uint32_t desriptorSizeSRV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t desriptorSizeRTV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//RTV�̐ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//�o�͌��ʂ�SRGB�ɕϊ����ď�������
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2d�̃e�N�X�`���Ƃ��ď�������
	//�f�B�X�N���v�^�̐擪���擾����
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = GetCPUDescriptorHandle(rtvHeap_.Get(), desriptorSizeRTV, 0);;
	//RTV��2���̂Ńf�B�X�N���v�^��2�p��
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	//�܂�1�ڂ����B1�ڂ͍ŏ��̂Ƃ���ɍ��hr�B���ꏊ��������Ŏw�肵�Ă�����K�v������
	rtvHandles[0] = rtvStartHandle;
	device_->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
	//2�ڂ̃f�B�X�N���v�^�n���h���𓾂�(���͂�)
	rtvHandles[1].ptr = rtvHandles[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//2�ڂ����
	device_->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);


}

// �[�x�o�b�t�@����
void DirectXCommon::CreateDepthBuffer() {


	const uint32_t desriptorSizeDSV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//DepthStencilTexture���E�B���h�E�̃T�C�Y�ō쐬
	depthStencilResource = CreateDepthStencilTextureResource(device_, backBufferWidth_, backBufferHeight_);

	//DSV�p�̃q�[�v
	dsvHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	//DSV�̐ݒ�
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//DSVHeap�̐擪��DSV������
	device_->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, GetCPUDescriptorHandle(dsvHeap_.Get(), desriptorSizeDSV, 0));

}

// �t�F���X����
void DirectXCommon::CreateFence() {

	//������0��Fence�����
	HRESULT hr = device_->CreateFence(fenceVal_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));

	//Frence��Signal�������߂̃C�x���g���쐬����
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);

}
