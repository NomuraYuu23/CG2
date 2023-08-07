#pragma once

#include <array>
#include <Windows.h>
#include <wrl.h>

#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>

class Input
{
public:

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
	void KeyboardUpdata();

	/// <summary>
	/// キーの押下をチェック
	/// </summary>
	/// <param name="keyNumber">キー番号</param>
	/// <returns>押されているか</returns>
	bool PushKey(uint8_t keyNumber);

	/// <summary>
	/// キーのトリガーをチェック
	/// </summary>
	/// <param name="keyNumber">キー番号</param>
	/// <returns>トリガーか</returns>
	bool TriggerKey(uint8_t keyNumber);

	const std::array<BYTE, 256>& GetAllKey() { return key_; }

private:

	// DirectInput
	Microsoft::WRL::ComPtr<IDirectInput8> directInput_;
	
	//キーボード
	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_;
	std::array<BYTE, 256> key_;
	std::array<BYTE, 256> keyPre_;


};

