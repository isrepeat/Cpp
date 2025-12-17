#pragma once

generic<class R, class T> ref class Func0 {
public:
	typedef R(__clrcall *CallbackFn)(T);

	Func0(T _this, CallbackFn fn)
		: _this(_this), fn(fn) {}

	R Invoke() {
		return fn(this->_this);
	}

private:
	T _this;
	CallbackFn fn;
};

generic<class R, class T, class A0> ref class Func1 {
public:
	typedef R(__clrcall *CallbackFn)(T, A0);

	Func1(T _this, CallbackFn fn, A0 a0)
		: _this(_this), fn(fn), a0(a0) {}

	R Invoke() {
		return fn(this->_this, this->a0);
	}

private:
	T _this;
	A0 a0;
	CallbackFn fn;
};

generic<class R, class T, class A0, class A1> ref class Func2 {
public:
	typedef R(__clrcall *CallbackFn)(T, A0, A1);

	Func2(T _this, CallbackFn fn, A0 a0, A1 a1)
		: _this(_this), fn(fn), a0(a0), a1(a1) {}

	R Invoke() {
		return fn(this->_this, this->a0, this->a1);
	}

private:
	T _this;
	A0 a0;
	A1 a1;
	CallbackFn fn;
};

generic<class R, class T, class A0, class A1, class A2> ref class Func3 {
public:
	typedef R(__clrcall *CallbackFn)(T, A0, A1, A2);

	Func3(T _this, CallbackFn fn, A0 a0, A1 a1, A2 a2)
		: _this(_this), fn(fn), a0(a0), a1(a1), a2(a2) {}

	R Invoke() {
		return fn(this->_this, this->a0, this->a1, this->a2);
	}

private:
	T _this;
	A0 a0;
	A1 a1;
	A2 a2;
	CallbackFn fn;
};

generic<class R, class T, class A0, class A1, class A2, class A3> ref class Func4 {
public:
	typedef R(__clrcall *CallbackFn)(T, A0, A1, A2, A3);

	Func4(T _this, CallbackFn fn, A0 a0, A1 a1, A2 a2, A3 a3)
		: _this(_this), fn(fn), a0(a0), a1(a1), a2(a2), a3(a3) {}

	R Invoke() {
		return fn(this->_this, this->a0, this->a1, this->a2, this->a3);
	}

private:
	T _this;
	A0 a0;
	A1 a1;
	A2 a2;
	A3 a3;
	CallbackFn fn;
};

generic<class R, class T, class A0, class A1, class A2, class A3, class A4> ref class Func5 {
public:
	typedef R(__clrcall *CallbackFn)(T, A0, A1, A2, A3, A4);

	Func5(T _this, CallbackFn fn, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
		: _this(_this), fn(fn), a0(a0), a1(a1), a2(a2), a3(a3), a4(a4) {}

	R Invoke() {
		return fn(this->_this, this->a0, this->a1, this->a2, this->a3, this->a4);
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

generic<class R, class T, class A0, class A1, class A2, class A3, class A4, class A5> ref class Func6 {
public:
	typedef R(__clrcall* CallbackFn)(T, A0, A1, A2, A3, A4, A5);

	Func6(T _this, CallbackFn fn, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
		: _this(_this), fn(fn), a0(a0), a1(a1), a2(a2), a3(a3), a4(a4), a5(a5) {}

	R Invoke() {
		return fn(this->_this, this->a0, this->a1, this->a2, this->a3, this->a4, this->a5);
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
template<int Count> struct FuncSelector {
};

template<> struct FuncSelector<0> {
	template<class R, class T> struct M {
		typedef Func0<R, T> Func;
	};
};

template<> struct FuncSelector<1> {
	template<class R, class T, class A0> struct M {
		typedef Func1<R, T, A0> Func;
	};
};

template<> struct FuncSelector<2> {
	template<class R, class T, class A0, class A1> struct M {
		typedef Func2<R, T, A0, A1> Func;
	};
};

template<> struct FuncSelector<3> {
	template<class R, class T, class A0, class A1, class A2> struct M {
		typedef Func3<R, T, A0, A1, A2> Func;
	};
};

template<> struct FuncSelector<4> {
	template<class R, class T, class A0, class A1, class A2, class A3> struct M {
		typedef Func4<R, T, A0, A1, A2, A3> Func;
	};
};

template<> struct FuncSelector<5> {
	template<class R, class T, class A0, class A1, class A2, class A3, class A4> struct M {
		typedef Func5<R, T, A0, A1, A2, A3, A4> Func;
	};
};

template<> struct FuncSelector<6> {
	template<class R, class T, class A0, class A1, class A2, class A3, class A4, class A5> struct M {
		typedef Func6<R, T, A0, A1, A2, A3, A4, A5> Func;
	};
};

// FuncRetType ====
template<class Fn> struct FuncRetType {
};

template<class R, class... Args> struct FuncRetType<R(__clrcall *)(Args ...)> {
	typedef R Ret;
};

// Functions ====
template<class R, class T, class... Args>
typename FuncSelector<sizeof...(Args)>:: template M<R, T, Args ...>::Func ^MakeFunc(
	T _this,
	typename FuncSelector<sizeof...(Args)>:: template M<R, T, Args ...>::Func::CallbackFn fn,
	Args ...args)
{
	return gcnew typename FuncSelector<sizeof...(Args)>:: template M<R, T, Args ...>::Func(_this, fn, args...);
}

template<class T, class Fn, class... Args>
System::Threading::Tasks::Task<typename FuncRetType<Fn>::Ret> ^StartFuncTask(
	T _this,
	Fn fn,
	Args ...args)
{
	typedef typename FuncRetType<Fn>::Ret Ret;
	typedef typename FuncSelector<sizeof...(Args)>:: template M<Ret, T, Args ...>::Func FuncType;
	typedef typename FuncSelector<sizeof...(Args)>:: template M<Ret, T, Args ...>::Func::CallbackFn CallbackType;

	FuncType ^action = MakeFunc<Ret>(_this, fn, args...);
	System::Func<Ret> ^sysFunc = gcnew System::Func<Ret>(action, &FuncType::Invoke);
	/*auto task = System::Threading::Tasks::Task::Run(sysFunc);*/
	auto taskFactory = gcnew System::Threading::Tasks::TaskFactory();
	auto task = taskFactory->StartNew(sysFunc);
	return task;
}