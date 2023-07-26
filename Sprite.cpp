#include "Sprite.h"
#include "TextureManager.h"
#include <cassert>

using namespace DirectX;
using namespace Microsoft::WRL;

// デバイス
ID3D12Device* Sprite::sDevice = nullptr;
// ディスクリプタサイズ
UINT Sprite::sDescriptorHandleIncrementSize;
// コマンドリスト
ID3D12GraphicsCommandList* Sprite::sCommandList = nullptr;
// ルートシグネチャ
ComPtr<ID3D12RootSignature> Sprite::sRootSignature;
// パイプラインステートオブジェクト
ComPtr<ID3D12PipelineState> Sprite::sPipelineState;
// 射影行列
DirectX::XMMATRIX Sprite::sMatProjection;

/// <summary>
/// 静的初期化
/// </summary>
/// <param name="device">デバイス</param>
/// <param name="window_width">画面幅</param>
/// <param name="window_height">画面高さ</param>
/// <param name="directoryPath">ディレクトリパス</param>
void Sprite::StaticInitialize(
	ID3D12Device* device, int window_width, int window_height,
	const std::wstring& directoryPath) {

	assert(device);
	
	sDevice = device;

	sDescriptorHandleIncrementSize = sDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	HRESULT hr;

	//dxcCompilerを初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcompiler));
	assert(SUCCEEDED(hr));

	//現時点でincludeはしないが、includeに対応するための設定を行っておく
	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;//0から始まる
	descriptorRange[0].NumDescriptors = 1;//数は一つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//Offsetを自動計算

	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootsignature{};
	descriptionRootsignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//RootParameter作成。複数設定できるので配列。今回は1つだけなので長さ1の配列
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   //CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;                   //レジスタ番号0とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   //CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0;                   //レジスタ番号0とバインド
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//Tableで利用する数
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   //CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//VertexShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1;
	descriptionRootsignature.pParameters = rootParameters;             //ルートパラメータ配列へのポインタ
	descriptionRootsignature.NumParameters = _countof(rootParameters); //配列の長さ

	//Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootsignature.pStaticSamplers = staticSamplers;
	descriptionRootsignature.NumStaticSamplers = _countof(staticSamplers);

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化
	depthStencilDesc.DepthEnable = true;
	//書き込み
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//シリアライズしてバイナリにする
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootsignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリを元に生成
	hr = sDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&sRootSignature));
	assert(SUCCEEDED(hr));

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	//すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	//ResiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shaderをコンパイルする
	IDxcBlob* vertexShaderBlob = CompileShader(L"Object3d.VS.hlsl",
		L"vs_6_0", dxcUtils, dxcompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);


	IDxcBlob* pixelShaderBlob = CompileShader(L"Object3d.PS.hlsl",
		L"ps_6_0", dxcUtils, dxcompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);

	//PSOを生成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = sRootSignature.Get();//RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;//InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };//PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;//BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むのかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//実際に生成
	hr = sDevice->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&sPipelineState));
	assert(SUCCEEDED(hr));

	// 射影行列計算
	sMatProjection = XMMatrixOrthographicOffCenterLH(
		0.0f, (float)window_width, (float)window_height, 0.0f, 0.0f, 1.0f
	);

}

