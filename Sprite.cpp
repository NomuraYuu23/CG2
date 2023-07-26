#include "Sprite.h"
#include "TextureManager.h"
#include <cassert>

using namespace DirectX;
using namespace Microsoft::WRL;

// �f�o�C�X
ID3D12Device* Sprite::sDevice = nullptr;
// �f�B�X�N���v�^�T�C�Y
UINT Sprite::sDescriptorHandleIncrementSize;
// �R�}���h���X�g
ID3D12GraphicsCommandList* Sprite::sCommandList = nullptr;
// ���[�g�V�O�l�`��
ComPtr<ID3D12RootSignature> Sprite::sRootSignature;
// �p�C�v���C���X�e�[�g�I�u�W�F�N�g
ComPtr<ID3D12PipelineState> Sprite::sPipelineState;
// �ˉe�s��
DirectX::XMMATRIX Sprite::sMatProjection;

/// <summary>
/// �ÓI������
/// </summary>
/// <param name="device">�f�o�C�X</param>
/// <param name="window_width">��ʕ�</param>
/// <param name="window_height">��ʍ���</param>
/// <param name="directoryPath">�f�B���N�g���p�X</param>
void Sprite::StaticInitialize(
	ID3D12Device* device, int window_width, int window_height,
	const std::wstring& directoryPath) {

	assert(device);
	
	sDevice = device;

	sDescriptorHandleIncrementSize = sDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	HRESULT hr;

	//dxcCompiler��������
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcompiler));
	assert(SUCCEEDED(hr));

	//�����_��include�͂��Ȃ����Ainclude�ɑΉ����邽�߂̐ݒ���s���Ă���
	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;//0����n�܂�
	descriptorRange[0].NumDescriptors = 1;//���͈��
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRV���g��
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//Offset�������v�Z

	//RootSignature�쐬
	D3D12_ROOT_SIGNATURE_DESC descriptionRootsignature{};
	descriptionRootsignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//RootParameter�쐬�B�����ݒ�ł���̂Ŕz��B�����1�����Ȃ̂Œ���1�̔z��
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   //CBV���g��
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShader�Ŏg��
	rootParameters[0].Descriptor.ShaderRegister = 0;                   //���W�X�^�ԍ�0�ƃo�C���h
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   //CBV���g��
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShader�Ŏg��
	rootParameters[1].Descriptor.ShaderRegister = 0;                   //���W�X�^�ԍ�0�ƃo�C���h
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTable���g��
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShader�Ŏg��
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Table�̒��g�̔z����w��
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//Table�ŗ��p���鐔
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   //CBV���g��
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//VertexShader�Ŏg��
	rootParameters[3].Descriptor.ShaderRegister = 1;
	descriptionRootsignature.pParameters = rootParameters;             //���[�g�p�����[�^�z��ւ̃|�C���^
	descriptionRootsignature.NumParameters = _countof(rootParameters); //�z��̒���

	//Sampler�̐ݒ�
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

	//DepthStencilState�̐ݒ�
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depth�̋@�\��L����
	depthStencilDesc.DepthEnable = true;
	//��������
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//��r�֐���LessEqual
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//�V���A���C�Y���ăo�C�i���ɂ���
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootsignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//�o�C�i�������ɐ���
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

	//BlendState�̐ݒ�
	D3D12_BLEND_DESC blendDesc{};
	//���ׂĂ̐F�v�f����������
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	//ResiterzerState�̐ݒ�
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//����(���v���)��\�����Ȃ�
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//�O�p�`�̒���h��Ԃ�
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shader���R���p�C������
	IDxcBlob* vertexShaderBlob = CompileShader(L"Object3d.VS.hlsl",
		L"vs_6_0", dxcUtils, dxcompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);


	IDxcBlob* pixelShaderBlob = CompileShader(L"Object3d.PS.hlsl",
		L"ps_6_0", dxcUtils, dxcompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);

	//PSO�𐶐�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = sRootSignature.Get();//RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;//InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };//PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;//BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
	//��������RTV�̏��
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//���p����g�|���W(�`��)�̃^�C�v�B�O�p�`
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//�ǂ̂悤�ɉ�ʂɐF��ł����ނ̂��̐ݒ�
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//DepthStencil�̐ݒ�
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//���ۂɐ���
	hr = sDevice->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&sPipelineState));
	assert(SUCCEEDED(hr));

	// �ˉe�s��v�Z
	sMatProjection = XMMatrixOrthographicOffCenterLH(
		0.0f, (float)window_width, (float)window_height, 0.0f, 0.0f, 1.0f
	);

}

