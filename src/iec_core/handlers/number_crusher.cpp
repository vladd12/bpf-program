#include "iec_core/handlers/number_crusher.h"

#include <algorithm>
#include <limits>

namespace handlers
{

void NumberCrusher::run()
{
    while (running)
    {
        exchange->get(buffer);
        auto sequence { parser.parse(buffer) };
        validator.update(sequence);
        auto points { parser.convert(sequence) };
        findMinMax(points);
    }
}

void NumberCrusher::setExchange(Exchange &exchange_) noexcept
{
    exchange = &exchange_;
}

void NumberCrusher::findMinMax(const std::vector<iec::Point> &points)
{
    static std::size_t count = 1;

    constexpr auto size = iec::unitsPerASDU;
    // определяем минимальные и максимальные значения всех сигналов
    std::array<f32, size> minArray, maxArray;
    minArray.fill(std::numeric_limits<f32>::max());
    maxArray.fill(std::numeric_limits<f32>::min());
    for (auto &point : points)
    {
        for (auto index = 0; index < size; ++index)
        {
            minArray[index] = std::min(minArray[index], point.values[index]);
            maxArray[index] = std::max(maxArray[index], point.values[index]);
        }
    }

    // выбор наибольшего из сигналов по размаху для синхронизации
    f32 magnitude = std::numeric_limits<f32>::min();
    [[maybe_unused]] std::size_t selected = 0;
    std::array<f32, size> adcMid;
    for (auto index = 0; index < size; ++index)
    {
        auto diff = maxArray[index] - minArray[index];
        if (diff > magnitude)
        {
            selected = index;
            magnitude = diff;
        }
        adcMid[index] = (maxArray[index] + minArray[index]) / 2;
    }

    /// TODO
    //    // определяем знак сигнала в первой точке
    //    i8 sign = (points[0].values[selected] >= 0) ? 1 : -1;
    //    for (std::size_t index = 0; index < points.size(); ++index)
    //    {
    //        auto position = points[index].values[selected] - adcMid[selected];
    //        if (position * sign < 0)
    //        {
    //            ;
    //        }
    //    }

    printf("Step: %lu\t\tNum points: %lu\n", count, points.size());
    printf("Mins:\nIA: %f\nIB: %f\nIC: %f\nIN: %f\n", minArray[0], minArray[1], minArray[2], minArray[3]);
    printf("UA: %f\nUB: %f\nUC: %f\nUN: %f\n", minArray[4], minArray[5], minArray[6], minArray[7]);
    printf("Maxs:\nIA: %f\nIB: %f\nIC: %f\nIN: %f\n", maxArray[0], maxArray[1], maxArray[2], maxArray[3]);
    printf("UA: %f\nUB: %f\nUC: %f\nUN: %f\n", maxArray[4], maxArray[5], maxArray[6], maxArray[7]);
    printf("Magnitude: %f\n\n", magnitude);
    ++count;
}

} // namespace handlers
