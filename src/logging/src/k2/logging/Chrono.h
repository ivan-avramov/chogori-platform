/*
MIT License

Copyright(c) 2020 Futurewei Cloud

    Permission is hereby granted,
    free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :

    The above copyright notice and this permission notice shall be included in all copies
    or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS",
    WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER
    LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#pragma once
#include <k2/logging/FormattingUtils.h>

#include <chrono>
#include <iostream>

using namespace std::chrono_literals;  // so that we can type "1ms"

namespace k2 {

typedef std::chrono::steady_clock Clock;
typedef Clock::duration Duration;
typedef std::chrono::time_point<Clock> TimePoint;

// Get particular resolution for a Duration, or for total elapsed time for a TimePoint
inline std::chrono::nanoseconds nsec(const Duration& dur) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(dur);
}
inline std::chrono::nanoseconds nsec(const TimePoint& tp) {
    return nsec(tp.time_since_epoch());
}
inline std::chrono::microseconds usec(const Duration& dur) {
    return std::chrono::duration_cast<std::chrono::microseconds>(dur);
}
inline std::chrono::microseconds usec(const TimePoint& tp) {
    return usec(tp.time_since_epoch());
}
inline std::chrono::milliseconds msec(const Duration& dur) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(dur);
}
inline std::chrono::milliseconds msec(const TimePoint& tp) {
    return msec(tp.time_since_epoch());
}
inline std::chrono::seconds sec(const Duration& dur) {
    return std::chrono::duration_cast<std::chrono::seconds>(dur);
}
inline std::chrono::seconds sec(const TimePoint& tp) {
    return sec(tp.time_since_epoch());
}

template <typename ClockT = Clock>
class Deadline {
public:
    Deadline(typename ClockT::duration dur) : _deadline(ClockT::now() + dur) {}

    typename ClockT::duration getRemaining() const {
        auto now = ClockT::now();
        if (now >= _deadline) {
            return typename ClockT::duration(0);
        }
        return _deadline - now;
    }

    bool isOver() const {
        return ClockT::now() >= _deadline;
    }

private:
    typename ClockT::time_point _deadline;
};  // class Deadline

struct Timestamp_ts {
    uint16_t micros;
    uint16_t millis;
    uint8_t secs;
    uint8_t mins;
    uint8_t hours;
    uint16_t days;
};

inline Timestamp_ts toTimestamp_ts(const TimePoint& tp) {
    auto now = usec(tp).count();
    auto [quotient1, micros] = std::ldiv(now, 1000);
    auto [quotient2, millis] = std::ldiv(quotient1, 1000);
    auto [quotient3, secs] = std::ldiv(quotient2, 60);
    auto [quotient4, mins] = std::div((int)quotient3, 60);  // the quotient is small enough to fit in an int now
    auto [days, hours] = std::div(quotient4, 24);
    return {(uint16_t)micros, (uint16_t)millis, (uint8_t)secs, (uint8_t)mins, (uint8_t)hours, (uint16_t)days};
}
}  // ns k2

// formatters
template <>
struct fmt::formatter<k2::Timestamp_ts> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(k2::Timestamp_ts const& ts, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), FMT_COMPILE("{:04}:{:02}:{:02}:{:02}.{:03}.{:03}"), ts.days, ts.hours, ts.mins, ts.secs, ts.millis, ts.micros);
    }
};

template <>
struct fmt::formatter<k2::Duration> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(k2::Duration const& dur, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", k2::TimePoint{} + dur);
    }
};

//std::chrono::duration<long int, std::ratio<1, 1000000000> >

template <typename X, typename Y>
struct fmt::formatter<std::chrono::duration<X,Y>> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(std::chrono::duration<X, Y> const& dur, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", k2::TimePoint{} + dur);
    }
};

template <>
struct fmt::formatter<k2::TimePoint> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(k2::TimePoint const& tp, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", k2::toTimestamp_ts(tp));
    }
};
