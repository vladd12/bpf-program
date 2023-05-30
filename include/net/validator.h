#pragma once

#include <iec_parser.h>

namespace net
{

/// \brief Enumeration for use as a validator state.
enum class State : ui8
{
    Initial = 0,
    Correct,
    Incorrect
};

/// \brief Enumeration for use as a validator strategy.
enum class Strategy : ui8
{
    ThrowException = 0,
    Statistics
};

/// \brief Class for validating output data from IecParser.
/// \see IecParser.
class Validator
{
private:
    static constexpr ui16 min = 0;         ///< Minimum value of the sample value count.
    static constexpr ui16 max80p = 3999;   ///< Maximum value of the sample value count for 80 points per period.
    static constexpr ui16 max256p = 12799; ///< Maximum value of the sample value count for 256 points per period.

    ui16 value;
    State state;
    Strategy strategy;

    /// \brief Updating the validator state by checking the stored value and misses.
    /// \details If the difference between the previous value and the new value is
    /// greater than the specified difference, then the validator state changes to incorrect.
    /// Otherwise, the validator state changes to correct. Checking are performed taking
    /// into consideration the boundary values.
    void validate(const ui16 svID, const ui8 count);

public:
    ui32 missedCount;
    ui64 capturedCount;

    explicit Validator();

    /// Setter for a validator strategy.
    void setStrategy(Strategy newStrategy) noexcept;

    /// \brief Updating validator state by the sequnce of ASDU given by IecParser.
    void update(const iec::SeqASDU &sequnce);

    /// \brief Reset a validator's state and packet counters.
    void reset() noexcept;
};

}
