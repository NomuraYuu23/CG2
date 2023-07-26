#pragma once

#include <DirectXMath.h>
#include <Windows.h>
#include <d3d12.h>
#include <string>
#include <wrl.h>
#include <dxcapi.h>

#pragma comment(lib, "dxcompiler.lib")

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"

/// <summary>
/// スプライト
/// </summary>
class Sprite
{
public:

	/// <summary>
	/// 頂点データ構造体
	/// </summary>
	struct VertexPosUv {
		DirectX::XMFLOAT3 pos; // xyz座標
		DirectX::XMFLOAT2 uv;  // uv座標
	};

	/// <summary>
	/// 定数バッファ用データ構造体
	/// </summary>
	struct ConstBufferData {
		DirectX::XMFLOAT4 color; // 色(RGBA)
		DirectX::XMMATRIX mat;   // 3D変換行列
	};

	struct VertexData {

		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;

	};
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};
	//Transform構造体
	struct TransformStructure {
		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};

public:

	/// <summary>
	/// 静的初期化
	/// </summary>
	/// <param name="device">デバイス</param>
	/// <param name="window_width">画面幅</param>
	/// <param name="window_height">画面高さ</param>
	/// <param name="directoryPath">ディレクトリパス</param>
	static void StaticInitialize(
		ID3D12Device* device, int window_width, int window_height,
		const std::wstring& directoryPath = L"Resources/");

	/// <summary>
	/// 静的前処理
	/// </summary>
	/// <param name="cmdList">描画コマンドリスト</param>
	static void PreDraw(ID3D12GraphicsCommandList* cmdList);

	/// <summary>
	/// 描画後処理
	/// </summary>
	static void PostDraw();

	/// <summary>
	/// スプライト生成
	/// </summary>
	/// <param name="textureHandle">テクスチャハンドル</param>
	/// <param name="position">座標</param>
	/// <param name="color">色</param>
	/// <param name="anchorpoint">アンカーポイント</param>
	/// <param name="isFlipX">左右反転</param>
	/// <param name="isFlipY">上下反転</param>
	/// <returns>生成されたスプライト</returns>
	static Sprite* Create(
		uint32_t textureHandle, const Vector3& scale, const Vector3& rotate, const Vector3& position);

private:

	// 頂点数
	static const int kVertNum = 6;
	// デバイス
	static ID3D12Device* sDevice;
	// ディスクリプタサイズ
	static UINT sDescriptorHandleIncrementSize;
	// コマンドリスト
	static ID3D12GraphicsCommandList* sCommandList;
	// ルートシグネチャ
	static Microsoft::WRL::ComPtr<ID3D12RootSignature> sRootSignature;
	// パイプラインステートオブジェクト
	static Microsoft::WRL::ComPtr<ID3D12PipelineState> sPipelineState;
	// 射影行列
	static DirectX::XMMATRIX sMatProjection;

public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Sprite();
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Sprite(
		uint32_t textureHandle, const Vector3& scale, const Vector3& rotate, const Vector3& position);

	/// <summary>
	/// 初期化
	/// </summary>
	/// <returns>成否</returns>
	bool Initialize();

	/// <summary>
	/// テクスチャハンドルの設定
	/// </summary>
	/// <param name="textureHandle"></param>
	void SetTextureHandle(uint32_t textureHandle);

	uint32_t GetTevtureHandle() { return textureHandle_;}

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private:
	// 頂点バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> vertBuff_;
	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff_;
	// 頂点バッファマップ
	VertexData* vertMap = nullptr;
	// 定数バッファマップ
	ConstBufferData* constMap = nullptr;
	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuff_;

	//インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW ibView_{};

	//インデックスリソースにデータを書き込む
	uint32_t* indexMap = nullptr;

	//Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズ
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixBuff_;
	//データを書き込む
	TransformationMatrix* transformationMatrixMap = nullptr;

	//CPUで動かす用のTransformを作る
	TransformStructure transformSprite;


	//テクスチャ番号
	UINT textureHandle_ = 0;
	// リソース設定
	D3D12_RESOURCE_DESC resourceDesc_;


private:
	/// <summary>
	/// 頂点データ転送
	/// </summary>
	void TransferVertices();

	//ログ
	static void Log(const std::string& message);

	//CompileShader
	static IDxcBlob* CompileShader(
		//CompilerするShanderファイルへのパス
		const std::wstring& filePath,
		//Compilenに使用するProfile
		const wchar_t* profile,
		//初期化で生成したものを3つ
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxCompiler,
		IDxcIncludeHandler* includeHandler);

	static std::wstring ConvertString(const std::string& str);

	static std::string ConvertString(const std::wstring& str);

	//Resource作成関数化
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const size_t& sizeInBytes);

};