/// <summary>
/// �ÓI�O����
/// </summary>
/// <param name="cmdList">�`��R�}���h���X�g</param>
void Sprite::PreDraw(ID3D12GraphicsCommandList* cmdList) {

	assert(Sprite::sCommandList == nullptr);

	sCommandList = cmdList;

	//RootSignature��ݒ�B
	sCommandList->SetPipelineState(sPipelineState.Get());//PS0��ݒ�
	sCommandList->SetGraphicsRootSignature(sRootSignature.Get());
	//�`���ݒ�BPS0�ɐݒ肵�Ă�����̂Ƃ͕ʁB
	sCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

/// <summary>
/// �`��㏈��
/// </summary>
void Sprite::PostDraw() {
	//�R�}���h���X�g������
	Sprite::sCommandList = nullptr;

}

/// <summary>
/// �X�v���C�g����
/// </summary>
/// <param name="textureHandle">�e�N�X�`���n���h��</param>
/// <param name="position">���W</param>
/// <param name="color">�F</param>
/// <param name="anchorpoint">�A���J�[�|�C���g</param>
/// <param name="isFlipX">���E���]</param>
/// <param name="isFlipY">�㉺���]</param>
/// <returns>�������ꂽ�X�v���C�g</returns>
Sprite* Sprite::Create(
	uint32_t textureHandle, DirectX::XMFLOAT2 position, DirectX::XMFLOAT4 color,
	DirectX::XMFLOAT2 anchorpoint, bool isFlipX, bool isFlipY) {

	// ���T�C�Y
	XMFLOAT2 size = { 100.0f, 100.0f };

	// �e�N�X�`�����擾
	const D3D12_RESOURCE_DESC& resDesc = TextureManager::GetInstance()->GetResourceDesc(textureHandle);
	// �X�v���C�g�̃T�C�Y���e�N�X�`���̃T�C�Y�ɐݒ�
	size = { (float)resDesc.Width, (float)resDesc.Height };

	// Sprite�̃C���X�^���X�𐶐�
	Sprite* sprite = new Sprite(textureHandle, position, size, color, anchorpoint, isFlipX, isFlipY);
	if (sprite == nullptr) {
		return nullptr;
	}

	// ������
	if (!sprite->Initialize()) {
		delete sprite;
		assert(0);
		return nullptr;
	}

	return sprite;

}

/// <summary>
/// �R���X�g���N�^
/// </summary>
Sprite::Sprite() {}

/// <summary>
/// �R���X�g���N�^
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
/// ������
/// </summary>
/// <returns>����</returns>
bool Sprite::Initialize() {

	assert(sDevice);

	HRESULT hr;

	resourceDesc_ = TextureManager::GetInstance()->GetResourceDesc(textureHandle_);
	
	{

	//���_���\�[�X�p�̃q�[�v�̐ݒ�
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//UploadHeap���g��
	//���_���\�[�X�̐ݒ�
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	//�o�b�t�@���\�[�X�B�e�N�X�`���̏ꍇ�͂܂��ʂ̐ݒ������
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeof(VertexPosUv) * kVertNum;//���\�[�X�̃T�C�Y�B�����Vector4��3���_��
	//�o�b�t�@�̏ꍇ�͂�����1�ɂ��錈�܂�
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	//�o�b�t�@�̏ꍇ�͂���ɂ��錈�܂�
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// ���_�o�b�t�@����
	hr = sDevice->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&vertBuff_));
	assert(SUCCEEDED(hr));

	// ���_�o�b�t�@�}�b�s���O
	hr = vertBuff_->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(hr));

	}

	// ���_�o�b�t�@�ւ̃f�[�^�]��
	TransferVertices();

	// ���_�o�b�t�@�r���[�̍쐬
	
	//���\�[�X�̐擪�̃A�h���X����g��
	vbView_.BufferLocation = vertBuff_->GetGPUVirtualAddress();
	//�g�p���郊�\�[�X�̃T�C�Y
	vbView_.SizeInBytes = sizeof(VertexPosUv) * 4;
	//1���_������̃T�C�Y
	vbView_.StrideInBytes = sizeof(VertexPosUv);

	{

		//�q�[�v�̐ݒ�
		D3D12_HEAP_PROPERTIES uploadHeapProperties{};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//UploadHeap���g��
		//���\�[�X�̐ݒ�
		D3D12_RESOURCE_DESC vertexResourceDesc{};
		//�o�b�t�@���\�[�X�B�e�N�X�`���̏ꍇ�͂܂��ʂ̐ݒ������
		vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		vertexResourceDesc.Width = (sizeof(ConstBufferData) + 0xff) & ~0xff;
		//�o�b�t�@�̏ꍇ�͂�����1�ɂ��錈�܂�
		vertexResourceDesc.Height = 1;
		vertexResourceDesc.DepthOrArraySize = 1;
		vertexResourceDesc.MipLevels = 1;
		vertexResourceDesc.SampleDesc.Count = 1;
		//�o�b�t�@�̏ꍇ�͂���ɂ��錈�܂�
		vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		// �萔�o�b�t�@����
		hr = sDevice->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
			&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&constBuff_));
		assert(SUCCEEDED(hr));

		// �萔�o�b�t�@�}�b�s���O
		hr = constBuff_->Map(0, nullptr, (void**)&constMap);
		assert(SUCCEEDED(hr));

	}

	return true;

}

