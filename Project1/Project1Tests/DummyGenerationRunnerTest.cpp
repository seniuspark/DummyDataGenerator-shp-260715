#include "gtest/gtest.h"

#include <chrono>
#include <filesystem>
#include <random>
#include <string>

#include "DummyGenerationRunner.h"
#include "JsonIo.h"

namespace
{
    std::filesystem::path MakeTempDir(const std::string& label)
    {
        std::filesystem::path dir = std::filesystem::temp_directory_path() /
            ("dummy_runner_test_" + label + "_" + std::to_string(std::random_device{}()));
        std::filesystem::create_directories(dir);
        return dir;
    }

    std::chrono::system_clock::time_point FixedNow()
    {
        using namespace std::chrono;
        return system_clock::time_point{ sys_days{ std::chrono::year{ 2026 } / 7 / 15 } } + hours{ 9 };
    }
}

class DummyGenerationRunnerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        tempDir_ = MakeTempDir("basic");
    }

    void TearDown() override
    {
        std::error_code ec;
        std::filesystem::remove_all(tempDir_, ec);
    }

    std::filesystem::path tempDir_;
};

TEST_F(DummyGenerationRunnerTest, RunDummyGeneration_AppendModeAddsRequestedCounts)
{
    CliOptions options;
    options.mode = RunMode::Append;
    options.sampleCount = 3;
    options.orderCount = 4;
    options.dataDir = tempDir_;

    std::mt19937 rng(42);
    DummyGenerationResult result = RunDummyGeneration(options, rng, FixedNow());

    EXPECT_EQ(result.samples.addedIds.size(), 3u);
    EXPECT_EQ(result.orders.addedIds.size(), 4u);

    std::vector<DummySample> samples = LoadSamples(tempDir_ / "samples.json");
    std::vector<DummyOrder> orders = LoadOrders(tempDir_ / "orders.json");
    EXPECT_EQ(samples.size(), 3u);
    EXPECT_EQ(orders.size(), 4u);
}

TEST_F(DummyGenerationRunnerTest, RunDummyGeneration_AppendModePreservesExistingData)
{
    SaveSamples(tempDir_ / "samples.json", { { "S-001", "Existing-A", 10.0, 0.8, 50 } });

    CliOptions options;
    options.mode = RunMode::Append;
    options.sampleCount = 2;
    options.orderCount = 0;
    options.dataDir = tempDir_;

    std::mt19937 rng(7);
    RunDummyGeneration(options, rng, FixedNow());

    std::vector<DummySample> samples = LoadSamples(tempDir_ / "samples.json");
    ASSERT_EQ(samples.size(), 3u);
    EXPECT_EQ(samples[0].sampleId, "S-001");
}

TEST_F(DummyGenerationRunnerTest, RunDummyGeneration_ClearModeDiscardsExistingData)
{
    SaveSamples(tempDir_ / "samples.json", { { "S-001", "Existing-A", 10.0, 0.8, 50 } });
    SaveOrders(tempDir_ / "orders.json", { { "ORD-20260101-0001", "S-001", "Old", 1, OrderStatus::Release, FixedNow() } });

    CliOptions options;
    options.mode = RunMode::Clear;
    options.sampleCount = 2;
    options.orderCount = 2;
    options.dataDir = tempDir_;

    std::mt19937 rng(9);
    RunDummyGeneration(options, rng, FixedNow());

    std::vector<DummySample> samples = LoadSamples(tempDir_ / "samples.json");
    std::vector<DummyOrder> orders = LoadOrders(tempDir_ / "orders.json");
    ASSERT_EQ(samples.size(), 2u);
    ASSERT_EQ(orders.size(), 2u);
    for (const auto& sample : samples)
    {
        EXPECT_NE(sample.name, "Existing-A");
    }
}
