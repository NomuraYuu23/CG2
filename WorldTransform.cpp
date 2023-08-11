#include "WorldTransform.h"

void WorldTransform::Initialize() {

	UpdateMatrix();

}

void WorldTransform::UpdateMatrix() {

	Matrix4x4Calc* calc = Matrix4x4Calc::GetInstance();

	worldMatrix_ = calc->MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	if (parent_) {
		worldMatrix_ = calc->Multiply(worldMatrix_, parent_->worldMatrix_);
	}

}