/// <summary>
/// 静的前処理
/// </summary>
/// <param name="cmdList">描画コマンドリスト</param>
void Sprite::PreDraw(ID3D12GraphicsCommandList* cmdList) {

	assert(Sprite::sCommandList == nullptr);

	sCommandList = cmdList;

	//RootSignatureを設定。
	sCommandList->SetPipelineState(sPipelineState.Get());//PS0を設定
	sCommandList->SetGraphicsRootSignature(sRootSignature.Get());
	//形状を設定。PS0に設定しているものとは別。
	sCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

/// <summary>
/// 描画後処理
/// </summary>
void Sprite::PostDraw() {
	//コマンドリストを解除
	Sprite::sCommandList = nullptr;

}

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
Sprite* Sprite::Create(
	uint32_t textureHandle, DirectX::XMFLOAT2 position, DirectX::XMFLOAT4 color,
	DirectX::XMFLOAT2 anchorpoint, bool isFlipX, bool isFlipY) {

	// 仮サイズ
	XMFLOAT2 size = { 100.0f, 100.0f };

	// テクスチャ情報取得
	const D3D12_RESOURCE_DESC& resDesc = TextureManager::GetInstance()->GetResourceDesc(textureHandle);
	// スプライトのサイズをテクスチャのサイズに設定
	size = { (float)resDesc.Width, (float)resDesc.Height };

	// Spriteのインスタンスを生成
	Sprite* sprite = new Sprite(textureHandle, position, size, color, anchorpoint, isFlipX, isFlipY);
	if (sprite == nullptr) {
		return nullptr;
	}

	// 初期化
	if (!sprite->Initialize()) {
		delete sprite;
		assert(0);
		return nullptr;
	}

	return sprite;

}

/// <summary>
/// コンストラクタ
/// </summary>
Sprite::Sprite() {}

/// <summary>
/// コンストラクタ
/// </summary>
Sprite::Sprite(
	uint32_t textureHandle, DirectX::XMFLOAT2 position, DirectX::XMFLOAT2 size,
	DirectX::XMFLOAT4 color, DirectX::XMFLOAT2 anchorpoint, bool isFlipX, bool isFlipY) {

	position_ = position;
	size_ = size;
	anchorPoint_ = anchorPoint_;
	matWorld_ = XMMatrixIdentity();
	color_ = color;
	textureHandle_ = textureHandle;
	isFlipX_ = isFlipX;
	isFlipY_ = isFlipY;
	texSize_ = size;

}

/// <summary>
/// 初期化
/// </summary>
/// <returns>成否</returns>
bool Sprite::Initialize() {

	assert(sDevice);

	HRESULT hr;

	resourceDesc_ = TextureManager::GetInstance()->GetResourceDesc(textureHandle_);
	
	{

	//頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//UploadHeapを使う
	//頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	//バッファリソース。テクスチャの場合はまた別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeof(VertexPosUv) * kVertNum;//リソースのサイズ。今回はVector4を3頂点分
	//バッファの場合はこれらは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 頂点バッファ生成
	hr = sDevice->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&vertBuff_));
	assert(SUCCEEDED(hr));

	// 頂点バッファマッピング
	hr = vertBuff_->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(hr));

	}

	// 頂点バッファへのデータ転送
	TransferVertices();

	// 頂点バッファビューの作成
	
	//リソースの先頭のアドレスから使う
	vbView_.BufferLocation = vertBuff_->GetGPUVirtualAddress();
	//使用するリソースのサイズ
	vbView_.SizeInBytes = sizeof(VertexPosUv) * 4;
	//1頂点あたりのサイズ
	vbView_.StrideInBytes = sizeof(VertexPosUv);

	{

		//ヒープの設定
		D3D12_HEAP_PROPERTIES uploadHeapProperties{};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//UploadHeapを使う
		//リソースの設定
		D3D12_RESOURCE_DESC vertexResourceDesc{};
		//バッファリソース。テクスチャの場合はまた別の設定をする
		vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		vertexResourceDesc.Width = (sizeof(ConstBufferData) + 0xff) & ~0xff;
		//バッファの場合はこれらは1にする決まり
		vertexResourceDesc.Height = 1;
		vertexResourceDesc.DepthOrArraySize = 1;
		vertexResourceDesc.MipLevels = 1;
		vertexResourceDesc.SampleDesc.Count = 1;
		//バッファの場合はこれにする決まり
		vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		// 定数バッファ生成
		hr = sDevice->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
			&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&constBuff_));
		assert(SUCCEEDED(hr));

		// 定数バッファマッピング
		hr = constBuff_->Map(0, nullptr, (void**)&constMap);
		assert(SUCCEEDED(hr));

	}

	return true;

}

/// <summary>
/// テクスチャハンドルの設定
/// </summary>
/// <param name="textureHandle"></param>
void Sprite::SetTextureHandle(uint32_t textureHandle) {

	textureHandle_ = textureHandle;
	resourceDesc_ = TextureManager::GetInstance()->GetResourceDesc(textureHandle_);

}

/// <summary>
/// 座標の設定
/// </summary>
/// <param name="position">座標</param>
void Sprite::SetPosition(const DirectX::XMFLOAT2& position) {

	position_ = position;

	TransferVertices();

}

/// <summary>
/// 角度の設定
/// </summary>
/// <param name="rotation">角度</param>
void Sprite::SetRotation(float rotation) {

	rotation_ = rotation;

	TransferVertices();

}

/// <summary>
/// サイズの設定
/// </summary>
/// <param name="size">サイズ</param>
void Sprite::SetSize(const DirectX::XMFLOAT2& size) {

	size_ = size;

	TransferVertices();

}

/// <summary>
/// アンカーポイントの設定
/// </summary>
/// <param name="anchorpoint">アンカーポイント</param>
void Sprite::SetAnchorPoint(const DirectX::XMFLOAT2& anchorpoint) {

	anchorPoint_ = anchorpoint;

	TransferVertices();

}

/// <summary>
/// 左右反転の設定
/// </summary>
/// <param name="isFlipX">左右反転</param>
void Sprite::SetISFlipX(bool isFlipX) {

	isFlipX_ = isFlipX;

	TransferVertices();

}

/// <summary>
/// 上下反転の設定
/// </summary>
/// <param name="isFlipX">上下反転</param>
void Sprite::SetISFlipY(bool isFlipY) {

	isFlipY_ = isFlipY;

	TransferVertices();

}

