#pragma once
#include "..\config.h"

#include <wrl.h>
#include <wrl/implements.h>
#include <windows.storage.streams.h>
#include <robuffer.h>
#include <cstdint>

class ArrayIBuffer :
	public Microsoft::WRL::RuntimeClass < Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::WinRtClassicComMix>,
	ABI::Windows::Storage::Streams::IBuffer,
	Windows::Storage::Streams::IBufferByteAccess >
{
public:
	virtual ~ArrayIBuffer(){
	}

#if HAVE_WINRT == 1
	Windows::Storage::Streams::IBuffer ^AsIBuffer(){
		Windows::Storage::Streams::IBuffer ^buf = reinterpret_cast<Windows::Storage::Streams::IBuffer ^>(this);
		return buf;
	}
#endif

	STDMETHODIMP RuntimeClassInitialize(){
		this->buffer = nullptr;
		this->size = 0;
		this->usedSize = 0;
		return S_OK;
	}

	STDMETHODIMP Buffer(byte **value){
		*value = this->buffer;
		return S_OK;
	}

	STDMETHODIMP get_Capacity(UINT32 *value){
		*value = this->size;
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
		return this->size;
	}

	const uint8_t *GetBuffer() const{
		return this->buffer;
	}

	void SetBuffer(uint8_t *buffer, uint32_t size, uint32_t usedSize = 0){
		this->buffer = buffer;
		this->size = size;
		this->usedSize = usedSize;
	}
private:
	uint8_t *buffer;
	uint32_t size;
	uint32_t usedSize;
};