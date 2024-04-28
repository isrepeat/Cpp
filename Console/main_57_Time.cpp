#define HELPERS_NS_ALIAS HH
#include <Windows.h>
#include <Helpers/Logger.h>
#include <chrono>

using namespace std::chrono_literals;

void TestCast() {
    LOG_FUNCTION_ENTER("TestCast()");

    std::chrono::milliseconds milliSec{ 1 };

    auto castToMilliSec = std::chrono::duration_cast<std::chrono::milliseconds>(milliSec);
    auto castToSec = std::chrono::duration_cast<std::chrono::seconds>(milliSec);
    LOG_DEBUG_D("castToSec = {}", castToSec.count());
    LOG_DEBUG_D("castToMilliSec = {}", castToMilliSec.count());

    auto castToFloatMilliSec = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(milliSec);
    auto castToFloatSec = std::chrono::duration_cast<std::chrono::duration<float>>(milliSec);
    LOG_DEBUG_D("castToFloatMilliSec = {}", castToFloatMilliSec.count());
    LOG_DEBUG_D("castToFloatSec = {}", castToFloatSec.count());

    auto castFromFloatSecToMilliSec = std::chrono::duration_cast<std::chrono::milliseconds>(castToFloatSec);
    LOG_DEBUG_D("castFromFloatSecToMilliSec = {}", castFromFloatSecToMilliSec.count());
}



//
// Rational
//
namespace H {
    template <typename _Rep>
    struct Rational {

        // This struct helps you to initialize many similar Rational-objects with same {num, den}
        struct Units {
            intmax_t num = 0;
            intmax_t den = 1;
        };

        Rational(Units units)
            : num{ units.num }
            , den{ units.den }
            , rational{ this->GetRationalValue() }
        {}

        Rational(intmax_t num, intmax_t den, _Rep valueRep = 1)
            : num{ num }
            , den{ den }
            , rational{ this->GetRationalValue() }
            , valueRep{ valueRep }
        {}

        template <typename _OtherRep, typename _OtherPeriod>
        Rational(intmax_t num, intmax_t den, const std::chrono::duration<_OtherRep, _OtherPeriod>& duration)
            : num{ num }
            , den{ den }
            , rational{ this->GetRationalValue() }
            , valueRep{ Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this).Value() }
        {}

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
            if (this->num == other.num && this->den == other.den) {
                return Rational<_ToRep>(this->num, this->den, this->valueRep);
            }
            else {
                return Rational<_ToRep>(
                    other.Numerator(),
                    other.Denumerator(),
                    std::round(this->valueRep * this->As<double>() / other.As<double>()));
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
            this->valueRep = Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this).Value();
            return *this;
            //return this->operator=(Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this));
        }

        //
        // Arithmetic operators
        //
        Rational operator+(const Rational& other) {
            return Rational{ this->num, this->den, this->valueRep + other.CastToRational<_Rep>(*this).Value() };
        }
        Rational operator-(const Rational& other) {
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
        Rational& operator+=(const std::chrono::duration<_OtherRep, _OtherPeriod>& duration) const {
            return *this += Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this);
        }
        template <typename _OtherRep, typename _OtherPeriod>
        Rational& operator-=(const std::chrono::duration<_OtherRep, _OtherPeriod>& duration) const {
            return *this += Rational<_OtherRep>{ _OtherPeriod::num, _OtherPeriod::den, duration.count() }.CastToRational<_Rep>(*this);
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



    template <intmax_t _Nx, intmax_t _Dx = 1>
    struct Ratio : std::ratio<_Nx, _Dx> {
        using _MyBase = std::ratio<_Nx, _Dx>;

        static constexpr double value = (double)_MyBase::num / _MyBase::den;
        static constexpr Ratio<_Dx, _Nx> inversed() { return Ratio<_Dx, _Nx>{}; };
    };

    template <typename ValueT, intmax_t _Nx, intmax_t _Dx>
    constexpr std::chrono::duration<ValueT, std::ratio<_Nx, _Dx>> operator*(ValueT value, H::Ratio<_Nx, _Dx> /*ratio*/) {
        return std::chrono::duration<ValueT, std::ratio<_Nx, _Dx>>(value);
    }
}


