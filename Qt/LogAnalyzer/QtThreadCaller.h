#pragma once
#include <QObject>
#include <QMetatype>
#include <functional>

// Registered metatypes
namespace R {
	using Callback = std::function<void()>;
	Q_DECLARE_METATYPE(R::Callback);
	static const bool regCallback = qRegisterMetaType<Callback>();
}

class QtThreadCaller : public QObject {
private:
	Q_OBJECT;
	QtThreadCaller();
public:
	~QtThreadCaller() = default;
	static QtThreadCaller& GetInstance();

	static void Invoke(R::Callback callback);

signals:
	void _Invoke(R::Callback callback);
};