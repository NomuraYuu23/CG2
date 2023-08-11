#include "GameScene.h"

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

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();

}

/// <summary>
/// 更新処理
/// </summary>
void GameScene::Update(){}

/// <summary>
/// 描画処理
/// </summary>
void GameScene::Draw() {}
