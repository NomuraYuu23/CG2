#include <Windows.h>

#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "VertexData.h"
#include "TransformationMatrix.h"
#include "TransformStructure.h"
#include "MaterialData.h"
#include "DirectionalLightData.h"

//クラス化
#include "WinApp.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "Sprite.h"
#include "Model.h"
#include "Material.h"
#include "DirectionalLight.h"
#include "D3DResourceLeakChecker.h"

// サウンド再生
#include <xaudio2.h>
#pragma comment(lib, "xaudio2.lib")
#include <fstream>
#include <wrl.h>
#include <cassert>

// チャンクヘッダ
struct ChunkHeader
{
	char id[4];// チャンク毎のID
	int32_t size; //チャンクサイズ
};

// RIFFヘッダチャンク
struct RiffHeader
{
	ChunkHeader chunk; // "RIFF"
	char type[4]; // "WAVE"
}; 

// FMTチャンク
struct FormatChunk {
	ChunkHeader chunk; // "fmt"
	WAVEFORMATEX fmt; // 波形フォーマット
};

// 音声データ
struct SoundData
{
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	BYTE* pBuffer;
	// バッファのサイズ
	unsigned int bufferSize;
};

// 音声データ読み込み
SoundData SoundLoadWave(const char* filename) {
	
	HRESULT result = 0;
	
	// ファイルオープン
	 
	// ファイル入力ストリームのインスタンス
	std::ifstream file;
	// .wavファイルをバイナリモードで開く
	file.open(filename, std::ios_base::binary);
	// ファイルオープン失敗を検出する
	assert(file.is_open());
	 
	// .wavデータ読み込み

	// RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFがチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	// タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	// Formatチャンクの読み込み
	FormatChunk format = {};
	// チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt", 4) != 0) {
		assert(0);
	}

	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	// Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	// JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		// 読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		// 再読み込み
		file.read((char*)&data, sizeof(data));
	}
	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	// Dataチャンクのデータ部(波形データ)の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// ファイルクローズ
	
	//Waveファイルを閉じる
	file.close();

	// 読み込んだ音声データをreturn

	// returnする為の音声データ
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;

}

// 音声データ解放
void SoundUnload(SoundData* soundData) {
	// バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};

}

