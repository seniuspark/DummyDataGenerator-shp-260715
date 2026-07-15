#include "OrderGenerator.h"

#include <array>
#include <stdexcept>

namespace
{
    const char* const kCustomerNamePrefixes[] = {
        "SK-Hynix", "Samsung-Elec", "LG-Innotek", "DB-HiTek", "Amkor-Tech",
    };

    constexpr size_t kCustomerNamePrefixCount =
        sizeof(kCustomerNamePrefixes) / sizeof(kCustomerNamePrefixes[0]);

    constexpr std::array<OrderStatus, 5> kAllStatuses = {
        OrderStatus::Reserved,
        OrderStatus::Rejected,
        OrderStatus::Producing,
        OrderStatus::Confirmed,
        OrderStatus::Release,
    };

    std::chrono::system_clock::time_point PastByHours(
        std::mt19937& rng, std::chrono::system_clock::time_point now, double minHours, double maxHours)
    {
        std::uniform_real_distribution<double> hoursDist(minHours, maxHours);
        auto elapsed = std::chrono::duration_cast<std::chrono::system_clock::duration>(
            std::chrono::duration<double, std::ratio<3600>>(hoursDist(rng)));
        return now - elapsed;
    }

    std::chrono::system_clock::time_point GenerateCreatedAt(
        std::mt19937& rng, OrderStatus status, std::chrono::system_clock::time_point now)
    {
        switch (status)
        {
        case OrderStatus::Release:
        case OrderStatus::Confirmed:
            return PastByHours(rng, now, 24.0, 24.0 * 30.0);
        case OrderStatus::Producing:
            return PastByHours(rng, now, 0.0, 24.0 * 3.0);
        case OrderStatus::Reserved:
            return PastByHours(rng, now, 0.0, 6.0);
        case OrderStatus::Rejected:
        default:
            return PastByHours(rng, now, 0.0, 24.0 * 30.0);
        }
    }
}

DummyOrder GenerateOrder(
    std::mt19937& rng,
    const std::vector<DummySample>& samples,
    std::chrono::system_clock::time_point now)
{
    if (samples.empty())
    {
        throw std::invalid_argument("GenerateOrder: samples must not be empty");
    }

    std::uniform_int_distribution<size_t> sampleDist(0, samples.size() - 1);
    std::uniform_int_distribution<size_t> statusDist(0, kAllStatuses.size() - 1);
    std::uniform_int_distribution<int> quantityDist(1, 100);
    std::uniform_int_distribution<size_t> prefixDist(0, kCustomerNamePrefixCount - 1);
    std::uniform_int_distribution<int> suffixDist(1, 999);

    DummyOrder order;
    order.sampleId = samples[sampleDist(rng)].sampleId;
    order.status = kAllStatuses[statusDist(rng)];
    order.quantity = quantityDist(rng);
    order.customerName =
        std::string(kCustomerNamePrefixes[prefixDist(rng)]) + "-" + std::to_string(suffixDist(rng));
    order.createdAt = GenerateCreatedAt(rng, order.status, now);

    return order;
}