//
// Hns
//
namespace H {
    namespace Chrono {
        template <class _Duration, class _ToRep>
        struct CastTo { // mb rename
            using type = std::chrono::duration<_ToRep, typename _Duration::period>;
        };

        using milliseconds_f = typename CastTo<std::chrono::milliseconds, float>::type;
        using seconds_f = typename CastTo<std::chrono::seconds, float>::type;

        template <class _To, class _Rep, class _Period>
        constexpr const std::chrono::duration<float, typename _To::period> duration_cast_float(const std::chrono::duration<_Rep, _Period>& _Dur) noexcept {
            return std::chrono::duration_cast<std::chrono::duration<float, typename _To::period>>(_Dur);
        }

        template <typename _Rep, typename _Period>
        struct DurationBase : std::chrono::duration<_Rep, _Period> {
            using _MyBase = std::chrono::duration<_Rep, _Period>;
            using _MyBase::duration;

            template <typename _OtherRep>
            DurationBase(Rational<_OtherRep> rational)
                : _MyBase::duration{ rational.CastToRational<_Rep>(this->ToRational()).Value() }
            {}

            operator uint64_t() const {
                return this->count();
            }
            operator int64_t() const {
                return this->count();
            }
            operator uint32_t() const {
                return this->count();
            }
            operator int32_t() const {
                return this->count();
            }
            explicit operator float() const {
                return duration_cast_float<_MyBase>(*this).count();
            }
            explicit operator double() const {
                // TODO: add duration_cast_double
                return duration_cast_float<_MyBase>(*this).count();
            }

            Rational<_Rep> ToRational() const {
                return { _Period::num, _Period::den, this->count() };
            }

            template <typename _OtherRep>
            Rational<_Rep> CastToRational(Rational<_OtherRep> other) const {
                return this->ToRational().CastToRational<_Rep>(other);
            }
        };

        template <typename _Rep, typename _Period>
        constexpr DurationBase<_Rep, _Period> operator+(const DurationBase<_Rep, _Period>& _Left, const DurationBase<_Rep, _Period>& _Right) noexcept {
            return std::chrono::operator+(_Left, _Right);
        }

        template <typename _Rep, typename _Period>
        constexpr DurationBase<_Rep, _Period> operator-(const DurationBase<_Rep, _Period>& _Left, const DurationBase<_Rep, _Period>& _Right) noexcept {
            return std::chrono::operator-(_Left, _Right);
        }

        using Hns = DurationBase<long long, std::ratio<1, 10'000'000>>; // Do not use unsigned bacause may be sideeffects when cast to float
    }

    inline namespace Literals {
        inline namespace ChronoLiterals {
            constexpr Chrono::Hns operator"" __hns(unsigned long long _Val) noexcept {
                return Chrono::Hns(_Val);
            }
        }
    }
}

using namespace H::ChronoLiterals;



template<class T>
T Clamp(T value, T min, T max) {
    T res = (std::min)((std::max)(min, value), max);
    return res;
}

