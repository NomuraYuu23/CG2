#include "Input.h"
#include <cassert>

#pragma comment(lib, "dinput8.lib")

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
	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(result));

	// 入力デ―タ形式のセット
	result = keyboard_->SetDataFormat(&c_dfDIKeyboard);//標準形式
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result = keyboard_->SetCooperativeLevel(
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
/// キーを押した状態か
/// </summary>
/// <param name="keyNumber">キー番号</param>
/// <returns>キーを押した状態か</returns>
bool Input::PushKey(uint8_t keyNumber) {

	//0でなければ押している
	if (key_[keyNumber]) {
		return true;
	}

	//押していない
	return false;

}

/// <summary>
/// キーを離した状態か
/// </summary>
/// <param name="keyNumber">キー番号</param>
/// <returns>キーを離した状態か</returns>
bool Input::NoPushKey(uint8_t keyNumber) {

	//0でなければ押している
	if (key_[keyNumber]) {
		return false;
	}

	//押していない
	return true;


}

/// <summary>
/// キーを押した瞬間か
/// </summary>
/// <param name="keyNumber">キー番号</param>
/// <returns>キーを押した瞬間か</returns>
bool Input::TriggerKey(uint8_t keyNumber) {

	//前回が0で、今回が0でなければtrue
	if (!keyPre_[keyNumber] && key_[keyNumber]) {
		return true;
	}

	// false
	return false;

}

/// <summary>
/// キーを離した瞬間か
/// </summary>
/// <param name="keyNumber">キー番号</param>
/// <returns>キーを離した瞬間か</returns>
bool Input::ReleaseKey(uint8_t keyNumber) {

	//前回が0でなく、今回が0でならtrue
	if (keyPre_[keyNumber] && !key_[keyNumber]) {
		return true;
	}

	// false
	return false;

}