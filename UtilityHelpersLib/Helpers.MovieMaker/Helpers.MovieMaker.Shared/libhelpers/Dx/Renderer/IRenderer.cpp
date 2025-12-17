#include "pch.h"
#include "IRenderer.h"

IRenderer::IRenderer(raw_ptr<DxDevice> dxDeviceSafeObj, raw_ptr<IOutput> output)
	: dxDeviceSafeObj(dxDeviceSafeObj)
	, output(output)
{}

IRenderer::~IRenderer() {
}