void TestHNS() {
    LOG_FUNCTION_ENTER("TestHNS()");
    
    constexpr auto countHns = 1'000'000__hns;
    constexpr auto countMs = 1ms;
    constexpr auto countS = 1s;

    //static_assert(countMs == countS, "");

    H::Chrono::Hns hns_1{ 10 };
    H::Chrono::Hns hns_2{ 35 };

    auto xxx = (float)(hns_1 - hns_2);

    int64_t hnsCastToInt = hns_1;
    float hnsCastToFloat = (double)hns_1;

    Clamp<H::Chrono::Hns>(hns_1, hns_1, countMs);

    LOG_DEBUG_D("countHns = {}", countHns.count());

    auto castFromHnsToMilliSec = std::chrono::duration_cast<std::chrono::milliseconds>(countHns);
    auto castFromHnsToSec = std::chrono::duration_cast<std::chrono::seconds>(countHns);
    LOG_DEBUG_D("castFromHnsToMilliSec = {}", castFromHnsToMilliSec.count());
    LOG_DEBUG_D("castFromHnsToSec = {}", castFromHnsToSec.count());

    auto castFromHnsToFloatMilliSec = H::Chrono::duration_cast_float<std::chrono::milliseconds>(countHns);
    auto castFromHnsToFloatSec = H::Chrono::duration_cast_float<std::chrono::seconds>(countHns);
    LOG_DEBUG_D("castFromHnsToFloatMilliSec = {}", castFromHnsToFloatMilliSec.count());
    LOG_DEBUG_D("castFromHnsToFloatSec = {}", castFromHnsToFloatSec.count());

    auto castFromFloatMilliSecToHns = H::Chrono::duration_cast_float<H::Chrono::Hns>(castFromHnsToFloatMilliSec);
    auto castFromFloatSecToHns = H::Chrono::duration_cast_float<H::Chrono::Hns>(castFromHnsToFloatSec);
    LOG_DEBUG_D("castFromFloatMilliSecToHns = {}", castFromFloatMilliSecToHns.count());
    LOG_DEBUG_D("castFromFloatSecToHns = {}", castFromFloatSecToHns.count());
}


void TestArithmetics() {
    LOG_FUNCTION_ENTER("TestArithmetics()");
    constexpr auto tpA = 100ms;
    constexpr auto tpB = 120ms;
    constexpr auto tpC = 200_hns;
    constexpr auto tpD = 400_hns;

    //static_assert(tpA - tpB == -20ms);
    //static_assert(tpC - tpD == -40_hns);

    LOG_DEBUG_D("tpA = {}ms", H::Chrono::duration_cast_float<std::chrono::milliseconds>(tpA).count());
    LOG_DEBUG_D("tpB = {}ms", H::Chrono::duration_cast_float<std::chrono::milliseconds>(tpB).count());
    LOG_DEBUG_D("tpC = {}ms = {}hns", H::Chrono::duration_cast_float<std::chrono::milliseconds>(tpC).count(), tpC.count());
    LOG_DEBUG_D("tpD = {}ms = {}hns", H::Chrono::duration_cast_float<std::chrono::milliseconds>(tpD).count(), tpD.count());

    LOG_DEBUG_D("   tpB - tpA = {}ms", (tpB - tpA).count());
    LOG_DEBUG_D("   tpA - tpB = {}ms", (tpA - tpB).count());

    //auto dtDC = H::Chrono::duration_cast_float<std::chrono::milliseconds>(tpD - tpC);
    //auto dtCD = H::Chrono::duration_cast_float<std::chrono::milliseconds>(tpC - tpD);

    auto dtDC = tpD - tpC;
    auto dtCD = tpC - tpD;
    auto dtDCms = H::Chrono::duration_cast_float<std::chrono::milliseconds>(dtDC);
    auto dtCDms = H::Chrono::duration_cast_float<std::chrono::milliseconds>(dtCD);

    LOG_DEBUG_D("1) tpD - tpC = {}hns = {}hns", (tpD - tpC).count(), (int64_t)((tpD - tpC).count()));
    LOG_DEBUG_D("2) tpD - tpC = {}hns = {}hns", dtDC.count(), (int64_t)(dtDC.count()));
    LOG_DEBUG_D("3) dtDC = {}ms", dtDCms.count());
    LOG_DEBUG_D("");
    LOG_DEBUG_D("1) tpC - tpD = {}hns = {}hns", (tpC - tpD).count(), (int64_t)((tpC - tpD).count()));
    LOG_DEBUG_D("2) tpC - tpD = {}hns = {}hns", dtCD.count(), (int64_t)(dtCD.count()));
    LOG_DEBUG_D("3) dtCD = {}ms", dtCDms.count());// , ()(dtCDms.count()));

    LOG_DEBUG_D("");
    //LOG_DEBUG_D("1) |tpC - tpD| < 25ms = {}", (tpC - tpD).count() < 25);
    LOG_DEBUG_D("   |tpC - tpD| < 25ms = {}", std::abs((int64_t)(dtCDms.count())) < 25);

    return;
}


