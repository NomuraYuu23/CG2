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

enum Mode
{
	none = 0,
	colorChange = 1

};

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

	//Transform変数を作る(カメラ)
	TransformStructure cameraTransform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -10.0f} };

	/*

	//テクスチャ
	uint32_t textureHandle = TextureManager::Load("resources/uvChecker.png", dxCommon);

	// マテリアル
	TransformStructure uvTransform{
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f},
	};
	std::unique_ptr<Material> material;
	material.reset(
		Material::Create()
	);

	TransformStructure uvTransformSprite{
	{1.0f,1.0f,1.0f},
	{0.0f,0.0f,0.0f},
	{0.0f,0.0f,0.0f},
	};
	std::unique_ptr<Material> materialSprite;
	materialSprite.reset(
		Material::Create()
	);

	// スプライト
	//Transform変数を作る
	TransformStructure transformSprite{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	std::unique_ptr<Sprite> sprite;
	sprite.reset(
		Sprite::Create(
			textureHandle, transformSprite, materialSprite.get()));

	// モデル

	//Transform変数を作る
	TransformStructure transform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	std::unique_ptr<Model> model;
	model.reset(Model::Create("resources", "axis.obj", dxCommon, material.get()));

	//平行光源リソースを作る
	DirectionalLightData directionalLightData;
	directionalLightData.color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData.direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData.intencity = 1.0f;
	std::unique_ptr<DirectionalLight> directionalLight;
	directionalLight.reset(DirectionalLight::Create());

	*/

	//生存フラグ
	bool isAliveSprite = true;
	bool isAliveBall = false;
	bool isAliveTriangle = false;
	bool isAliveTriangle2 = false;


	// スプライト

	//テクスチャ
	uint32_t textureHandle = TextureManager::Load("resources/uvChecker.png", dxCommon);

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

	//Transform変数を作る
	TransformStructure transformTriangle2{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	std::unique_ptr<Model> modelTriangle2;
	modelTriangle2.reset(Model::Create("resources", "Triangle.obj", dxCommon, materialTriangle.get()));

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
		ImGui::End();

		//カメラ
		ImGui::Begin("Camera");
		ImGui::DragFloat3("translate", &cameraTransform.translate.x);
		ImGui::DragFloat3("rotate", &cameraTransform.rotate.x, 0.01f, -10.0f, 10.0f);
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
			ImGui::End();

			materialSprite->Update(uvTransformSprite, colorSprite, false);
			sprite->Update(transformSprite);

		}

		//モデル球
		if (isAliveBall) {

			ImGui::Begin("ModelBall");
			ImGui::DragFloat3("translate", &transformBall.translate.x, 0.1f);
			ImGui::DragFloat3("scale", &transformBall.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat3("rotate", &transformBall.rotate.x, 0.01f, -10.0f, 10.0f);
			ImGui::ColorEdit3("color", &colorBall.x);
			ImGui::End();

			materialBall->Update(uvTransformBall, colorBall, false);
			modelBall->Update(transformBall, cameraTransform);

		}

		//モデル三角
		if (isAliveTriangle) {

			ImGui::Begin("Triangle");
			ImGui::DragFloat3("translate", &transformTriangle.translate.x, 0.1f);
			ImGui::DragFloat3("scale", &transformTriangle.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat3("rotate", &transformTriangle.rotate.x, 0.01f, -10.0f, 10.0f);
			ImGui::ColorEdit3("color", &colorTriangle.x);
			ImGui::End();

			materialTriangle->Update(uvTransformTriangle, colorTriangle, false);
			modelTriangle->Update(transformTriangle, cameraTransform);

		}

		//モデル三角2
		if (isAliveTriangle) {
		
			ImGui::Begin("transformTriangle2");
			ImGui::DragFloat3("translate", &transformTriangle2.translate.x, 0.1f);
			ImGui::DragFloat3("scale", &transformTriangle2.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat3("rotate", &transformTriangle2.rotate.x, 0.01f, -10.0f, 10.0f);
			ImGui::ColorEdit3("color", &colorTriangle.x);
			ImGui::End();

			if (!isAliveTriangle) {
				materialTriangle->Update(uvTransformTriangle, colorTriangle, false);
			}
			modelTriangle2->Update(transformTriangle2, cameraTransform);
		
		}
		//開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
		ImGui::ShowDemoWindow();

		/*
		sprite->Update(transformSprite);

		model->Update(transform, cameraTransform);

		material->Update(uvTransform, Vector4(1.0f, 1.0f, 1.0f, 1.0f), true);
		materialSprite->Update(uvTransformSprite, Vector4(1.0f, 1.0f, 1.0f, 1.0f), false);

		directionalLight->Update(directionalLightData);

		*/


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
		//directionalLight->Draw(dxCommon->GetCommadList());

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

	//色々な解放処理の前に書く
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//ゲームウィンドウの破棄
	win->TerminateGameWindow();

	return 0;
}
