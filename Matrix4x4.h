#pragma once


//行列4x4
struct Matrix4x4 {

	float m[4][4];

};


class Matrix4x4Calc
{
public:

    /// <summary>
    /// シングルトンインスタンスの取得
    /// </summary>
    /// <returns></returns>
    static Matrix4x4Calc* GetInstance();

    //加算
    Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);
    //減算
    Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);
    //積
    Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
    //逆行列
    Matrix4x4 Inverse(const Matrix4x4& m);
    //転置行列
    Matrix4x4 Transpose(const Matrix4x4& m);
    //単位行列の作成
    Matrix4x4 MakeIdentity4x4();

    //平行移動行列
    Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
    //拡大縮小行列
    Matrix4x4 MakeScaleMatrix(const Vector3& scale);
    //座標変換
    Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);
    //座標変換(平行移動なし)
    Vector3 TransformNormal(const Vector3& vector, const Matrix4x4& matrix);

    //X軸回転行列
    Matrix4x4 MakeRotateXMatrix(float radian);
    //Y軸回転行列
    Matrix4x4 MakeRotateYMatrix(float radian);
    //Z軸回転行列
    Matrix4x4 MakeRotateZMatrix(float radian);

    //3次元アフィン変換行列
    Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

    // 正射影行列
    Matrix4x4 MakeOrthographicMatrix(
        float left, float top, float right, float bottom, float nearClip, float farClip);

    // 透視投影行列
    Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

    // ビューポート変換行列
    Matrix4x4 MakeViewportMatrix(
        float left, float top, float width, float height, float minDepth, float maxDepth);


private:
    Matrix4x4Calc() = default;
    ~Matrix4x4Calc() = default;
    Matrix4x4Calc(const Matrix4x4Calc&) = delete;
    Matrix4x4Calc& operator=(const Matrix4x4Calc&) = delete;
};
