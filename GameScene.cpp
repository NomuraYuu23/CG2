#include "GameScene.h"

#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include "WinApp.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "Sprite.h"
#include "Model.h"
#include "Material.h"
#include "DirectionalLight.h"
#include "D3DResourceLeakChecker.h"

/// <summary>
/// コンストラクタ
/// </summary>
GameScene::GameScene(){}

/// <summary>
/// デストラクタ
/// </summary>
GameScene::~GameScene(){}

/// <summary>
/// 初期化
/// </summary>
void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	viewProjection.Initialize();

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();

}

/// <summary>
/// 更新処理
/// </summary>
void GameScene::Update(){


}

/// <summary>
/// 描画処理
/// </summary>
void GameScene::Draw() {

	//ゲームの処理 

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(dxCommon_->GetCommadList());

	//背景スプライト描画

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();


#pragma endregion

	Model::PreDraw(dxCommon_->GetCommadList());

	//モデル

	Model::PostDraw();

#pragma region 前景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(dxCommon_->GetCommadList());

	//背景スプライト描画

	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion

}
