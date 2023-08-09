#pragma once
#include <vector>

//3次元ベクトル
struct Vector3
{
	float x;
	float y;
	float z;

};

class Vector3Calc
{
public:

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static Vector3Calc* GetInstance();

	//加算
	Vector3 Add(const Vector3& v1, const Vector3& v2);
	//減算
	Vector3 Subtract(const Vector3& v1, const Vector3& v2);
	//スカラー倍
	Vector3 Multiply(float scalar, const Vector3& v);
	//内積
	float Dot(const Vector3& v1, const Vector3& v2);
	//長さ（ノルム）
	float Length(const Vector3& v);
	//正規化
	Vector3 Normalize(const Vector3& v);
	//クロス積
	Vector3 Cross(const Vector3& v1, const Vector3& v2);

	// 線形補間
	Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

	// 3次スプライン曲線
	Vector3 CatmullRomSpline(
		const std::vector<Vector3>& controlPoints, const float& t);

	// 反射ベクトル
	Vector3 Reflect(const Vector3& input, const Vector3& normal);

private:
	Vector3Calc() = default;
	~Vector3Calc() = default;
	Vector3Calc(const Vector3Calc&) = delete;
	Vector3Calc& operator=(const Vector3Calc&) = delete;

};
