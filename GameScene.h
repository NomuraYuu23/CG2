#pragma once
#include "DirectXCommon.h"
#include "Audio.h"// サウンド再生
#include "Input.h"//入力デバイス
#include "DebugCamera.h"//デバッグカメラ

#include "WorldTransform.h"

class GameScene
{

public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	GameScene();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameScene();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

private:

	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;
	Audio* audio_ = nullptr;

	//デバッグカメラ
	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;

};
