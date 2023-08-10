#include "DebugCamera.h"
#include "Input.h"

/// <summary>
/// 初期化
/// </summary>
void DebugCamera::Initialize() {

	transform_ = { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -10.0f } };
	debugCameraUsed = false;

}

/// <summary>
/// 更新
/// </summary>
TransformStructure DebugCamera::Update(const TransformStructure& releaseCameratransform) {

	Input* input = Input::GetInstance();

	if (input->TriggerKey(DIK_0)) {
		if (debugCameraUsed) {
			debugCameraUsed = false;
		}
		else {
			debugCameraUsed = true;
		}
	}

	if (debugCameraUsed) {
		
		if (input->PushKey(DIK_D)) {
			transform_.translate.x += 0.1f;
		}
		if (input->PushKey(DIK_A)) {
			transform_.translate.x -= 0.1f;
		}
		if (input->PushKey(DIK_W)) {
			transform_.translate.y += 0.1f;
		}
		if (input->PushKey(DIK_S)) {
			transform_.translate.y -= 0.1f;
		}
		if (input->PushKey(DIK_E)) {
			transform_.translate.z += 0.1f;
		}
		if (input->PushKey(DIK_Q)) {
			transform_.translate.z -= 0.1f;
		}

		if (input->PushKey(DIK_RIGHT)) {
			transform_.rotate.y += 0.01f;
		}
		if (input->PushKey(DIK_LEFT)) {
			transform_.rotate.y -= 0.01f;
		}
		if (input->PushKey(DIK_DOWN)) {
			transform_.rotate.x += 0.01f;
		}
		if (input->PushKey(DIK_UP)) {
			transform_.rotate.x -= 0.01f;
		}

		return transform_;

	}

	return releaseCameratransform;

}