// 音声再生
void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData) {

	HRESULT result;

	// 波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();

}

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	WinApp* win = nullptr;
	DirectXCommon* dxCommon = nullptr;

	//ゲームウィンドウの作成
	win = WinApp::GetInstance();
	win->CreateGameWindow();

	//DirectX初期化
	dxCommon = DirectXCommon::GetInstance();
	dxCommon->Initialize(win);

	//テクスチャマネージャー
	TextureManager::GetInstance()->Initialize(dxCommon->GetDevice());

	// スプライト静的初期化
	Sprite::StaticInitialize(dxCommon->GetDevice());

	// モデル静的初期化
	Model::StaticInitialize(dxCommon->GetDevice());

	// マテリアル静的初期化
	Material::StaticInitialize(dxCommon->GetDevice());
	
	// 光源静的初期化
	DirectionalLight::StaticInitialize(dxCommon->GetDevice());

	// リリースチェッカー
	D3DResourceLeakChecker leakChecker;

	//ImGuiの初期化。

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(win->GetHwnd());
	ImGui_ImplDX12_Init(dxCommon->GetDevice(),
		2,								 // ダブルバッファ
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, // SRGB
		TextureManager::StaticGetDescriptorHeap(),
		TextureManager::StaticGetCPUDescriptorHandle(),
		TextureManager::StaticGetGPUDescriptorHandle());


	// サウンド再生
	HRESULT result;
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;

	// XAudioのエンジンのインスタンスを生成
	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	// マスターボイスを生成
	result = xAudio2->CreateMasteringVoice(&masterVoice);

	// 音声読み込み
	SoundData soundData1 = SoundLoadWave("Resources/Alarm01.wav");

	//Transform変数を作る(カメラ)
	TransformStructure cameraTransform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -10.0f} };

	//生存フラグ
	bool isAliveSprite = true;
	bool isAliveBall = false;
	bool isAliveTriangle = false;
	bool isAliveTriangle2 = false;
	bool isAliveTeapot = false;
	bool isAliveBunny = false;
	bool isAliveMultiMesh = false;

	// スプライト

	//テクスチャ
	uint32_t textureHandle = TextureManager::Load("resources/uvChecker.png", dxCommon);
	uint32_t textureHandle2 = TextureManager::Load("resources/monsterBall.png", dxCommon);

	bool isUvChecker = true;

	TransformStructure uvTransformSprite{
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f},
	};
	std::unique_ptr<Material> materialSprite;
	materialSprite.reset(
		Material::Create()
	);

	//Transform変数を作る
	TransformStructure transformSprite{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	std::unique_ptr<Sprite> sprite;
	sprite.reset(
		Sprite::Create(
			textureHandle, transformSprite, materialSprite.get()));

	Vector4 colorSprite = { 1.0f,1.0f,1.0f,1.0f };

	// モデル球

	// マテリアル
	TransformStructure uvTransformBall{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};
	std::unique_ptr<Material> materialBall;
	materialBall.reset(
		Material::Create()
	);

	//Transform変数を作る
	TransformStructure transformBall{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	std::unique_ptr<Model> modelBall;
	modelBall.reset(Model::Create("resources", "Ball.obj", dxCommon, materialBall.get()));

	Vector4 colorBall = { 1.0f,1.0f,1.0f,1.0f };
	int enableLightingBall = HalfLambert;

	// モデル三角形

	// マテリアル
	TransformStructure uvTransformTriangle{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};
	std::unique_ptr<Material> materialTriangle;
	materialTriangle.reset(
		Material::Create()
	);

	//Transform変数を作る
	TransformStructure transformTriangle{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	std::unique_ptr<Model> modelTriangle;
	modelTriangle.reset(Model::Create("resources", "Triangle.obj", dxCommon, materialTriangle.get()));

	Vector4 colorTriangle = { 1.0f,1.0f,1.0f,1.0f };
	int enableLightingTriangle = None;

	//Transform変数を作る
	TransformStructure transformTriangle2{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	std::unique_ptr<Model> modelTriangle2;
	modelTriangle2.reset(Model::Create("resources", "Triangle.obj", dxCommon, materialTriangle.get()));

	//ティーポット

	// マテリアル
	TransformStructure uvTransformTeapot{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};
	std::unique_ptr<Material> materialTeapot;
	materialTeapot.reset(
		Material::Create()
	);

	//Transform変数を作る
	TransformStructure transformTeapot{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	std::unique_ptr<Model> modelTeapot;
	modelTeapot.reset(Model::Create("resources", "teapot.obj", dxCommon, materialTeapot.get()));

	Vector4 colorTeapot = { 1.0f,1.0f,1.0f,1.0f };
	int enableLightingTeapot = HalfLambert;


	//バニー

	// マテリアル
	TransformStructure uvTransformBunny{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};
	std::unique_ptr<Material> materialBunny;
	materialBunny.reset(
		Material::Create()
	);

	//Transform変数を作る
	TransformStructure transformBunny{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	std::unique_ptr<Model> modelBunny;
	modelBunny.reset(Model::Create("resources", "bunny.obj", dxCommon, materialBunny.get()));

	Vector4 colorBunny = { 1.0f,1.0f,1.0f,1.0f };
	int enableLightingBunny = HalfLambert;

	//マルチメッシュ

	// マテリアル
	TransformStructure uvTransformMultiMesh{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};
	std::unique_ptr<Material> materialMultiMesh;
	materialMultiMesh.reset(
		Material::Create()
	);

	//Transform変数を作る
	TransformStructure transformMultiMesh{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	std::unique_ptr<Model> modelMultiMesh;
	modelMultiMesh.reset(Model::Create("resources", "multiMesh.obj", dxCommon, materialMultiMesh.get()));

	Vector4 colorMultiMesh = { 1.0f,1.0f,1.0f,1.0f };
	int enableLightingMultiMesh = HalfLambert;

	//平行光源リソースを作る
	DirectionalLightData directionalLightData;
	directionalLightData.color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData.direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData.intencity = 1.0f;
	std::unique_ptr<DirectionalLight> directionalLight;
	directionalLight.reset(DirectionalLight::Create());

	//サウンド
	SoundPlayWave(xAudio2.Get(), soundData1);

	//ウィンドウののボタンが押されるまでループ
	while (true) {
		//Windowにメッセージが来てたら最優先で処理させる
		if (win->ProcessMessage()) {
			break;
		}


		//ゲームの処理 
		//ImGui
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//生存フラグ
		ImGui::Begin("IsAlive");
		ImGui::Checkbox("Sprie", &isAliveSprite);
		ImGui::Checkbox("Ball", &isAliveBall);
		ImGui::Checkbox("Triangle", &isAliveTriangle);
		ImGui::Checkbox("Triangle2", &isAliveTriangle2);
		ImGui::Checkbox("Teapot", &isAliveTeapot);
		ImGui::Checkbox("Bunny", &isAliveBunny);
		ImGui::Checkbox("MultiMesh", &isAliveMultiMesh);
		ImGui::End();

		//カメラ
		ImGui::Begin("Camera");
		ImGui::DragFloat3("translate", &cameraTransform.translate.x, 0.01f);
		ImGui::DragFloat3("rotate", &cameraTransform.rotate.x, 0.01f, -10.0f, 10.0f);
		ImGui::End();

		//光源
		ImGui::Begin("directionalLight");
		ImGui::ColorEdit3("color", &directionalLightData.color.x);
		ImGui::DragFloat3("direction", &directionalLightData.direction.x, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat("intencity", &directionalLightData.intencity, 0.01f);
		ImGui::End();

		//スプライト
		if (isAliveSprite) {

			ImGui::Begin("Sprite");
			ImGui::DragFloat2("translate", &transformSprite.translate.x);
			ImGui::DragFloat2("scale", &transformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("rotate", &transformSprite.rotate.z);
			ImGui::DragFloat2("uvTranslate", &uvTransformSprite.translate.x, 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat2("uvScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("uvRotate", &uvTransformSprite.rotate.z);
			ImGui::ColorEdit3("color", &colorSprite.x);
			ImGui::Checkbox("uvChacker", &isUvChecker);
			ImGui::End();

			if (isUvChecker) {
				sprite->SetTextureHandle(textureHandle);
			}
			else {
				sprite->SetTextureHandle(textureHandle2);
			}

			materialSprite->Update(uvTransformSprite, colorSprite, None);
			sprite->Update(transformSprite);

		}

		//モデル球
		if (isAliveBall) {

			ImGui::Begin("ModelBall");
			ImGui::DragFloat3("translate", &transformBall.translate.x, 0.1f);
			ImGui::DragFloat3("scale", &transformBall.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat3("rotate", &transformBall.rotate.x, 0.01f, -10.0f, 10.0f);
			ImGui::ColorEdit3("color", &colorBall.x);
			ImGui::Text("enableLighting");
			ImGui::RadioButton("None", &enableLightingBall, None);
			ImGui::SameLine();
			ImGui::RadioButton("Lambert", &enableLightingBall, Lambert);
			ImGui::SameLine();
			ImGui::RadioButton("HalfLambert", &enableLightingBall, HalfLambert);
			ImGui::End();

			materialBall->Update(uvTransformBall, colorBall, enableLightingBall);
			modelBall->Update(transformBall, cameraTransform);

		}

		//モデル三角
		if (isAliveTriangle) {

			ImGui::Begin("Triangle");
			ImGui::DragFloat3("translate", &transformTriangle.translate.x, 0.1f);
			ImGui::DragFloat3("scale", &transformTriangle.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat3("rotate", &transformTriangle.rotate.x, 0.01f, -10.0f, 10.0f);
			ImGui::ColorEdit3("color", &colorTriangle.x);
			ImGui::Text("enableLighting");
			ImGui::RadioButton("None", &enableLightingTriangle, None);
			ImGui::SameLine();
			ImGui::RadioButton("Lambert", &enableLightingTriangle, Lambert);
			ImGui::SameLine();
			ImGui::RadioButton("HalfLambert", &enableLightingTriangle, HalfLambert);
			ImGui::End();

			materialTriangle->Update(uvTransformTriangle, colorTriangle, enableLightingTriangle);
			modelTriangle->Update(transformTriangle, cameraTransform);

		}

		//モデル三角2
		if (isAliveTriangle2) {
		
			ImGui::Begin("transformTriangle2");
			ImGui::DragFloat3("translate", &transformTriangle2.translate.x, 0.1f);
			ImGui::DragFloat3("scale", &transformTriangle2.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat3("rotate", &transformTriangle2.rotate.x, 0.01f, -10.0f, 10.0f);
			ImGui::ColorEdit3("color", &colorTriangle.x);
			ImGui::Text("enableLighting");
			ImGui::RadioButton("None", &enableLightingTriangle, None);
			ImGui::SameLine();
			ImGui::RadioButton("Lambert", &enableLightingTriangle, Lambert);
			ImGui::SameLine();
			ImGui::RadioButton("HalfLambert", &enableLightingTriangle, HalfLambert);
			ImGui::End();

			if (!isAliveTriangle) {
				materialTriangle->Update(uvTransformTriangle, colorTriangle, enableLightingTriangle);
			}
			modelTriangle2->Update(transformTriangle2, cameraTransform);
		
		}

		//ティーポット
		if (isAliveTeapot) {

			ImGui::Begin("ModelTeapot");
			ImGui::DragFloat3("translate", &transformTeapot.translate.x, 0.1f);
			ImGui::DragFloat3("scale", &transformTeapot.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat3("rotate", &transformTeapot.rotate.x, 0.01f, -10.0f, 10.0f);
			ImGui::ColorEdit3("color", &colorTeapot.x);
			ImGui::Text("enableLighting");
			ImGui::RadioButton("None", &enableLightingTeapot, None);
			ImGui::SameLine();
			ImGui::RadioButton("Lambert", &enableLightingTeapot, Lambert);
			ImGui::SameLine();
			ImGui::RadioButton("HalfLambert", &enableLightingTeapot, HalfLambert);
			ImGui::End();

			materialTeapot->Update(uvTransformTeapot, colorTeapot, enableLightingTeapot);
			modelTeapot->Update(transformTeapot, cameraTransform);

		}

		//バニー
		if (isAliveBunny) {

			ImGui::Begin("ModelBunny");
			ImGui::DragFloat3("translate", &transformBunny.translate.x, 0.1f);
			ImGui::DragFloat3("scale", &transformBunny.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat3("rotate", &transformBunny.rotate.x, 0.01f, -10.0f, 10.0f);
			ImGui::ColorEdit3("color", &colorBunny.x);
			ImGui::Text("enableLighting");
			ImGui::RadioButton("None", &enableLightingBunny, None);
			ImGui::SameLine();
			ImGui::RadioButton("Lambert", &enableLightingBunny, Lambert);
			ImGui::SameLine();
			ImGui::RadioButton("HalfLambert", &enableLightingBunny, HalfLambert);
			ImGui::End();

			materialBunny->Update(uvTransformBunny, colorBunny, enableLightingBunny);
			modelBunny->Update(transformBunny, cameraTransform);

		}

		//バニー
		if (isAliveMultiMesh) {

			ImGui::Begin("ModelMultiMesh");
			ImGui::DragFloat3("translate", &transformMultiMesh.translate.x, 0.1f);
			ImGui::DragFloat3("scale", &transformMultiMesh.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat3("rotate", &transformMultiMesh.rotate.x, 0.01f, -10.0f, 10.0f);
			ImGui::ColorEdit3("color", &colorMultiMesh.x);
			ImGui::Text("enableLighting");
			ImGui::RadioButton("None", &enableLightingMultiMesh, None);
			ImGui::SameLine();
			ImGui::RadioButton("Lambert", &enableLightingMultiMesh, Lambert);
			ImGui::SameLine();
			ImGui::RadioButton("HalfLambert", &enableLightingMultiMesh, HalfLambert);
			ImGui::End();

			materialMultiMesh->Update(uvTransformMultiMesh, colorMultiMesh, enableLightingMultiMesh);
			modelMultiMesh->Update(transformMultiMesh, cameraTransform);

		}

		//光源
		directionalLight->Update(directionalLightData);

		//開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
		ImGui::ShowDemoWindow();

		//ImGuiの内部コマンドを生成する
		ImGui::Render();

		//描画前処理
		dxCommon->PreDraw();

#pragma region 背景スプライト描画
		// 背景スプライト描画前処理
		Sprite::PreDraw(dxCommon->GetCommadList());

		//背景スプライト描画
		if (isAliveSprite) {
			sprite->Draw();
		}

		// スプライト描画後処理
		Sprite::PostDraw();
		// 深度バッファクリア
		dxCommon->ClearDepthBuffer();


#pragma endregion

		//光源
		directionalLight->Draw(dxCommon->GetCommadList());

		Model::PreDraw(dxCommon->GetCommadList());

		//モデル
		if (isAliveBall) {
			modelBall->Draw();
		}
		if (isAliveTriangle) {
			modelTriangle->Draw();
		}
		if (isAliveTriangle2) {
			modelTriangle2->Draw();
		}
		if (isAliveTeapot) {
			modelTeapot->Draw();
		}
		if (isAliveBunny) {
			modelBunny->Draw();
		}
		if (isAliveMultiMesh) {
			modelMultiMesh->Draw();
		}

		Model::PostDraw();

#pragma region 前景スプライト描画
	// 背景スプライト描画前処理
		Sprite::PreDraw(dxCommon->GetCommadList());

		//背景スプライト描画
		//sprite->Draw();

		// スプライト描画後処理
		Sprite::PostDraw();

#pragma endregion


		// シェーダーリソースビューをセット
		TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(dxCommon->GetCommadList(), 2, 0);
		//実際のcommandListのImGuiの描画コマンドを積む
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommadList());


		//描画後処理
		dxCommon->PostDraw();

	}

	//出力ウインドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	// サウンド後始末

	// XAudio2解放
	xAudio2.Reset();
	// 音声データ解放
	SoundUnload(&soundData1);

	//色々な解放処理の前に書く
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//ゲームウィンドウの破棄
	win->TerminateGameWindow();

	return 0;
}
