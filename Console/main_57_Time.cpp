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




namespace H {
    namespace Chrono {
        template <class _To, class _Rep, class _Period>
        constexpr const std::chrono::duration<float, typename _To::period> duration_cast_float(const std::chrono::duration<_Rep, _Period>& _Dur) noexcept {
            return std::chrono::duration_cast<std::chrono::duration<float, typename _To::period>>(_Dur);
        }

        template <typename _Rep, typename _Period>
        struct DurationBase : std::chrono::duration<_Rep, _Period> {
            using _MyBase = std::chrono::duration<_Rep, _Period>;
            using _MyBase::duration;

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
        };

        template <typename _Rep, typename _Period>
        constexpr DurationBase<_Rep, _Period> operator+(const DurationBase<_Rep, _Period>& _Left, const DurationBase<_Rep, _Period>& _Right) noexcept {
            return std::chrono::operator+(_Left, _Right);
        }

        template <typename _Rep, typename _Period>
        constexpr DurationBase<_Rep, _Period> operator-(const DurationBase<_Rep, _Period>& _Left, const DurationBase<_Rep, _Period>& _Right) noexcept {
        	return std::chrono::operator-(_Left, _Right);
        }

        using Hns = DurationBase<long long, std::ratio<1, 10'000'000>>;
        //using Hns = std::chrono::duration<unsigned long long, std::ratio<1, 10'000'000>>;
    }

    inline namespace Literals {
        inline namespace ChronoLiterals {
            constexpr Chrono::Hns operator"" _hns(unsigned long long _Val) noexcept {
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
    
    constexpr auto countHns = 1'000'000_hns;
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

int main() {
    HH::Flags<lg::InitFlags> loggerInitFlags =
        lg::InitFlags::DefaultFlags |
        lg::InitFlags::EnableLogToStdout;

    lg::DefaultLoggers::Init(L"D:\\main_57_Time.log", loggerInitFlags);

    //TestCast();
    //TestHNS();
    TestArithmetics();

    Sleep(100'000);
    return 0;
}