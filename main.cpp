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
#include "SafeDelete.h"

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
#include "Audio.h"

//入力デバイス
#include "Input.h"

//デバッグカメラ
#include "DebugCamera.h"
#include "GameScene.h"

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	WinApp* win = nullptr;
	DirectXCommon* dxCommon = nullptr;

	Audio* audio = nullptr;
	Input* input = nullptr;

	//ゲームウィンドウの作成
	win = WinApp::GetInstance();
	win->CreateGameWindow();

	//DirectX初期化
	dxCommon = DirectXCommon::GetInstance();
	dxCommon->Initialize(win);

	//入力デバイス
	input = Input::GetInstance();
	input->Initialize(win->GetHInstance(), win->GetHwnd());

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

	//サウンド
	audio = Audio::GetInstance();
	audio->Initialize();

	// リリースチェッカー
	D3DResourceLeakChecker leakChecker;

	//ゲームシーン
	std::unique_ptr<GameScene> gameScene = std::make_unique<GameScene>();
	gameScene->Initialize();

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

	//ウィンドウののボタンが押されるまでループ
	while (true) {
		//Windowにメッセージが来てたら最優先で処理させる
		if (win->ProcessMessage()) {
			break;
		}

		//入力デバイス
		input->Update();
		if (input->PushKey(DIK_SPACE)) {
			input->JoystickConnected(win->GetHwnd());
		}

		//ゲームの処理 
		//ImGui
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		//開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
		ImGui::ShowDemoWindow();

		//ImGuiの内部コマンドを生成する
		ImGui::Render();

		// ゲームシーン更新
		gameScene->Update();


		//描画前処理
		dxCommon->PreDraw();

		//ゲームシーン描画処理
		gameScene->Draw();

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
	audio->Finalize();

	//色々な解放処理の前に書く
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//ゲームウィンドウの破棄
	win->TerminateGameWindow();

	return 0;
}
