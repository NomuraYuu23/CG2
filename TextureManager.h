#pragma once

#include <array>
#include "externals/DirectXTex/d3dx12.h"
#include <string>
#include <unordered_map>
#include <wrl.h>
#include "externals/DirectXTex/DirectXTex.h"

#include "WinApp.h"
#include "DirectXCommon.h"

using namespace DirectX;

//テクスチャマネージャー
class TextureManager
{
public:
	//ディスクリプタの数
	static const size_t kNumDescriptors = 256;

	struct Texture{
		// テクスチャリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		// シェーダーリソースビューのハンドル(CPU)
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;
		// シェーダーリソースビューのハンドル(GPU)
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;
		//名前
		std::string name;
	};

	/// <summary>
	/// 読み込み
	/// </summary>
	/// <param name="fileName">ファイル名</param>
	/// <returns>テクスチャハンドル</returns>
	static uint32_t Load(const std::string& fileName, DirectXCommon* dxCommon);

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static TextureManager* GetInstance();

	/// <summary>
	/// システム初期化
	/// </summary>
	/// <param name="device">デバイス</param>
	/// <param name="directoryPath"></param>
	void Initialize(ID3D12Device* device, std::string directoryPath = "Resources/");

	/// <summary>
	/// 全テクスチャリセット
	/// </summary>
	void ResetAll();

	/// <summary>
	/// リソース情報取得
	/// </summary>
	/// <param name="textureHandle">テクスチャハンドル</param>
	/// <returns>リソース情報</returns>
	const D3D12_RESOURCE_DESC GetResourceDesc(uint32_t textureHandle);

	/// <summary>
	/// ディスクリプタテーブルをセット
	/// </summary>
	/// <param name="commandList">コマンドリスト</param>
	/// <param name="rootParamIndex">ルートパラメータ番号</param>
	/// <param name="textureHandle">テクスチャハンドル</param>
	void SetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList* commandList, UINT rootParamIndex, uint32_t textureHandle);

	static ID3D12DescriptorHeap* StaticGetDescriptorHeap() { return GetInstance()->descriptorHeap_.Get(); }

	static D3D12_CPU_DESCRIPTOR_HANDLE StaticGetCPUDescriptorHandle() { return GetInstance()->GetCPUDescriptorHandle(GetInstance()->descriptorHeap_.Get(), GetInstance()->device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), 0); }

	static D3D12_GPU_DESCRIPTOR_HANDLE StaticGetGPUDescriptorHandle() { return GetInstance()->GetGPUDescriptorHandle(GetInstance()->descriptorHeap_.Get(), GetInstance()->device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), 0); }


private:
	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;

	//デバイス
	ID3D12Device* device_;
	//ディスクリプタサイズ
	UINT descriptorHandleIncrementSize = 0u;
	//ディレクトリパス
	std::string directoryPath_;
	//ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;
	//次に使うディスクリプタヒープの番号
	uint32_t indexNextDescriptorHeap = 0u;
	//テクスチャコンテナ
	std::array<Texture, kNumDescriptors> textures_;

	//コンバートストリング
	std::wstring ConvertString(const std::string& str);
	//コンバートストリング
	std::string ConvertString(const std::wstring& str);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// Resource作成関数化
	/// </summary>
	/// <param name="sizeInBytes"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const size_t& sizeInBytes);

	/// <summary>
	/// テキストデータを読む
	/// </summary>
	ScratchImage LoadTexture(const std::string& filePath);
	
	/// <summary>
	/// TextureResourceを作る
	/// </summary>
	/// <param name="metadata"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const TexMetadata& metadata);

	/// <summary>
	/// TextureResourceにデータを転送する
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const ScratchImage& mipImages,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);

	/// <summary>
	/// 読み込み
	/// </summary>
	/// <param name="fileName">ファイル名</param>
	/// <returns></returns>
	uint32_t LoadInternal(const std::string& fileName ,DirectXCommon* dxCommon);

};

