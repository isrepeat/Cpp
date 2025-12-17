#pragma once
// http://www.codeproject.com/Articles/12771/CAutoNativePtr-A-managed-smart-pointer-for-using-n?msg=1356536#xx1356536xx
/***************************************************************
*
*   Module Name:
*      msclr_auto_ptr.h
*
*   Abstract:
*      A auto pointer that can be   used as a member of
*      a CLR class.
*
*   Author:
*      Denis N. Shevchenko [dsheva@gmail.com] 1-Feb-2006
*
*   License:
*      You may freely use and modify this class as long as you
*      include this copyright.
*
*      This code is provided "as is" without express
*      or implied warranty.
*
*      Copyright © Denis N. Shevchenko, 2006.
*
***************************************************************/

#pragma once

#include <msclr\safebool.h>
#include <assert.h>
#include <type_traits>


namespace msclr
{


	template<class T>
	ref class auto_ptr
	{
	public:
		typedef T element_type;

	public:
		auto_ptr() /*throw()*/
			: p_(0)
		{
		}

		explicit auto_ptr(T *p) /*throw()*/
			: p_(p)
		{
		}

		auto_ptr(auto_ptr<T> %rhs) /*throw()*/
			: p_(rhs.release())
		{
		}

		template<class T2>
		auto_ptr(auto_ptr<T2> %rhs) /*throw()*/
			: p_(rhs.release())
		{
		}

		~auto_ptr() /*throw()*/
		{
			delete p_;
		}

		!auto_ptr() /*throw()*/
		{
			//using namespace std;
			//assert(("msclr::auto_ptr<> : Finalizer used! The variable deleted in non-deterministic way.", false));

			delete p_;
		}

	public:
		template<class T2>
		operator auto_ptr<T2>() /*throw()*/
		{
			return auto_ptr<T2>(*this);
		}

		auto_ptr<T>% operator=(auto_ptr<T> %rhs) /*throw()*/
		{
			reset(rhs.release());
			return *this;
		}

		template<class T2>
		auto_ptr<T>% operator=(auto_ptr<T2> %rhs) /*throw()*/
		{
			reset(rhs.release());
			return *this;
		}

		static T& operator*(auto_ptr %ap) /*const throw()*/
		{
			if (!ap.p_)
				throw gcnew System::NullReferenceException("msclr::auto_ptr<>");
			return *ap.p_;
		}

		T* operator->() /*const throw()*/
		{
			return &**this;
		}

	public:
		T* get() /*const throw()*/
		{
			return p_;
		}

		T* release() /*throw()*/
		{
			T *p = p_;
			p_ = 0;
			return p;
		}

		void reset() /*throw()*/
		{
			reset(0);
		}

		void reset(T *p) /*throw()*/
		{
			if (p != p_)
				delete p_;
			p_ = p;
		}

		// Additional functionality like in msclr::auto_handle
	public:
		void swap(auto_ptr<T> %rhs) /*throw()*/
		{
			auto_ptr<T> tmp = rhs;
			rhs = *this;
			*this = tmp;
		}

		// for use when auto_ptr appears in a conditional
		operator _detail_class::_safe_bool() /*throw()*/
		{
			return p_ != 0 ? _detail_class::_safe_true : _detail_class::_safe_false;
		}

		// for use when auto_ptr appears in a conditional
		bool operator!() /*throw()*/
		{
			return p_ == 0;
		}

	private:
		T *p_;
	};


	template<typename T>
	void swap(auto_ptr<T> %lhs, auto_ptr<T> %rhs)
	{
		lhs.swap(rhs);
	}

	template<typename T, class... Args>
	auto_ptr<T> make_auto(Args&&... args) {
		return auto_ptr<T>(new T(std::forward<Args>(args)...));
	}


} // namespace msclr