template <class _To, class _Rep, class _Period>
constexpr _To duration_cast(const std::chrono::duration<_Rep, _Period>& _Dur) noexcept(
    is_arithmetic_v<_Rep>&& is_arithmetic_v<typename _To::rep>) /* strengthened */ {
    // convert duration to another duration; truncate
    using _CF = std::ratio_divide<_Period, typename _To::period>;

    using _ToRep = typename _To::rep;
    using _CR = std::common_type_t<_ToRep, _Rep, intmax_t>;

    constexpr bool _Num_is_one = _CF::num == 1;
    constexpr bool _Den_is_one = _CF::den == 1;

    if (_Den_is_one) {
        if (_Num_is_one) {
            return static_cast<_To>(static_cast<_ToRep>(_Dur.count()));
        }
        else {
            return static_cast<_To>(
                static_cast<_ToRep>(static_cast<_CR>(_Dur.count()) * static_cast<_CR>(_CF::num)));
        }
    }
    else {
        if (_Num_is_one) {
            return static_cast<_To>(
                static_cast<_ToRep>(static_cast<_CR>(_Dur.count()) / static_cast<_CR>(_CF::den)));
        }
        else {
            return static_cast<_To>(static_cast<_ToRep>(
                static_cast<_CR>(_Dur.count()) * static_cast<_CR>(_CF::num) / static_cast<_CR>(_CF::den)));
        }
    }
}



void TestRational() {
    H::Rational<int64_t> rat1{ 1, 1000, 1 };
    H::Rational<int64_t> rat2{ 60, 1 };

    bool res = rat1 < rat2;

    std::max(rat1, rat2);
    //auto rat1CastedToRat2 = rat1.CastToRational<float>(rat2);
    //auto valueCastedToRat2 = rat1CastedToRat2.Value();

    //auto resCastedToRat1 = rat1CastedToRat2.CastToRational<float>(rat1);
    //auto valueOrigin = resCastedToRat1.Value();

    int audioSampleRate = 48'000;
    H::Chrono::Hns audioDurationHns = 333'333__hns;
    //{
    //    H::Rational<int64_t> audioDuration = audioDurationHns.CastToRational(H::Rational<int64_t>{ 1, audioSampleRate });
    //    H::Chrono::Hns audioDurationHns_ = audioDuration;
    //    LOG_DEBUG_D("audioDuration.Value() = {}, audioDurationHns_.count() = {}"
    //        , audioDuration.Value()
    //        , audioDurationHns_.count()
    //    );
    //}

    {
        H::Rational<int64_t> audioDuration{ 1, audioSampleRate };
        audioDuration = audioDurationHns;
        H::Chrono::Hns audioDurationHns_ = audioDuration;
        LOG_DEBUG_D("audioDuration.Value() = {}, audioDurationHns_.count() = {}"
            , audioDuration.Value()
            , audioDurationHns_.count()
        );
    }

    NOOP;
    //using ratMs = H::Ratio<1, 1000>;
    //using ratMin = H::Ratio<60, 1>;

    //constexpr int countMs = 5;
    //constexpr auto countMsInSec = 1 * ratMs{};
    //constexpr auto countMsInMin = 1 * ratMin{};

    ////auto inversedMin = H::Rational<60, 1>::inversed();

    //////using ratMs = std::ratio<1, 1000>;
    //////using ratMin = std::ratio<60, 1>;
    //using _CF = std::ratio_divide<ratMs::_MyBase, ratMin::_MyBase>;

    //_CF::num;
    //_CF::den;
}


int main() {
    HH::Flags<lg::InitFlags> loggerInitFlags =
        lg::InitFlags::DefaultFlags |
        lg::InitFlags::EnableLogToStdout;

    lg::DefaultLoggers::Init(L"D:\\main_57_Time.log", loggerInitFlags);

    //TestCast();
    //TestHNS();
    //TestArithmetics();
    TestRational();

    Sleep(100'000);
    return 0;
}