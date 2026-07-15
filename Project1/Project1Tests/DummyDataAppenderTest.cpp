#include "gtest/gtest.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <random>
#include <set>

#include "DummyDataAppender.h"
#include "JsonIo.h"

namespace
{
    std::filesystem::path MakeTempDir(const std::string& label)
    {
        std::filesystem::path dir = std::filesystem::temp_directory_path() /
            ("dummy_appender_test_" + label + "_" + std::to_string(std::random_device{}()));
        std::filesystem::create_directories(dir);
        return dir;
    }

    std::chrono::system_clock::time_point FixedNow()
    {
        using namespace std::chrono;
        return system_clock::time_point{ sys_days{ std::chrono::year{ 2026 } / 7 / 15 } } + hours{ 9 };
    }
}

class DummyDataAppenderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        tempDir_ = MakeTempDir("basic");
        samplesFile_ = tempDir_ / "samples.json";
        ordersFile_ = tempDir_ / "orders.json";
    }

    void TearDown() override
    {
        std::error_code ec;
        std::filesystem::remove_all(tempDir_, ec);
    }

    std::filesystem::path tempDir_;
    std::filesystem::path samplesFile_;
    std::filesystem::path ordersFile_;
};

TEST_F(DummyDataAppenderTest, AppendSamples_ToNonExistentFile_CreatesFileWithNewSamplesOnly)
{
    std::mt19937 rng(1);

    AppendResult result = AppendDummySamples(samplesFile_, 3, rng);

    EXPECT_EQ(result.addedIds.size(), 3u);
    std::vector<DummySample> samples = LoadSamples(samplesFile_);
    ASSERT_EQ(samples.size(), 3u);
    for (const auto& sample : samples)
    {
        EXPECT_GT(sample.yield, 0.0);
        EXPECT_LE(sample.yield, 1.0);
        EXPECT_GT(sample.avgProductionTime, 0.0);
    }
}

TEST_F(DummyDataAppenderTest, AppendSamples_ToExistingFile_PreservesOldEntriesAndAddsNew)
{
    std::vector<DummySample> preexisting = {
        { "S-001", "Existing-A", 10.0, 0.8, 50 },
        { "S-002", "Existing-B", 20.0, 0.9, 60 },
    };
    SaveSamples(samplesFile_, preexisting);

    std::mt19937 rng(2);
    AppendResult result = AppendDummySamples(samplesFile_, 3, rng);

    EXPECT_EQ(result.addedIds.size(), 3u);
    std::vector<DummySample> samples = LoadSamples(samplesFile_);
    ASSERT_EQ(samples.size(), 5u);

    EXPECT_EQ(samples[0].sampleId, "S-001");
    EXPECT_EQ(samples[0].name, "Existing-A");
    EXPECT_DOUBLE_EQ(samples[0].avgProductionTime, 10.0);
    EXPECT_DOUBLE_EQ(samples[0].yield, 0.8);
    EXPECT_EQ(samples[0].stock, 50);

    EXPECT_EQ(samples[1].sampleId, "S-002");
    EXPECT_EQ(samples[1].name, "Existing-B");
}

TEST_F(DummyDataAppenderTest, AppendSamples_NewIdsDoNotCollideWithExisting)
{
    std::vector<DummySample> preexisting = {
        { "S-001", "Existing-A", 10.0, 0.8, 50 },
        { "S-005", "Existing-B", 20.0, 0.9, 60 },
    };
    SaveSamples(samplesFile_, preexisting);

    std::mt19937 rng(3);
    AppendResult result = AppendDummySamples(samplesFile_, 3, rng);

    EXPECT_EQ(result.addedIds, (std::vector<std::string>{ "S-006", "S-007", "S-008" }));

    std::vector<DummySample> samples = LoadSamples(samplesFile_);
    std::set<std::string> ids;
    for (const auto& sample : samples)
    {
        EXPECT_TRUE(ids.insert(sample.sampleId).second) << "duplicate id " << sample.sampleId;
    }
    EXPECT_EQ(ids.size(), 5u);
}