/// <summary>
/// �e�N�X�`���n���h���̐ݒ�
/// </summary>
/// <param name="textureHandle"></param>
void Sprite::SetTextureHandle(uint32_t textureHandle) {

	textureHandle_ = textureHandle;
	resourceDesc_ = TextureManager::GetInstance()->GetResourceDesc(textureHandle_);

}

/// <summary>
/// ���W�̐ݒ�
/// </summary>
/// <param name="position">���W</param>
void Sprite::SetPosition(const DirectX::XMFLOAT2& position) {

	position_ = position;

	TransferVertices();

}

/// <summary>
/// �p�x�̐ݒ�
/// </summary>
/// <param name="rotation">�p�x</param>
void Sprite::SetRotation(float rotation) {

	rotation_ = rotation;

	TransferVertices();

}

/// <summary>
/// �T�C�Y�̐ݒ�
/// </summary>
/// <param name="size">�T�C�Y</param>
void Sprite::SetSize(const DirectX::XMFLOAT2& size) {

	size_ = size;

	TransferVertices();

}

/// <summary>
/// �A���J�[�|�C���g�̐ݒ�
/// </summary>
/// <param name="anchorpoint">�A���J�[�|�C���g</param>
void Sprite::SetAnchorPoint(const DirectX::XMFLOAT2& anchorpoint) {

	anchorPoint_ = anchorpoint;

	TransferVertices();

}

/// <summary>
/// ���E���]�̐ݒ�
/// </summary>
/// <param name="isFlipX">���E���]</param>
void Sprite::SetISFlipX(bool isFlipX) {

	isFlipX_ = isFlipX;

	TransferVertices();

}

/// <summary>
/// �㉺���]�̐ݒ�
/// </summary>
/// <param name="isFlipX">�㉺���]</param>
void Sprite::SetISFlipY(bool isFlipY) {

	isFlipY_ = isFlipY;

	TransferVertices();

}

/// <summary>
/// �e�N�X�`���͈͐ݒ�
/// </summary>
/// <param name="texBase">�e�N�X�`��������W</param>
/// <param name="texSize">�e�N�X�`���T�C�Y</param>
void Sprite::SetTextureRect(const DirectX::XMFLOAT2& texBase, const DirectX::XMFLOAT2& texSize) {

	texBase_ = texBase;
	texSize_ = texSize;

	TransferVertices();

}

