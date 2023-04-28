#pragma once

#include <iec_parser.h>
#include <stdexcept>

namespace net
{

/// \brief Enumeration for use as a validator state.
enum class State : ui8
{
    Initial = 0,
    Correct,
    Incorrect
};

/// \brief Class for validating output data from IecParser.
/// \see IecParser.
class Validator
{
private:
    static constexpr ui16 min = 0;        ///< Minimum value of the sample value count.
    static constexpr ui16 max80p = 3999;  ///< Maximum value of the sample value count for 80 points per period.
    static constexpr ui16 max256p = 5000; ///< Maximum value of the sample value count for 256 points per period.

    ui16 value;
    State state;

    /// \brief Updating the validator state by checking the stored value and misses.
    /// \details If the difference between the previous value and the new value is
    /// greater than the specified difference, then the validator state changes to incorrect.
    /// Otherwise, the validator state changes to correct. Checking are performed taking
    /// into consideration the boundary values.
    void update(const ui16 newValue, const ui8 diff) noexcept;

    /// \brief Checking the validator state.
    /// \details If the validator state is incorrect an exception will be thrown.
    void validate();

public:
    explicit Validator();

    /// \brief Updating validator state by the sequnce of ASDU given by IecParser.
    void update(const iec::SeqASDU &sequnce);
};

}
