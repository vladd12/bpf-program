#include "iec_core/handlers/number_crusher.h"

#include <algorithm>
#include <limits>

namespace handlers
{

constexpr auto f32min = std::numeric_limits<f32>::min();
constexpr auto f32max = std::numeric_limits<f32>::max();
constexpr auto u64max = std::numeric_limits<u64>::max();

void NumberCrusher::run()
{
    while (running)
    {
        exchange->get(buffer);
        auto sequence { parser.parse(buffer) };
        validator.update(sequence);
        parser.convert(sequence, points);
        if (points.size() > 512)
        {
            findMinMax(points);
            points.clear();
        }
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
    minArray.fill(f32max);
    maxArray.fill(f32min);
    for (auto &point : points)
    {
        for (auto index = 0; index < size; ++index)
        {
            minArray[index] = std::min(minArray[index], point.values[index]);
            maxArray[index] = std::max(maxArray[index], point.values[index]);
        }
    }

    // выбор наибольшего из сигналов по размаху для синхронизации
    f32 magnitude = f32min;
    std::size_t selected = 0;
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

    // определяем знак сигнала в первой точке
    i8 sign = (points[0].values[selected] - adcMid[selected] < 0) ? -1 : 1;
    std::array<u64, 3> periods { u64max, u64max, u64max };
    std::size_t period_index = 0;
    for (std::size_t index = 1; index < points.size(); ++index)
    {
        // находим переходы через ноль
        auto position = points[index].values[selected] - adcMid[selected];
        if (position * sign < 0)
        {
            periods[period_index] = index;
            ++period_index;
            if (period_index == 3)
                break;
        }
    }

    // не найден первый переход через ноль!
    if (periods[0] == u64max)
        return;
    // нет противоположного перехода через ноль (велик последний полупериод)!
    if (periods[1] == u64max)
        return;
    // нет второго одноименного перехода через ноль (велик период)!
    if (periods[2] == u64max)
        return;

    printf("Step: %lu\t\tNum points: %lu\n", count, points.size());
    printf("Mins:\nIA: %f\nIB: %f\nIC: %f\nIN: %f\n", minArray[0], minArray[1], minArray[2], minArray[3]);
    printf("UA: %f\nUB: %f\nUC: %f\nUN: %f\n", minArray[4], minArray[5], minArray[6], minArray[7]);
    printf("Maxs:\nIA: %f\nIB: %f\nIC: %f\nIN: %f\n", maxArray[0], maxArray[1], maxArray[2], maxArray[3]);
    printf("UA: %f\nUB: %f\nUC: %f\nUN: %f\n", maxArray[4], maxArray[5], maxArray[6], maxArray[7]);
    printf("Magnitude: %f\n\n", magnitude);
    ++count;
}

} // namespace handlers