/// <summary>
/// �`��
/// </summary>
void Sprite::Draw() {

	// ���[���h�s��̍X�V
	matWorld_ = XMMatrixIdentity();
	matWorld_ *= XMMatrixRotationZ(rotation_);
	matWorld_ *= XMMatrixTranslation(position_.x, position_.y, 0.0f);
	
	// �萔�o�b�t�@�Ƀf�[�^�]��
	constMap->color = color_;
	constMap->mat = matWorld_ * sMatProjection; // �s��̍���

	// ���_�o�b�t�@�̐ݒ�
	sCommandList->IASetVertexBuffers(0, 1, &vbView_);

	// ���_�o�b�t�@�r���[���Z�b�g
	sCommandList->SetGraphicsRootConstantBufferView(0, constBuff_->GetGPUVirtualAddress());
	// �V�F�[�_�[���\�[�X�r���[���Z�b�g
	TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(sCommandList, 1, textureHandle_);
	// �`��R�}���h
	sCommandList->DrawInstanced(4, 1, 0, 0);

}

/// <summary>
/// ���_�f�[�^�]��
/// </summary>
void Sprite::TransferVertices() {

	HRESULT hr;

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

	// ���_�f�[�^
	VertexPosUv vertices[kVertNum];

	vertices[LB].pos = { left, bottom, 0.0f };
	vertices[LT].pos = { left, top, 0.0f };
	vertices[RB].pos = { right, bottom, 0.0f };
	vertices[RT].pos = { right, top, 0.0f };

	// �e�N�X�`�����擾
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
	
	// ���_�o�b�t�@�ւ̃f�[�^�]��
	memcpy(vertMap, vertices, sizeof(vertices));

}

//CompileShader
IDxcBlob* Sprite::CompileShader(
	//Compiler����Shander�t�@�C���ւ̃p�X
	const std::wstring& filePath,
	//Compilen�Ɏg�p����Profile
	const wchar_t* profile,
	//�������Ő����������̂�3��
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxCompiler,
	IDxcIncludeHandler* includeHandler)
{
	//htsl�t�@�C����ǂ�
	//���ꂩ��V�F�[�_�[���R���p�C������|�����O�ɏo��
	Log(ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));
	//hlsl�t�@�C����ǂ�
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	//�ǂ߂Ȃ������~�߂�
	assert(SUCCEEDED(hr));
	//�ǂݍ��񂾃t�@�C���̓��e��ݒ肷��
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8; //UTF8�̕����R�[�h�ł��邱�Ƃ�ʒm

	//Compile����
	LPCWSTR arguments[] = {
		filePath.c_str(), //�R���p�C���Ώۂ�hlsl�t�@�C����
		L"-E",L"main",//�G���g���[�|�C���g�̎w��,��{�I��main�ȊO�ɂ͂��Ȃ�
		L"-T", profile, //ShaderProfile�̐ݒ�
		L"-Zi", L"-Qembed_debug", //�f�o�b�O�p�̏��𖄂ߍ���
		L"-Od", //�œK�����O���Ă���
		L"-Zpr", //���������C�A�E�g�͍쐬
	};
	//���ۂ�Shader���R���p�C������
	IDxcResult* shaderResult = nullptr;
	hr = dxCompiler->Compile(
		&shaderSourceBuffer,//�ǂݍ��񂾃t�@�C��
		arguments,//�R���p�C���I�v�V����
		_countof(arguments),//�R���p�C���I�v�V�����̐�
		includeHandler,//include���ӂ��܂ꂽ���X
		IID_PPV_ARGS(&shaderResult)//�R���p�C������
	);
	//�R���p�C���G���[�ł͂Ȃ�dix���N���ł��Ȃ��Ȃǒv���I�ȏ�
	assert(SUCCEEDED(hr));

	//�x���E�G���[���łĂ��Ȃ����m�F����
	//�x���E�G���[���o�Ă��烍�O�ɏo���Ď~�߂�
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		assert(false);
	}

	//compile���ʂ��󂯎���ĕԂ�
	//�R���p�C�����ʂ�����s�p�̃o�C�i���������擾
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//�����������O���o��
	Log(ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));
	//�����g��Ȃ����\�[�X�����
	shaderSource->Release();
	shaderResult->Release();
	//���s�p�̃o�C�i����ԋp
	return shaderBlob;

}

//���O
void Sprite::Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}

//�R���o�[�g�X�g�����O
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