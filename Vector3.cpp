#include "Vector3.h"
#include <cmath>

/// <summary>
/// シングルトンインスタンスの取得
/// </summary>
/// <returns></returns>
Vector3Calc* Vector3Calc::GetInstance() {
	static Vector3Calc instance;
	return &instance;
}

//加算
Vector3 Vector3Calc::Add(const Vector3& v1, const Vector3& v2) {

	Vector3 result;

	result = { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };

	return result;

}

//減算
Vector3 Vector3Calc::Subtract(const Vector3& v1, const Vector3& v2) {

	Vector3 result;

	result = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };

	return result;

}

//スカラー倍
Vector3 Vector3Calc::Multiply(float scalar, const Vector3& v) {

	Vector3 result;

	result = { scalar * v.x, scalar * v.y, scalar * v.z };

	return result;

}

//内積
float Vector3Calc::Dot(const Vector3& v1, const Vector3& v2) {

	float result;

	result = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;

	return result;

}

//長さ（ノルム）
float Vector3Calc::Length(const Vector3& v) {

	float result;

	result = std::sqrtf(Dot(v, v));

	return result;

}


//正規化
Vector3 Vector3Calc::Normalize(const Vector3& v) {

	Vector3 result;
	float norm;

	norm = Length(v);

	if (v.x != 0.0) {
		result.x = v.x / norm;
	}
	else {
		result.x = 0.0f;
	}

	if (v.y != 0.0) {
		result.y = v.y / norm;
	}
	else {
		result.y = 0.0f;
	}


	if (v.z != 0.0) {
		result.z = v.z / norm;
	}
	else {
		result.z = 0.0f;
	}

	return result;

}

//クロス積
Vector3 Vector3Calc::Cross(const Vector3& v1, const Vector3& v2) {

	Vector3 result = { v1.y * v2.z - v1.z * v2.y,v1.z * v2.x - v1.x * v2.z,v1.x * v2.y - v1.y * v2.x, };

	return result;

}

// 線形補間
Vector3 Vector3Calc::Lerp(const Vector3& v1, const Vector3& v2, float t) {

	Vector3 result = Add(v1, Multiply(t, Subtract(v2, v1)));

	return result;
}

// 3次スプライン曲線
Vector3 Vector3Calc::CatmullRomSpline(
	const std::vector<Vector3>& controlPoints, const float& t) {

	Vector3 result;

	uint32_t point = int(float(controlPoints.size() - 1) * t);
	float t_ = float(controlPoints.size() - 1) * t - float(point);
	if (point == controlPoints.size() - 1) {
		point = int(float(controlPoints.size() - 1));
		t_ = 1.0f;
	}

	Vector3 p0, p1, p2, p3;

	if (point <= 0) {

		p0 = controlPoints[0];
		p1 = controlPoints[0];
		p2 = controlPoints[1];
		p3 = controlPoints[2];

	}
	else if (point >= controlPoints.size() - 2) {

		p0 = controlPoints[controlPoints.size() - 3];
		p1 = controlPoints[controlPoints.size() - 2];
		p2 = controlPoints[controlPoints.size() - 1];
		p3 = controlPoints[controlPoints.size() - 1];

	}
	else {

		p0 = controlPoints[--point];
		p1 = controlPoints[++point];
		p2 = controlPoints[++point];
		p3 = controlPoints[++point];
	}

	result.x = 1.0f / 2.0f *
		((-1.0f * p0.x + 3.0f * p1.x + -3.0f * p2.x + p3.x) * std::powf(t_, 3.0f) +
			(2.0f * p0.x + -5.0f * p1.x + 4.0f * p2.x + -1.0f * p3.x) * std::powf(t_, 2.0f) +
			(-1.0f * p0.x + p2.x) * t_ + 2.0f * p1.x);

	result.y = 1.0f / 2.0f *
		((-1.0f * p0.y + 3.0f * p1.y + -3.0f * p2.y + p3.y) * std::powf(t_, 3.0f) +
			(2.0f * p0.y + -5.0f * p1.y + 4.0f * p2.y + -1.0f * p3.y) * std::powf(t_, 2.0f) +
			(-1.0f * p0.y + p2.y) * t_ + 2.0f * p1.y);

	result.z = 1.0f / 2.0f *
		((-1.0f * p0.z + 3.0f * p1.z + -3.0f * p2.z + p3.z) * std::powf(t_, 3.0f) +
			(2.0f * p0.z + -5.0f * p1.z + 4.0f * p2.z + -1.0f * p3.z) * std::powf(t_, 2.0f) +
			(-1.0f * p0.z + p2.z) * t_ + 2.0f * p1.z);

	return result;
}

// 反射ベクトル
Vector3 Vector3Calc::Reflect(const Vector3& input, const Vector3& normal) {

	return Subtract(input, Multiply(2.0f * Dot(input, normal), normal));

}
