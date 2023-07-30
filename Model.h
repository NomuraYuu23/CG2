#pragma once
#include <DirectXMath.h>
#include <Windows.h>
#include <d3d12.h>
#include <string>
#include <wrl.h>
#include <dxcapi.h>

#pragma comment(lib, "dxcompiler.lib")

#include "DirectXCommon.h"

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"

#include "VertexData.h"
#include "TransformationMatrix.h"
#include "TransformStructure.h"

#include "Material.h"

#include <vector>

class Model
{

public:

	struct MaterialData {
		std::string textureFilePath;
	};

	struct ModelData {

		std::vector<VertexData> vertices;
		MaterialData material;
	
	};

	/// <summary>
	/// 静的初期化
	/// </summary>
	/// <param name="device">デバイス</param>
	static void StaticInitialize(ID3D12Device* device);

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
	/// 3Dモデル生成
	/// </summary>
	/// <returns></returns>
	static Model* Create(const std::string& directoryPath, const std::string& filename, DirectXCommon* dxCommon, Material* material);
	static Model* Create(const std::string& directoryPath, const std::string& filename, DirectXCommon* dxCommon, std::vector<Material*> materials);

private:

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

private:

	/// <summary>
	/// グラフィックパイプライン生成
	/// </summary>
	static void InitializeGraphicsPipeline();


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

public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const std::string& directoryPath, const std::string& filename, DirectXCommon* dxCommon, Material* material);
	void Initialize(const std::string& directoryPath, const std::string& filename, DirectXCommon* dxCommon, std::vector<Material*> materials);

	/// <summary>
	/// 更新
	/// </summary>
	void Update(const TransformStructure& transform, const TransformStructure& cameraTransform);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// メッシュデータ生成
	/// </summary>
	void CreateMesh(const std::string& directoryPath, const std::string& filename);

	Model::MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	//objファイルを読む
	std::vector<Model::ModelData> LoadObjFile(const std::string& directoryPath, const std::string& filename);


	/// <summary>
	/// テクスチャハンドルの設定
	/// </summary>
	/// <param name="textureHandle"></param>
	void SetTextureHandle(uint32_t textureHandle);

	//uint32_t GetTevtureHandle() { return textureHandle_; }

private:
	// 頂点バッファ
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertBuffs_;
	// 頂点バッファマップ
	std::vector<VertexData*> vertMaps;
	// 頂点バッファビュー
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vbViews_{};

	// TransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズ
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixBuff_;
	//データを書き込む
	TransformationMatrix* transformationMatrixMap = nullptr;

	//CPUで動かす用のTransformを作る
	TransformStructure transform_;


	//以下複数にする

	//モデル読み込み
	std::vector<Model::ModelData> modelDatas_;

	//テクスチャ番号
	std::vector<UINT> textureHandles_;
	// リソース設定
	std::vector<D3D12_RESOURCE_DESC> resourceDescs_;

	// マテリアル
	std::vector<Material*> materials_;

};

