// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_TIMESTAMP_H
#define MUDUO_BASE_TIMESTAMP_H

#include <boost/operators.hpp>

///
/// Time stamp in UTC, in microseconds resolution.
///
/// This class is immutable.
/// It's recommended to pass it by value, since it's passed in register on x64.
///
class Timestamp : public boost::equality_comparable<Timestamp>,
                  public boost::less_than_comparable<Timestamp>
{
public:
    ///
    /// Constucts an invalid Timestamp.
    ///
    Timestamp()
        : microSecondsSinceEpoch_(0)
    {
    }

    ///
    /// Constucts a Timestamp at specific time
    ///
    /// @param microSecondsSinceEpochArg
    explicit Timestamp(const int64_t microSecondsSinceEpochArg)
        : microSecondsSinceEpoch_(microSecondsSinceEpochArg)
    {
    }

    void swap(Timestamp& that) noexcept
    {
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }

    // default copy/assignment/dtor are Okay

    [[nodiscard]] std::string toString() const;
    [[nodiscard]] std::string toFormattedString(bool showMicroseconds = true) const;

    [[nodiscard]] bool valid() const { return microSecondsSinceEpoch_ > 0; }

    // for internal usage.
    [[nodiscard]] int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

    [[nodiscard]] time_t secondsSinceEpoch() const
    {
        return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    }

    ///
    /// Get time of now.
    ///
    static Timestamp now();

    static Timestamp invalid()
    {
        return {};
    }

    static Timestamp fromUnixTime(const time_t t)
    {
        return fromUnixTime(t, 0);
    }

    static Timestamp fromUnixTime(const time_t t, const int microseconds)
    {
        return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
    }

    static constexpr int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator<(const Timestamp lhs, const Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(const Timestamp lhs, const Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

///
/// Gets time difference of two timestamps, result in seconds.
///
/// @param high, low
/// @return (high-low) in seconds
/// @c double has 52-bit precision, enough for one-microsecond
/// resolution for next 100 years.
inline double timeDifference(const Timestamp high, const Timestamp low)
{
    const int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

///
/// Add @c seconds to given timestamp.
///
/// @return timestamp+seconds as Timestamp
///
inline Timestamp addTime(const Timestamp timestamp, const double seconds)
{
    const auto delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

#endif  // MUDUO_BASE_TIMESTAMP_H
