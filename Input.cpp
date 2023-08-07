#include "Input.h"
#include <cassert>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

/// <summary>
/// 初期化
/// </summary>
/// <param name="hInstance"></param>
/// <param name="hwnd"></param>
void Input::Initialize(HINSTANCE hInstance, HWND hwnd) {

	// DirectInputの初期化
	directInput_ = nullptr;
	HRESULT result = DirectInput8Create(
		hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput_, nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイスの生成
	IDirectInputDevice8* keyboard = nullptr;
	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	// 入力デ―タ形式のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);//標準形式
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result = keyboard->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

}

/// <summary>
/// 毎フレーム
/// </summary>
void Input::Update() {

	//キーボード関連更新
	KeyboardUpdata();

}

/// <summary>
/// キーボード関連更新
/// </summary>
void Input::KeyboardUpdata() {

	//キーボード動作開始
	keyboard_->Acquire();

	//前回のキー入力を保存
	keyPre_ = key_;

	// 全キーの入力状態を取得する
	keyboard_->GetDeviceState((DWORD)size(key_), key_.data());


}

/// <summary>
/// キーの押下をチェック
/// </summary>
/// <param name="keyNumber">キー番号</param>
/// <returns>押されているか</returns>
bool Input::PushKey(uint8_t keyNumber) {

	//0でなければ押している
	if (key_[keyNumber]) {
		return true;
	}

	//押していない
	return false;

}

/// <summary>
/// キーのトリガーをチェック
/// </summary>
/// <param name="keyNumber">キー番号</param>
/// <returns>トリガーか</returns>
bool Input::TriggerKey(uint8_t keyNumber) {

	//前回が0で、今回が0でなければトリガー
	if (!keyPre_[keyNumber] && key_[keyNumber]) {
		return true;
	}

	// トリガーじゃない
	return false;

}