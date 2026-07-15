#include "gtest/gtest.h"

#include <chrono>
#include <random>
#include <set>
#include <vector>

#include "DummySample.h"
#include "OrderGenerator.h"
#include "OrderStatus.h"

namespace
{
    constexpr int kSeedCount = 200;

    std::vector<DummySample> MakeSamples(size_t count)
    {
        std::vector<DummySample> samples;
        for (size_t i = 0; i < count; ++i)
        {
            DummySample sample;
            sample.sampleId = "S-" + std::to_string(i + 1);
            sample.name = "Sample-" + std::to_string(i + 1);
            sample.avgProductionTime = 10.0;
            sample.yield = 0.9;
            sample.stock = 100;
            samples.push_back(sample);
        }
        return samples;
    }

    bool ContainsSampleId(const std::vector<DummySample>& samples, const std::string& sampleId)
    {
        for (const DummySample& sample : samples)
        {
            if (sample.sampleId == sampleId)
            {
                return true;
            }
        }
        return false;
    }
}

TEST(GenerateOrder, SampleIdAlwaysReferencesExistingSample)
{
    const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    const size_t sampleCounts[] = { 1, 3, 10 };

    for (size_t sampleCount : sampleCounts)
    {
        std::vector<DummySample> samples = MakeSamples(sampleCount);

        for (uint32_t seed = 0; seed < kSeedCount; ++seed)
        {
            std::mt19937 rng(seed);
            DummyOrder order = GenerateOrder(rng, samples, now);

            EXPECT_TRUE(ContainsSampleId(samples, order.sampleId))
                << "seed=" << seed << " sampleCount=" << sampleCount << " sampleId=" << order.sampleId;
        }
    }
}

TEST(GenerateOrder, QuantityAlwaysPositive)
{
    std::vector<DummySample> samples = MakeSamples(3);
    const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    for (uint32_t seed = 0; seed < kSeedCount; ++seed)
    {
        std::mt19937 rng(seed);
        DummyOrder order = GenerateOrder(rng, samples, now);

        EXPECT_GE(order.quantity, 1) << "seed=" << seed;
    }
}

TEST(GenerateOrder, StatusAlwaysOneOfFiveValidValues)
{
    std::vector<DummySample> samples = MakeSamples(3);
    const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    for (uint32_t seed = 0; seed < kSeedCount; ++seed)
    {
        std::mt19937 rng(seed);
        DummyOrder order = GenerateOrder(rng, samples, now);

        EXPECT_TRUE(
            order.status == OrderStatus::Reserved ||
            order.status == OrderStatus::Rejected ||
            order.status == OrderStatus::Producing ||
            order.status == OrderStatus::Confirmed ||
            order.status == OrderStatus::Release)
            << "seed=" << seed;
    }
}

TEST(GenerateOrder, ReleaseStatusHasPastCreatedAt)
{
    std::vector<DummySample> samples = MakeSamples(3);
    const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    int releaseOrConfirmedCount = 0;

    for (uint32_t seed = 0; seed < kSeedCount; ++seed)
    {
        std::mt19937 rng(seed);
        DummyOrder order = GenerateOrder(rng, samples, now);

        if (order.status == OrderStatus::Release || order.status == OrderStatus::Confirmed)
        {
            ++releaseOrConfirmedCount;
            EXPECT_LT(order.createdAt, now) << "seed=" << seed;
        }
    }

    EXPECT_GT(releaseOrConfirmedCount, 0);
}

TEST(GenerateOrder, ReservedStatusHasRecentOrPastCreatedAt)
{
    std::vector<DummySample> samples = MakeSamples(3);
    const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    int reservedCount = 0;

    for (uint32_t seed = 0; seed < kSeedCount; ++seed)
    {
        std::mt19937 rng(seed);
        DummyOrder order = GenerateOrder(rng, samples, now);

        if (order.status == OrderStatus::Reserved)
        {
            ++reservedCount;
            EXPECT_LE(order.createdAt, now) << "seed=" << seed;
        }
    }

    EXPECT_GT(reservedCount, 0);
}

TEST(GenerateOrder, EmptySampleListThrowsInvalidArgument)
{
    std::vector<DummySample> emptySamples;
    const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::mt19937 rng(1);

    EXPECT_THROW(GenerateOrder(rng, emptySamples, now), std::invalid_argument);
}

TEST(GenerateOrder, IsDeterministicForSameSeed)
{
    std::vector<DummySample> samples = MakeSamples(3);
    const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    std::mt19937 rngA(42);
    std::mt19937 rngB(42);

    DummyOrder a = GenerateOrder(rngA, samples, now);
    DummyOrder b = GenerateOrder(rngB, samples, now);

    EXPECT_EQ(a.sampleId, b.sampleId);
    EXPECT_EQ(a.customerName, b.customerName);
    EXPECT_EQ(a.quantity, b.quantity);
    EXPECT_EQ(a.status, b.status);
    EXPECT_EQ(a.createdAt, b.createdAt);
}

TEST(GenerateOrder, CustomerNameIsNeverEmpty)
{
    std::vector<DummySample> samples = MakeSamples(3);
    const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    for (uint32_t seed = 0; seed < kSeedCount; ++seed)
    {
        std::mt19937 rng(seed);
        DummyOrder order = GenerateOrder(rng, samples, now);

        EXPECT_FALSE(order.customerName.empty()) << "seed=" << seed;
    }
}
