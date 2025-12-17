#include "pch.h"
#include "D2DTransformState.h"

D2DTransformState::D2DTransformState(ID2D1DeviceContext *d2dCtx)
	: d2dCtx(d2dCtx)
{
	this->d2dCtx->GetTransform(&this->transform);
}

D2DTransformState::D2DTransformState(D2DTransformState &&other) 
	: d2dCtx(std::move(other.d2dCtx)), transform(std::move(other.transform))
{
	other.d2dCtx = nullptr;
}

D2DTransformState::~D2DTransformState() {
	if (this->d2dCtx) {
		this->d2dCtx->SetTransform(this->transform);
	}
}

D2DTransformState &D2DTransformState::operator=(D2DTransformState &&other) {
	if (this != &other) {
		this->d2dCtx = std::move(other.d2dCtx);
		this->transform = std::move(other.transform);

		other.d2dCtx = nullptr;
	}

	return *this;
}

const D2D1_MATRIX_3X2_F &D2DTransformState::GetTransform() const {
	return this->transform;
}