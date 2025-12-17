#pragma once
#include "..\config.h"

#include <wrl.h>
#include <wrl/implements.h>
#include <windows.storage.streams.h>
#include <robuffer.h>
#include <vector>
#include <cstdint>

class VectorIBuffer : 
	public Microsoft::WRL::RuntimeClass < Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::WinRtClassicComMix>,
	ABI::Windows::Storage::Streams::IBuffer,
	Windows::Storage::Streams::IBufferByteAccess >
{
public:
	virtual ~VectorIBuffer(){
	}

#if HAVE_WINRT == 1
	Windows::Storage::Streams::IBuffer ^AsIBuffer(){
		Windows::Storage::Streams::IBuffer ^buf = reinterpret_cast<Windows::Storage::Streams::IBuffer ^>(this);
		return buf;
	}
#endif

	STDMETHODIMP RuntimeClassInitialize(){
		this->usedSize = 0;
		return S_OK;
	}

	STDMETHODIMP Buffer(byte **value){
		*value = this->buffer.data();
		return S_OK;
	}

	STDMETHODIMP get_Capacity(UINT32 *value){
		*value = static_cast<UINT32>(this->buffer.size());
		return S_OK;
	}

	STDMETHODIMP get_Length(UINT32 *value){
		*value = this->usedSize;
		return S_OK;
	}

	STDMETHODIMP put_Length(UINT32 value){
		this->usedSize = value;
		return S_OK;
	}

	uint32_t GetUsedSize() const{
		return this->usedSize;
	}

	size_t GetSize() const{
		return this->buffer.size();
	}

	void SetSize(size_t v){
		this->buffer.resize(v);
	}

	const std::vector<uint8_t> &GetBuffer() const{
		return this->buffer;
	}

	std::vector<uint8_t> MoveBuffer() const{
		return std::move(this->buffer);
	}

	void SetBuffer(const std::vector<uint8_t> &v){
		this->buffer = v;
	}

	void SetBuffer(std::vector<uint8_t> &&v){
		this->buffer = std::move(v);
	}
private:
	std::vector<uint8_t> buffer;
	uint32_t usedSize;
};