#pragma once

#include <utility>
#include <memory>

namespace H {
	class IScope {
	public:
		virtual ~IScope() = default;

		virtual void EndScope() = 0;
		virtual void DisableScope() = 0;
	};

	template<class Fn>
	class Scope final : public IScope {
	public:
		Scope() = default;

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

		Scope& operator=(const Scope&) = delete;

		Scope& operator=(Scope&& other) {
			swap(*this, other);
			return *this;
		}

		void EndScope() override {
			if (!m_valid) {
				return;
			}

			m_fn();
			this->DisableScope();
		}

		void DisableScope() override {
			m_valid = false;
		}

		friend void swap(Scope& a, Scope& b) {
			using std::swap;
			swap(a.m_valid, b.m_valid);
			swap(a.m_fn, b.m_fn);
		}

	private:
		bool m_valid = false;
		Fn m_fn;
	};


	template<class Fn>
	Scope<Fn> MakeScope(Fn fn) {
		return Scope<Fn>(std::move(fn));
	}

	template<class Fn>
	std::unique_ptr<Scope<Fn>> MakeScopeUPtr(Fn fn) {
		return std::make_unique<Scope<Fn>>(std::move(fn));
	}
}
