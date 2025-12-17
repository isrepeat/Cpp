#pragma once

#include <malloc.h>
#include <vector>

// never allocate on heap!!!
template<class T> class dynamic_stack_array{
public:
	dynamic_stack_array()
		: count(0), memory(nullptr){
	}

	dynamic_stack_array(size_t count)
		: count(count)
	{
		this->memory = static_cast<T *>(_malloca(this->count * sizeof(T)));
	}

	dynamic_stack_array(const dynamic_stack_array &other)
		: count(other.count)
	{
		this->memory = static_cast<T *>(_malloca(this->count * sizeof(T)));
		std::memcpy(this->memory, other.memory, this->count * sizeof(T));
	}

	dynamic_stack_array(dynamic_stack_array &&other)
		: count(other.count){
		this->memory = other.memory;
		other.memory = nullptr;
		other.count = 0;
	}

	~dynamic_stack_array(){
		if (this->memory){
			_freea(this->memory);
		}
	}

	dynamic_stack_array &operator=(const dynamic_stack_array &other){
		if (this != &other){
			this->count = other.count;
			this->memory = static_cast<T *>(_malloca(this->count * sizeof(T)));
			std::memcpy(this->memory, other.memory, this->count * sizeof(T));
		}

		return *this;
	}

	dynamic_stack_array &operator=(dynamic_stack_array &&other){
		if (this != &other){
			this->count = other.count;
			this->memory = other.memory;
			other.memory = nullptr;
			other.count = 0;
		}

		return *this;
	}

	T &operator[] (const int idx){
		return this->memory[idx];
	}

	T const &operator[] (const int idx) const{
		return this->memory[idx];
	}

	T &operator[] (const size_t idx){
		return this->memory[idx];
	}

	T const &operator[] (const size_t idx) const{
		return this->memory[idx];
	}

	T *data(){
		return this->memory;
	}

	T const *data() const{
		return this->memory;
	}

	size_t size() const{
		return this->count;
	}

	void resize(size_t count){
		if (this->memory){
			_freea(this->memory);
		}

		this->count = count;
		this->memory = static_cast<T *>(_malloca(this->count * sizeof(T)));
	}
private:
	T *memory;
	size_t count;
};