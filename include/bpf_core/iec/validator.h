#pragma once

#include <bpf_core/iec/iec_parser.h>

namespace iec
{

/// \brief Enumeration for use as a validator state.
enum class State : u8
{
    Initial = 0,
    Correct,
    Incorrect
};

/// \brief Enumeration for use as a validator strategy.
enum class Strategy : u8
{
    ThrowException = 0,
    Statistics
};

/// \brief Class for validating output data from IecParser.
/// \see IecParser.
class Validator
{
private:
    static constexpr u16 min = 0;         ///< Minimum value of the sample value count.
    static constexpr u16 max80p = 3999;   ///< Maximum value of the sample value count for 80 points per period.
    static constexpr u16 max256p = 12799; ///< Maximum value of the sample value count for 256 points per period.

    u16 value;
    State state;
    Strategy strategy;

    /// \brief Updating the validator state by checking the stored value and misses.
    /// \details If the difference between the previous value and the new value is
    /// greater than the specified difference, then the validator state changes to incorrect.
    /// Otherwise, the validator state changes to correct. Checking are performed taking
    /// into consideration the boundary values.
    void validate(const u16 svID, const u8 count);

public:
    u32 missedCount;
    u64 capturedCount;

    explicit Validator() noexcept;

    /// Setter for a validator strategy.
    void setStrategy(Strategy newStrategy) noexcept;

    /// \brief Updating validator state by the sequnce of ASDU given by IecParser.
    void update(const iec::SeqASDU &sequnce);

    /// \brief Reset a validator's state and packet counters.
    void reset() noexcept;
};

} // namespace iec
