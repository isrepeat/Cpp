#pragma once
#include "common.h"
#include "Macros.h"
#include "Logger.h"
#include <chrono>
#include <ratio>

namespace HELPERS_NS {
	//
	// Rational
	//
	template <typename _Rep>
	struct Rational {

		// This struct helps you to initialize many similar Rational-objects with same {num, den}
		struct Units {
			intmax_t num = 0;
			intmax_t den = 1;
		};

		Rational(Units units)
			: num{ units.num }
			, den{ this->GetSafeDenumerator(units.den) }
			, rational{ this->GetRationalValue() } {
		}

		Rational(intmax_t num, intmax_t den, _Rep valueRep = 1)
			: num{ num }
			, den{ this->GetSafeDenumerator(den) }
			, rational{ this->GetRationalValue() }
			, valueRep{ valueRep } {
		}

		template <typename _OtherRep, typename _OtherPeriod>
		Rational(intmax_t num, intmax_t den, const std::chrono::duration<_OtherRep, _OtherPeriod>& duration)
			: num{ num }
			, den{ this->GetSafeDenumerator(den) }
			, rational{ this->GetRationalValue() }
			, valueRep{ Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this).Value() } {
		}

		intmax_t Numerator() const {
			return this->num;
		}
		intmax_t Denumerator() const {
			return this->den;
		}
		_Rep Value() const {
			return this->valueRep;
		}

		template <typename _ToRep>
		_ToRep As() const {
			return static_cast<_ToRep>(rational);
		}

		template <typename _ToRep, typename _OtherRep>
		Rational<_ToRep> CastToRational(Rational<_OtherRep> other) const {
			if (this->num == other.Numerator() && this->den == other.Denumerator()) {
				return Rational<_ToRep>(this->num, this->den, this->valueRep);
			}
			else {
				return Rational<_ToRep>(
					other.Numerator(),
					other.Denumerator(),
					static_cast<_ToRep>(std::round(this->valueRep * this->As<double>() / other.As<double>())));
			}
		}

		Rational Inversed() const {
			return Rational(this->den, this->num, this->valueRep);
		}

		//
		// Assign operators
		//
		Rational& operator=(_Rep newValueRep) {
			this->valueRep = newValueRep;
			return *this;
		}
		Rational& operator=(const Rational& other) {
			this->valueRep = other.CastToRational<_Rep>(*this).Value();
			return *this;
		}
		template <typename _OtherRep, typename _OtherPeriod>
		Rational& operator=(const std::chrono::duration<_OtherRep, _OtherPeriod>& duration) {
			return *this = Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this);
		}

		//
		// Arithmetic operators
		//
		Rational operator+(const Rational& other) const {
			return Rational{ this->num, this->den, this->valueRep + other.CastToRational<_Rep>(*this).Value() };
		}
		Rational operator-(const Rational& other) const {
			return Rational{ this->num, this->den, this->valueRep - other.CastToRational<_Rep>(*this).Value() };
		}
		Rational& operator+=(const Rational& other) {
			this->valueRep += other.CastToRational<_Rep>(*this).Value();
			return *this;
		}
		Rational& operator-=(const Rational& other) {
			this->valueRep -= other.CastToRational<_Rep>(*this).Value();
			return *this;
		}
		bool operator < (const Rational& other) const {
			return this->valueRep < other.CastToRational<_Rep>(*this).Value();
		}
		bool operator > (const Rational& other) const {
			return this->valueRep > other.CastToRational<_Rep>(*this).Value();
		}
		bool operator <= (const Rational& other) const {
			return this->valueRep <= other.CastToRational<_Rep>(*this).Value();
		}
		bool operator >= (const Rational& other) const {
			return this->valueRep >= other.CastToRational<_Rep>(*this).Value();
		}
		bool operator==(const Rational& other) const {
			return this->valueRep == other.CastToRational<_Rep>(*this).Value();
		}
		bool operator!=(const Rational& other) const {
			return this->valueRep != other.CastToRational<_Rep>(*this).Value();
		}

		template <typename _OtherRep, typename _OtherPeriod>
		Rational operator+(const std::chrono::duration<_OtherRep, _OtherPeriod>& duration) const {
			return *this + Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this);
		}
		template <typename _OtherRep, typename _OtherPeriod>
		Rational operator-(const std::chrono::duration<_OtherRep, _OtherPeriod>& duration) const {
			return *this - Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this);
		}
		template <typename _OtherRep, typename _OtherPeriod>
		Rational& operator+=(const std::chrono::duration<_OtherRep, _OtherPeriod>& duration) {
			return *this += Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this);
		}
		template <typename _OtherRep, typename _OtherPeriod>
		Rational& operator-=(const std::chrono::duration<_OtherRep, _OtherPeriod>& duration) {
			return *this -= Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this);
		}

		template <typename _OtherRep, typename _OtherPeriod>
		bool operator < (const std::chrono::duration<_OtherRep, _OtherPeriod>& duration) const {
			return *this < Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this);
		}
		template <typename _OtherRep, typename _OtherPeriod>
		bool operator > (const std::chrono::duration<_OtherRep, _OtherPeriod>& duration) const {
			return *this > Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this);
		}
		template <typename _OtherRep, typename _OtherPeriod>
		bool operator <= (const std::chrono::duration<_OtherRep, _OtherPeriod>& duration) const {
			return *this <= Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this);
		}
		template <typename _OtherRep, typename _OtherPeriod>
		bool operator >= (const std::chrono::duration<_OtherRep, _OtherPeriod>& duration) const {
			return *this >= Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this);
		}
		template <typename _OtherRep, typename _OtherPeriod>
		bool operator==(const std::chrono::duration<_OtherRep, _OtherPeriod>& duration) const {
			return *this == Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this);
		}
		template <typename _OtherRep, typename _OtherPeriod>
		bool operator!=(const std::chrono::duration<_OtherRep, _OtherPeriod>& duration) const {
			return *this != Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this);
		}

	private:
		intmax_t GetSafeDenumerator(intmax_t denumerator) {
			if (denumerator == 0) {
				LOG_WARNING_D("Was attempt dividing by zero (denumerator == 0)");
				Dbreak;
				return 1;
			}
			return denumerator;
		}

		double GetRationalValue() {
			if (den == 0) {
				throw std::exception("Divide by zero");
			}
			return static_cast<double>(num) / den;
		}

	private:
		const intmax_t num;
		const intmax_t den;
		const double rational;
		_Rep valueRep; // representation value
	};



	// TODO: overload for any integer types
	template<typename _Rep>
	bool operator < (const Rational<_Rep>& rational, int value) {
		return rational.Value() < value;
	}
	template<typename _Rep>
	bool operator > (const Rational<_Rep>& rational, int value) {
		return rational.Value() > value;
	}
	template<typename _Rep>
	bool operator <= (const Rational<_Rep>& rational, int value) {
		return rational.Value() <= value;
	}
	template<typename _Rep>
	bool operator >= (const Rational<_Rep>& rational, int value) {
		return rational.Value() >= value;
	}
	template<typename _Rep>
	bool operator==(const Rational<_Rep>& rational, int value) {
		return rational.Value() == value;
	}
	template<typename _Rep>
	bool operator!=(const Rational<_Rep>& rational, int value) {
		return rational.Value() != value;
	}
}


