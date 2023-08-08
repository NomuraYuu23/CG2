#include "Input.h"
#include <cassert>

#pragma comment(lib, "dinput8.lib")

//インスタンス
Input* Input::GetInstance() {

	static Input instance;

	return &instance;

}

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
	result = directInput_->CreateDevice(GUID_SysKeyboard, &directKeyboard_, NULL);
	assert(SUCCEEDED(result));

	// 入力デ―タ形式のセット
	result = directKeyboard_->SetDataFormat(&c_dfDIKeyboard);//標準形式
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result = directKeyboard_->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	// マウスデバイスの生成
	result = directInput_->CreateDevice(GUID_SysMouse, &directMouse_, NULL);
	assert(SUCCEEDED(result));

	// 入力デ―タ形式のセット
	result = directMouse_->SetDataFormat(&c_dfDIMouse2);//標準形式
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result = directKeyboard_->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));

}

/// <summary>
/// 毎フレーム
/// </summary>
void Input::Update() {

	//キーボード関連更新
	KeyboardUpdate();

	//マウス関連更新
	MouseUpdate();

}

/// <summary>
/// キーボード関連更新
/// </summary>
void Input::KeyboardUpdate() {

	//キーボード動作開始
	directKeyboard_->Acquire();

	//前回のキー入力を保存
	keyPre_ = key_;

	// 全キーの入力状態を取得する
	directKeyboard_->GetDeviceState((DWORD)size(key_), key_.data());


}

/// <summary>
/// キーを押した状態か
/// </summary>
/// <param name="keyNumber">キー番号</param>
/// <returns>キーを押した状態か</returns>
bool Input::PushKey(uint8_t keyNumber) const {

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
bool Input::NoPushKey(uint8_t keyNumber) const {

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
bool Input::TriggerKey(uint8_t keyNumber) const {

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
bool Input::ReleaseKey(uint8_t keyNumber) const {

	//前回が0でなく、今回が0でならtrue
	if (keyPre_[keyNumber] && !key_[keyNumber]) {
		return true;
	}

	// false
	return false;

}


/// <summary>
/// マウス関連更新
/// </summary>
void Input::MouseUpdate() {

	// マウス動作開始
	directMouse_->Acquire();

	//前回のマウス入力を保存
	mousePre_ = mouse_;

	// マウスの入力状態を取得する
	directMouse_->GetDeviceState(sizeof(mouse_), &mouse_);

}

/// <summary>
/// マウスボタンを押した状態か
/// </summary>
/// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
/// <returns>押されているか</returns>
bool Input::PushMouse(uint8_t mouseNumber) const {

	// クリックされているか
	if (mouse_.rgbButtons[mouseNumber]) {
		return true;
	}

	return false;

}


/// <summary>
/// マウスボタンを離した状態か
/// </summary>
/// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
/// <returns>マウスボタンを離した状態か</returns>
bool Input::NoPushMouse(uint8_t mouseNumber) const {

	// クリックされているか
	if (mouse_.rgbButtons[mouseNumber]) {
		return false;
	}

	return true;

}

/// <summary>
/// マウスのトリガーをチェック。押した瞬間だけtrueになる
/// </summary>
/// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
/// <returns>トリガーか</returns>
bool Input::TriggerMouse(uint8_t mouseNumber) const {

	//クリックした瞬間
	if (mouse_.rgbButtons[mouseNumber] && !mousePre_.rgbButtons[mouseNumber]) {
		return true;
	}

	return false;

}

/// <summary>
/// マウスのトリガーをチェック。押した瞬間だけtrueになる
/// </summary>
/// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
/// <returns>トリガーか</returns>
bool Input::ReleaseMouse(uint8_t mouseNumber) const {

	//クリックやめた瞬間
	if (!mouse_.rgbButtons[mouseNumber] && mousePre_.rgbButtons[mouseNumber]) {
		return true;
	}

	return false;

}

/// <summary>
/// マウス移動量を取得
/// </summary>
/// <returns>マウス移動量</returns>
Input::MouseMove Input::GetMouseMove() {

	return Input::MouseMove(mouse_.lX, mouse_.lY, mouse_.lZ);

}

/// <summary>
/// マウスの位置を取得する（ウィンドウ座標系）
/// </summary>
/// <returns>マウスの位置</returns>
const Vector2& Input::GetMousePosition(HWND hwnd) {

	//マウス座標(スクリーン座標)を取得する
	POINT mousePosition;
	//マウス座標(スクリーン座標)を取得する
	GetCursorPos(&mousePosition);
	//クライアントエリア座標に変換する
	ScreenToClient(hwnd, &mousePosition);

	mousePosition_.x = float(mousePosition.x);
	mousePosition_.y = float(mousePosition.y);

	return mousePosition_;

}