/// <summary>
/// テクスチャ範囲設定
/// </summary>
/// <param name="texBase">テクスチャ左上座標</param>
/// <param name="texSize">テクスチャサイズ</param>
void Sprite::SetTextureRect(const DirectX::XMFLOAT2& texBase, const DirectX::XMFLOAT2& texSize) {

	texBase_ = texBase;
	texSize_ = texSize;

	TransferVertices();

}

/// <summary>
/// 描画
/// </summary>
void Sprite::Draw() {

	// ワールド行列の更新
	matWorld_ = XMMatrixIdentity();
	matWorld_ *= XMMatrixRotationZ(rotation_);
	matWorld_ *= XMMatrixTranslation(position_.x, position_.y, 0.0f);
	
	// 定数バッファにデータ転送
	constMap->color = color_;
	constMap->mat = matWorld_ * sMatProjection; // 行列の合成

	// 頂点バッファの設定
	sCommandList->IASetVertexBuffers(0, 1, &vbView_);

	// 頂点バッファビューをセット
	sCommandList->SetGraphicsRootConstantBufferView(1, constBuff_->GetGPUVirtualAddress());

	// シェーダーリソースビューをセット
	TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(sCommandList, 2, textureHandle_);
	// 描画コマンド
	sCommandList->DrawInstanced(4, 1, 0, 0);

}

/// <summary>
/// 頂点データ転送
/// </summary>
void Sprite::TransferVertices() {

	//HRESULT hr;

	enum {LB, LT, RB, RT};

	float left = (0.0f - anchorPoint_.x) * size_.x;
	float right = (1.0f - anchorPoint_.x) * size_.x;
	float top = (0.0f - anchorPoint_.y) * size_.y;
	float bottom = (1.0f - anchorPoint_.y) * size_.y;
	if (isFlipX_) {
		left = -left;
		right = -right;
	}

	if (isFlipY_) {
		top = -top;
		bottom = -bottom;
	}

	// 頂点データ
	VertexPosUv vertices[kVertNum];

	vertices[LB].pos = { left, bottom, 0.0f };
	vertices[LT].pos = { left, top, 0.0f };
	vertices[RB].pos = { right, bottom, 0.0f };
	vertices[RT].pos = { right, top, 0.0f };

	// テクスチャ情報取得
	{

		float tex_left = texBase_.x / resourceDesc_.Width;
		float tex_right = (texBase_.x + texSize_.x) / resourceDesc_.Width;
		float tex_top = texBase_.y / resourceDesc_.Height;
		float tex_bottom = (texBase_.y + texSize_.y) / resourceDesc_.Height;

		vertices[LB].uv = { tex_left, tex_bottom };
		vertices[LT].uv = { tex_left, tex_top };
		vertices[RB].uv = { tex_right, tex_bottom };
		vertices[RT].uv = { tex_right, tex_top };

	}
	
	// 頂点バッファへのデータ転送
	memcpy(vertMap, vertices, sizeof(vertices));

}

//CompileShader
IDxcBlob* Sprite::CompileShader(
	//CompilerするShanderファイルへのパス
	const std::wstring& filePath,
	//Compilenに使用するProfile
	const wchar_t* profile,
	//初期化で生成したものを3つ
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxCompiler,
	IDxcIncludeHandler* includeHandler)
{
	//htslファイルを読む
	//これからシェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));
	//hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	//読めなかった止める
	assert(SUCCEEDED(hr));
	//読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8; //UTF8の文字コードであることを通知

	//Compileする
	LPCWSTR arguments[] = {
		filePath.c_str(), //コンパイル対象のhlslファイル名
		L"-E",L"main",//エントリーポイントの指定,基本的にmain以外にはしない
		L"-T", profile, //ShaderProfileの設定
		L"-Zi", L"-Qembed_debug", //デバッグ用の情報を埋め込む
		L"-Od", //最適化を外しておく
		L"-Zpr", //メモリレイアウトは作成
	};
	//実際にShaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxCompiler->Compile(
		&shaderSourceBuffer,//読み込んだファイル
		arguments,//コンパイルオプション
		_countof(arguments),//コンパイルオプションの数
		includeHandler,//includeがふくまれた諸々
		IID_PPV_ARGS(&shaderResult)//コンパイル結果
	);
	//コンパイルエラーではなくdixが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));

	//警告・エラーがでていないか確認する
	//警告・エラーが出てたらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		assert(false);
	}

	//compile結果を受け取って返す
	//コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//成功したログを出す
	Log(ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));
	//もう使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();
	//実行用のバイナリを返却
	return shaderBlob;

}

//ログ
void Sprite::Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}

//コンバートストリング
std::wstring Sprite::ConvertString(const std::string& str) {
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

std::string Sprite::ConvertString(const std::wstring& str) {
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