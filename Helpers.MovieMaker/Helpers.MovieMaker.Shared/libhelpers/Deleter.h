#pragma once

template<class T>
struct DeleteFn {
	typedef void (*Delete)(T *obj);
};

template<class T, class DeleteType>
void SimpleDelete(T *obj) {
	DeleteType *tmp = reinterpret_cast<DeleteType *>(obj);
	delete tmp;
}

template<class T, class DeleteType>
void DynamicDelete(T *obj) {
	DeleteType *tmp = dynamic_cast<DeleteType *>(obj);
	delete tmp;
}