TEST_F(DummyDataAppenderTest, AppendOrders_ToExistingFile_PreservesOldEntriesAndAddsNew)
{
    SaveSamples(samplesFile_, { { "S-001", "Wafer-A", 10.0, 0.8, 50 } });

    std::vector<DummyOrder> preexisting = {
        { "ORD-20260715-0001", "S-001", "OldCustomer", 10, OrderStatus::Release, FixedNow() },
    };
    SaveOrders(ordersFile_, preexisting);

    std::mt19937 rng(4);
    AppendResult result = AppendDummyOrders(samplesFile_, ordersFile_, 3, rng, FixedNow());

    EXPECT_EQ(result.addedIds.size(), 3u);
    std::vector<DummyOrder> orders = LoadOrders(ordersFile_);
    ASSERT_EQ(orders.size(), 4u);

    EXPECT_EQ(orders[0].orderId, "ORD-20260715-0001");
    EXPECT_EQ(orders[0].customerName, "OldCustomer");
    EXPECT_EQ(orders[0].status, OrderStatus::Release);
}

TEST_F(DummyDataAppenderTest, AppendOrders_ReferencesOnlyExistingOrJustGeneratedSamples)
{
    SaveSamples(samplesFile_, {
        { "S-001", "Wafer-A", 10.0, 0.8, 50 },
        { "S-002", "Wafer-B", 20.0, 0.7, 30 },
    });

    std::mt19937 rng(5);
    AppendResult result = AppendDummyOrders(samplesFile_, ordersFile_, 10, rng, FixedNow());

    std::vector<DummyOrder> orders = LoadOrders(ordersFile_);
    ASSERT_EQ(orders.size(), 10u);
    std::set<std::string> validSampleIds = { "S-001", "S-002" };
    for (const auto& order : orders)
    {
        EXPECT_TRUE(validSampleIds.count(order.sampleId)) << "order references unknown sample " << order.sampleId;
    }
}

TEST_F(DummyDataAppenderTest, AppendOrders_NoSamplesAvailable_Throws)
{
    std::mt19937 rng(6);

    EXPECT_THROW(AppendDummyOrders(samplesFile_, ordersFile_, 1, rng, FixedNow()), std::invalid_argument);
}

TEST_F(DummyDataAppenderTest, AppendSamples_MalformedExistingFile_ReportsErrorWithoutDataLoss)
{
    {
        std::ofstream output(samplesFile_);
        output << "{ this is not valid json";
    }

    std::mt19937 rng(7);
    EXPECT_THROW(AppendDummySamples(samplesFile_, 2, rng), DummyDataIoException);

    std::ifstream input(samplesFile_);
    std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "{ this is not valid json");
}

TEST_F(DummyDataAppenderTest, AppendSamples_ClearOptionReplacesFileInsteadOfAppending)
{
    SaveSamples(samplesFile_, { { "S-001", "Existing-A", 10.0, 0.8, 50 } });

    std::mt19937 rng(8);
    AppendResult result = AppendDummySamples(samplesFile_, 2, rng, /*clearExisting=*/true);

    EXPECT_EQ(result.addedIds.size(), 2u);
    std::vector<DummySample> samples = LoadSamples(samplesFile_);
    ASSERT_EQ(samples.size(), 2u);
    for (const auto& sample : samples)
    {
        EXPECT_NE(sample.name, "Existing-A") << "clear option must discard the previous data";
    }
}

TEST_F(DummyDataAppenderTest, AppendSamples_DefaultBehaviorIsAppendNotClear)
{
    SaveSamples(samplesFile_, { { "S-001", "Existing-A", 10.0, 0.8, 50 } });

    std::mt19937 rng(9);
    AppendDummySamples(samplesFile_, 2, rng);

    std::vector<DummySample> samples = LoadSamples(samplesFile_);
    EXPECT_EQ(samples.size(), 3u);
    EXPECT_EQ(samples[0].sampleId, "S-001");
}
