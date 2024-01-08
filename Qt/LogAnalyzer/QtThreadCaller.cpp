#include "QtThreadCaller.h"

// NOTE: First creating must be in qt thread, otherwise signals can't invoked
QtThreadCaller& QtThreadCaller::GetInstance() {
	static QtThreadCaller instance;
	return instance;
}

QtThreadCaller::QtThreadCaller()
	: QObject(nullptr)
{
	connect(this, &QtThreadCaller::_Invoke, this, [this](R::Callback callback) {
		if (callback) {
			callback();
		}
		});
}

void QtThreadCaller::Invoke(R::Callback callback) {
	auto& _this = GetInstance();
	emit _this._Invoke(callback);
}