#pragma once

#include <array>
#include <Windows.h>
#include <wrl.h>

#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>
#pragma comment(lib, "dxguid.lib")
#include <Xinput.h>

#include "Vector2.h"

class Input
{
public:
	struct MouseMove {
		LONG lX;
		LONG lY;
		LONG lZ;
	};


	static Input* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="hInstance"></param>
	/// <param name="hwnd"></param>
	void Initialize(HINSTANCE hInstance, HWND hwnd);

	/// <summary>
	/// 毎フレーム
	/// </summary>
	void Update();

	/// <summary>
	/// キーボード関連更新
	/// </summary>
	void KeyboardUpdate();

	/// <summary>
	/// キーを押した状態か
	/// </summary>
	/// <param name="keyNumber">キー番号</param>
	/// <returns>キーを押した状態か</returns>
	bool PushKey(uint8_t keyNumber) const;

	/// <summary>
	/// キーを離した状態か
	/// </summary>
	/// <param name="keyNumber">キー番号</param>
	/// <returns>キーを離した状態か</returns>
	bool NoPushKey(uint8_t keyNumber) const;

	/// <summary>
	/// キーを押した瞬間か
	/// </summary>
	/// <param name="keyNumber">キー番号</param>
	/// <returns>キーを押した瞬間か</returns>
	bool TriggerKey(uint8_t keyNumber) const;

	/// <summary>
	/// キーを離した瞬間か
	/// </summary>
	/// <param name="keyNumber">キー番号</param>
	/// <returns>キーを離した瞬間か</returns>
	bool ReleaseKey(uint8_t keyNumber) const;

	const std::array<BYTE, 256>& GetAllKey() const { return key_; }

	/// <summary>
	/// マウス関連更新
	/// </summary>
	void MouseUpdate();

	/// <summary>
	/// マウスボタンを押した状態か
	/// </summary>
	/// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
	/// <returns>マウスボタンを押した状態か</returns>
	bool PushMouse(uint8_t mouseNumber) const;

	/// <summary>
	/// マウスボタンを離した状態か
	/// </summary>
	/// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
	/// <returns>マウスボタンを離した状態か</returns>
	bool NoPushMouse(uint8_t mouseNumber) const;

	/// <summary>
	/// マウスボタンを押した瞬間か
	/// </summary>
	/// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
	/// <returns>マウスボタンを押した瞬間か</returns>
	bool TriggerMouse(uint8_t mouseNumber) const;

	/// <summary>
	/// マウスボタンを離した瞬間か
	/// </summary>
	/// <param name="buttonNumber">マウスボタン番号(0:左,1:右,2:中,3~7:拡張マウスボタン)</param>
	/// <returns>マウスボタンを離した瞬間か</returns>
	bool ReleaseMouse(uint8_t mouseNumber) const;

	/// <summary>
	/// マウス移動量を取得
	/// </summary>
	/// <returns>マウス移動量</returns>
	MouseMove GetMouseMove();

	/// <summary>
	/// マウスの位置を取得する（ウィンドウ座標系）
	/// </summary>
	/// <returns>マウスの位置</returns>
	const Vector2& GetMousePosition(HWND hwnd);

	const DIMOUSESTATE2& GetAllMouse() const { return mouse_; }

	/// <summary>
	/// ジョイスティック関連更新
	/// </summary>
	void JoystickUpdate();

	/// <summary>
	/// ジョイスティックボタンを押した状態か
	/// </summary>
	/// <param name="buttonNumber"></param>
	/// <returns>ジョイスティックボタンを押した状態か</returns>
	bool PushJoystick(uint8_t joystickNumber) const;

	/// <summary>
	/// ジョイスティックボタンを離した状態か
	/// </summary>
	/// <param name="buttonNumber"></param>
	/// <returns>ジョイスティックボタンを離した状態か</returns>
	bool NoPushJoystick(uint8_t joystickNumber) const;

	/// <summary>
	/// ジョイスティックボタンを押した瞬間か
	/// </summary>
	/// <param name="buttonNumber"></param>
	/// <returns>ジョイスティックボタンを押した瞬間か</returns>
	bool TriggerJoystick(uint8_t joystickNumber) const;

	/// <summary>
	/// ジョイスティックボタンを離した瞬間か
	/// </summary>
	/// <param name="buttonNumber"></param>
	/// <returns>ジョイスティックボタンを離した瞬間か</returns>
	bool ReleaseJoystick(uint8_t mousjoystickNumbereNumber) const;

	/// <summary>
	/// 左のアナログスティックの状態を取得
	/// </summary>
	/// <returns>0~</returns>
	Vector2 GetLeftAnalogstick() const;

	/// <summary>
	/// 右のアナログスティックの状態を取得
	/// </summary>
	/// <returns></returns>
	Vector2 GetRightAnalogstick() const;

	/// <summary>
	/// 左右のトリガーの状態を取得()
	/// </summary>
	/// <returns></returns>
	float GetLRTrrigger() const;

	const DIJOYSTATE2& GetJoystickState() const { return joystick_; }

	/// <summary>
	/// ジョイスティック接続
	/// </summary>
	void JoystickConnected(HWND hwnd);


private:

	Input() = default;
	~Input() = default;
	Input(const Input&) = delete;
	const Input& operator=(const Input&) = delete;


	// DirectInput
	Microsoft::WRL::ComPtr<IDirectInput8> directInput_ = nullptr;
	
	//キーボード
	Microsoft::WRL::ComPtr<IDirectInputDevice8> directKeyboard_;
	std::array<BYTE, 256> key_;
	std::array<BYTE, 256> keyPre_;

	// マウス
	Microsoft::WRL::ComPtr<IDirectInputDevice8> directMouse_ = nullptr;
	DIMOUSESTATE2 mouse_;
	DIMOUSESTATE2 mousePre_;
	Vector2 mousePosition_;

	// ジョイスティック
	Microsoft::WRL::ComPtr<IDirectInputDevice8> directJoystick_ = nullptr;
	DIJOYSTATE2 joystick_;
	DIJOYSTATE2 joystickPre_;
	bool joystickConnected;

};