// 
// [Deprecated] [Backward compatibility] Rational.
//
template<class T>
class Rational {
public:
    Rational() = default;

    constexpr Rational(const T& num, const T& den)
        : num(num), den(den)
    {}

    template<class RationalT>
    Rational(const RationalT& r)
        : num(static_cast<T>(r.num)), den(static_cast<T>(r.den))
    {}

    bool operator==(const Rational& other) const {
        bool equ = this->den == other.den && this->num == other.num;
        return equ;
    }

    bool operator!=(const Rational& other) const {
        return !(*this == other);
    }

    // converts value with rounding +0.5
    T ConvertValueTo(const T& value, const Rational<T>& other) const {
        T d = this->den * other.num;
        /*
        perform +0.5 rounding to save info about fractional part converting in both directions
        It is what Media Foundation resampler does
        example:
            with rounding
                50(48k) -> 10416.6(hns) -> 10417hns
                130(48k) -> 27083.3(hns) -> 27083hns

                10417hns -> 50.0016(48k) -> 50(48k)
                27083hns -> 129.9984(48k) -> 130(48k)
            without rounding:
                50(48k) -> 10416.6(hns) -> 10416hns
                130(48k) -> 27083.3(hns) -> 27083hns

                10416hns -> 49.9968(48k) -> 49(48k)
                27083hns -> 129.9984(48k) -> 129(48k)
        */
        T res = Rational::Mul3Div(value, other.den, this->num, d / 2, d);
        return res;
    }

    // converts value with trunation of fractional part
    T ConvertTruncValueTo(const T& value, const Rational<T>& other) const {
        T d = this->den * other.num;
        T res = Rational::Mul3Div(value, other.den, this->num, d);
        return res;
    }

    // converts value with ceiling
    T ConvertCeilValueTo(const T& value, const Rational<T>& other) const {
        T d = this->den * other.num;
        T res = Rational::Mul3Div(value, other.den, this->num, d - 1, d);
        return res;
    }

    T num = T(1);
    T den = T(1);

private:
    /*
       Performs (a * b * c) / d minimizing chance for overflow even on big numbers.
       a should be greatest value of a,b,c, b middle, c smallest to have better precision
    */
    template<class T> static T Mul3Div(T a, T b, T c, T d) {
        T q = a / d;
        T r = a % d;

        T q1 = (r * b) / d;
        T r1 = (r * b) % d;

        T x1 = q * b * c;
        T x2 = q1 * c;
        T x3 = (r1 * c) / d;

        T res = x1 + x2 + x3;

        return res;
    }

    /*
        Performs ((a * b * c) + e) / d minimizing chance for overflow even on big numbers.
        a should be greatest value of a,b,c, b middle, c smallest to have better precision
    */
    template<class T> static T Mul3Div(T a, T b, T c, T e, T d) {
        T q = a / d;
        T r = a % d;

        T q1 = (r * b) / d;
        T r1 = (r * b) % d;

        T x1 = q * b * c;
        T x2 = q1 * c;
        T x3 = (r1 * c + e) / d;

        T res = x1 + x2 + x3;

        return res;
    }
};