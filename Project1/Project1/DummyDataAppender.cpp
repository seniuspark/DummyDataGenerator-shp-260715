#include "DummyDataAppender.h"

#include <stdexcept>

#include "IdAllocator.h"
#include "JsonIo.h"
#include "OrderGenerator.h"
#include "SampleGenerator.h"

namespace
{
    std::vector<std::string> CollectSampleIds(const std::vector<DummySample>& samples)
    {
        std::vector<std::string> ids;
        ids.reserve(samples.size());
        for (const auto& sample : samples)
        {
            ids.push_back(sample.sampleId);
        }
        return ids;
    }

    std::vector<std::string> CollectOrderIds(const std::vector<DummyOrder>& orders)
    {
        std::vector<std::string> ids;
        ids.reserve(orders.size());
        for (const auto& order : orders)
        {
            ids.push_back(order.orderId);
        }
        return ids;
    }
}

AppendResult AppendDummySamples(
    const std::filesystem::path& samplesFile,
    int count,
    std::mt19937& rng,
    bool clearExisting)
{
    std::vector<DummySample> existing = clearExisting ? std::vector<DummySample>{} : LoadSamples(samplesFile);
    std::vector<std::string> existingIds = CollectSampleIds(existing);

    AppendResult result;
    for (int i = 0; i < count; ++i)
    {
        DummySample sample = GenerateSample(rng);
        sample.sampleId = NextSampleId(existingIds);
        existingIds.push_back(sample.sampleId);
        result.addedIds.push_back(sample.sampleId);
        existing.push_back(sample);
    }

    SaveSamples(samplesFile, existing);
    return result;
}

AppendResult AppendDummyOrders(
    const std::filesystem::path& samplesFile,
    const std::filesystem::path& ordersFile,
    int count,
    std::mt19937& rng,
    std::chrono::system_clock::time_point now,
    bool clearExisting)
{
    std::vector<DummySample> samples = LoadSamples(samplesFile);
    if (samples.empty())
    {
        throw std::invalid_argument(
            "AppendDummyOrders: no samples available to reference in " + samplesFile.string());
    }

    std::vector<DummyOrder> existing = clearExisting ? std::vector<DummyOrder>{} : LoadOrders(ordersFile);
    std::vector<std::string> existingIds = CollectOrderIds(existing);

    AppendResult result;
    for (int i = 0; i < count; ++i)
    {
        DummyOrder order = GenerateOrder(rng, samples, now);
        order.orderId = NextOrderId(existingIds, now);
        existingIds.push_back(order.orderId);
        result.addedIds.push_back(order.orderId);
        existing.push_back(order);
    }

    SaveOrders(ordersFile, existing);
    return result;
}
