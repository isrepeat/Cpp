#pragma once

generic<class T, class In0> ref class Action0In1 {
public:
	typedef void(__clrcall *CallbackFn)(T, In0);

	Action0In1(T _this, CallbackFn fn)
		: _this(_this), fn(fn) {}

	void Invoke(In0 arg) {
		fn(this->_this, arg);
	}

private:
	T _this;
	CallbackFn fn;
};

generic<class T> ref class Action0 {
public:
	typedef void(__clrcall *CallbackFn)(T);

	Action0(T _this, CallbackFn fn)
		: _this(_this), fn(fn) {}

	void Invoke() {
		fn(this->_this);
	}

private:
	T _this;
	CallbackFn fn;
};

generic<class T, class A0> ref class Action1 {
public:
	typedef void(__clrcall *CallbackFn)(T, A0);

	Action1(T _this, CallbackFn fn, A0 a0)
		: _this(_this), fn(fn), a0(a0) {}

	void Invoke() {
		fn(this->_this, this->a0);
	}

private:
	T _this;
	A0 a0;
	CallbackFn fn;
};

generic<class T, class A0, class A1> ref class Action2 {
public:
	typedef void(__clrcall *CallbackFn)(T, A0, A1);

	Action2(T _this, CallbackFn fn, A0 a0, A1 a1)
		: _this(_this), fn(fn), a0(a0), a1(a1) {}

	void Invoke() {
		fn(this->_this, this->a0, this->a1);
	}

private:
	T _this;
	A0 a0;
	A1 a1;
	CallbackFn fn;
};

generic<class T, class A0, class A1, class A2> ref class Action3 {
public:
	typedef void(__clrcall *CallbackFn)(T, A0, A1, A2);

	Action3(T _this, CallbackFn fn, A0 a0, A1 a1, A2 a2)
		: _this(_this), fn(fn), a0(a0), a1(a1), a2(a2) {}

	void Invoke() {
		fn(this->_this, this->a0, this->a1, this->a2);
	}

private:
	T _this;
	A0 a0;
	A1 a1;
	A2 a2;
	CallbackFn fn;
};

generic<class T, class A0, class A1, class A2, class A3> ref class Action4 {
public:
	typedef void(__clrcall *CallbackFn)(T, A0, A1, A2, A3);

	Action4(T _this, CallbackFn fn, A0 a0, A1 a1, A2 a2, A3 a3)
		: _this(_this), fn(fn), a0(a0), a1(a1), a2(a2), a3(a3) {}

	void Invoke() {
		fn(this->_this, this->a0, this->a1, this->a2, this->a3);
	}

private:
	T _this;
	A0 a0;
	A1 a1;
	A2 a2;
	A3 a3;
	CallbackFn fn;
};

generic<class T, class A0, class A1, class A2, class A3, class A4> ref class Action5 {
public:
	typedef void(__clrcall *CallbackFn)(T, A0, A1, A2, A3, A4);

	Action5(T _this, CallbackFn fn, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
		: _this(_this), fn(fn), a0(a0), a1(a1), a2(a2), a3(a3), a4(a4) {}

	void Invoke() {
		fn(this->_this, this->a0, this->a1, this->a2, this->a3, this->a4);
	}

private:
	T _this;
	A0 a0;
	A1 a1;
	A2 a2;
	A3 a3;
	A4 a4;
	CallbackFn fn;
};

generic<class T, class A0, class A1, class A2, class A3, class A4, class A5> ref class Action6 {
public:
	typedef void(__clrcall *CallbackFn)(T, A0, A1, A2, A3, A4, A5);

	Action6(T _this, CallbackFn fn, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
		: _this(_this), fn(fn), a0(a0), a1(a1), a2(a2), a3(a3), a4(a4), a5(a5) {}

	void Invoke() {
		fn(this->_this, this->a0, this->a1, this->a2, this->a3, this->a4, this->a5);
	}

private:
	T _this;
	A0 a0;
	A1 a1;
	A2 a2;
	A3 a3;
	A4 a4;
	A5 a5;
	CallbackFn fn;
};

// Selector ====
template<int Count> struct ActionSelector {
};

template<> struct ActionSelector<0> {
	template<class T> struct M {
		typedef Action0<T> Action;
	};
};

template<> struct ActionSelector<1> {
	template<class T, class A0> struct M {
		typedef Action1<T, A0> Action;
	};
};

template<> struct ActionSelector<2> {
	template<class T, class A0, class A1> struct M {
		typedef Action2<T, A0, A1> Action;
	};
};

template<> struct ActionSelector<3> {
	template<class T, class A0, class A1, class A2> struct M {
		typedef Action3<T, A0, A1, A2> Action;
	};
};

template<> struct ActionSelector<4> {
	template<class T, class A0, class A1, class A2, class A3> struct M {
		typedef Action4<T, A0, A1, A2, A3> Action;
	};
};

template<> struct ActionSelector<5> {
	template<class T, class A0, class A1, class A2, class A3, class A4> struct M {
		typedef Action5<T, A0, A1, A2, A3, A4> Action;
	};
};

template<> struct ActionSelector<6> {
	template<class T, class A0, class A1, class A2, class A3, class A4, class A5> struct M {
		typedef Action6<T, A0, A1, A2, A3, A4, A5> Action;
	};
};

// Functions ====
template<class T, class... Args>
typename ActionSelector<sizeof...(Args)>:: template M<T, Args ...>::Action ^MakeAction(
	T _this,
	typename ActionSelector<sizeof...(Args)>:: template M<T, Args ...>::Action::CallbackFn fn,
	Args ...args)
{
	return gcnew typename ActionSelector<sizeof...(Args)>:: template M<T, Args ...>::Action(_this, fn, args...);
}

template<class T, class... Args>
System::Threading::Tasks::Task ^StartActionTask(
	T _this,
	typename ActionSelector<sizeof...(Args)>:: template M<T, Args ...>::Action::CallbackFn fn,
	Args ...args) 
{
	typedef typename ActionSelector<sizeof...(Args)>:: template M<T, Args ...>::Action ActionType;

	ActionType ^action = MakeAction(_this, fn, args...);
	System::Action ^sysAction = gcnew System::Action(action, &ActionType::Invoke);
	/*auto task = System::Threading::Tasks::Task::Run(sysAction);*/
	auto taskFactory = gcnew System::Threading::Tasks::TaskFactory();
	auto task = taskFactory->StartNew(sysAction);
	return task;
}