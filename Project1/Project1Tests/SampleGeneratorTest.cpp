#include "gtest/gtest.h"

#include <random>
#include <set>

#include "SampleGenerator.h"

namespace
{
    constexpr int kSeedCount = 200;
}

TEST(GenerateSample, YieldAlwaysInValidRange)
{
    for (uint32_t seed = 0; seed < kSeedCount; ++seed)
    {
        std::mt19937 rng(seed);
        DummySample sample = GenerateSample(rng);

        EXPECT_GT(sample.yield, 0.0) << "seed=" << seed;
        EXPECT_LE(sample.yield, 1.0) << "seed=" << seed;
    }
}

TEST(GenerateSample, AvgProductionTimeAlwaysPositive)
{
    for (uint32_t seed = 0; seed < kSeedCount; ++seed)
    {
        std::mt19937 rng(seed);
        DummySample sample = GenerateSample(rng);

        EXPECT_GT(sample.avgProductionTime, 0.0) << "seed=" << seed;
    }
}

TEST(GenerateSample, StockAlwaysNonNegative)
{
    for (uint32_t seed = 0; seed < kSeedCount; ++seed)
    {
        std::mt19937 rng(seed);
        DummySample sample = GenerateSample(rng);

        EXPECT_GE(sample.stock, 0) << "seed=" << seed;
    }
}

TEST(GenerateSample, NameIsNeverEmpty)
{
    for (uint32_t seed = 0; seed < kSeedCount; ++seed)
    {
        std::mt19937 rng(seed);
        DummySample sample = GenerateSample(rng);

        EXPECT_FALSE(sample.name.empty()) << "seed=" << seed;
    }
}

TEST(GenerateSample, IsDeterministicForSameSeed)
{
    std::mt19937 rngA(42);
    std::mt19937 rngB(42);

    DummySample a = GenerateSample(rngA);
    DummySample b = GenerateSample(rngB);

    EXPECT_EQ(a.name, b.name);
    EXPECT_DOUBLE_EQ(a.avgProductionTime, b.avgProductionTime);
    EXPECT_DOUBLE_EQ(a.yield, b.yield);
    EXPECT_EQ(a.stock, b.stock);
}

TEST(GenerateSample, DifferentSeedsProduceVariation)
{
    std::set<std::string> names;
    std::set<double> yields;
    std::set<double> productionTimes;
    std::set<int> stocks;

    for (uint32_t seed = 0; seed < kSeedCount; ++seed)
    {
        std::mt19937 rng(seed);
        DummySample sample = GenerateSample(rng);

        names.insert(sample.name);
        yields.insert(sample.yield);
        productionTimes.insert(sample.avgProductionTime);
        stocks.insert(sample.stock);
    }

    EXPECT_GT(names.size(), 1u);
    EXPECT_GT(yields.size(), 1u);
    EXPECT_GT(productionTimes.size(), 1u);
    EXPECT_GT(stocks.size(), 1u);
}

TEST(GenerateSample, BoundarySeeds)
{
    const uint32_t boundarySeeds[] = { 0u, 1u, UINT32_MAX };

    for (uint32_t seed : boundarySeeds)
    {
        std::mt19937 rng(seed);
        DummySample sample = GenerateSample(rng);

        EXPECT_GT(sample.yield, 0.0) << "seed=" << seed;
        EXPECT_LE(sample.yield, 1.0) << "seed=" << seed;
        EXPECT_GT(sample.avgProductionTime, 0.0) << "seed=" << seed;
        EXPECT_GE(sample.stock, 0) << "seed=" << seed;
        EXPECT_FALSE(sample.name.empty()) << "seed=" << seed;
    }
}
