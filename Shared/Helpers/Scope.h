#pragma once
#include <utility>

namespace H {
	template<class Fn>
	class Scope {
	public:
		Scope()
			: m_valid(false)
		{}

		Scope(Fn fn)
			: m_valid(true)
			, m_fn(std::move(fn))
		{}

		Scope(const Scope&) = delete;

		Scope(Scope&& other)
			: Scope()
		{
			swap(*this, other);
		}

		~Scope() {
			try {
				this->EndScope();
			}
			catch (...) {
			}
		}

		Scope& operator=(Scope other) {
			swap(*this, other);
			return *this;
		}

		void EndScope() {
			if (!m_valid) {
				return;
			}

			m_fn();
			m_valid = false;
		}

		friend void swap(Scope& a, Scope& b) {
			using std::swap;
			swap(a.m_valid, b.m_valid);
			swap(a.m_fn, b.m_fn);
		}

	private:
		bool m_valid;
		Fn m_fn;
	};

	template<class Fn>
	Scope<Fn> MakeScope(Fn fn) {
		return Scope<Fn>(std::move(fn));
	}
}