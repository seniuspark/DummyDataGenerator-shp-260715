#include "gtest/gtest.h"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <random>

#include "JsonIo.h"

namespace
{
    std::filesystem::path MakeTempDir(const std::string& label)
    {
        std::filesystem::path dir = std::filesystem::temp_directory_path() /
            ("dummy_jsonio_test_" + label + "_" + std::to_string(std::random_device{}()));
        std::filesystem::create_directories(dir);
        return dir;
    }

    std::chrono::system_clock::time_point MakeDateTime(
        int year, unsigned month, unsigned day, int hour, int minute, int second)
    {
        using namespace std::chrono;
        sys_days days = year_month_day{ std::chrono::year{ year } / month / day };
        return system_clock::time_point{ days } + hours{ hour } + minutes{ minute } + seconds{ second };
    }
}

class JsonIoTest : public ::testing::Test
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

TEST_F(JsonIoTest, Iso8601_RoundTripsSecondPrecision)
{
    auto tp = MakeDateTime(2026, 4, 16, 9, 30, 45);

    std::string formatted = FormatIso8601(tp);
    EXPECT_EQ(formatted, "2026-04-16T09:30:45");

    auto parsed = ParseIso8601(formatted);
    EXPECT_EQ(parsed, tp);
}

TEST_F(JsonIoTest, LoadSamples_FileDoesNotExist_ReturnsEmpty)
{
    std::filesystem::path file = tempDir_ / "samples.json";

    EXPECT_TRUE(LoadSamples(file).empty());
}

TEST_F(JsonIoTest, SaveThenLoadSamples_RoundTripsAllFields)
{
    std::filesystem::path file = tempDir_ / "samples.json";
    std::vector<DummySample> samples = {
        { "S-001", "SiC-Wafer-A", 12.5, 0.93, 120 },
        { "S-002", "GaN-Substrate-B", 30.0, 1.0, 0 },
    };

    SaveSamples(file, samples);
    std::vector<DummySample> loaded = LoadSamples(file);

    ASSERT_EQ(loaded.size(), 2u);
    EXPECT_EQ(loaded[0].sampleId, "S-001");
    EXPECT_EQ(loaded[0].name, "SiC-Wafer-A");
    EXPECT_DOUBLE_EQ(loaded[0].avgProductionTime, 12.5);
    EXPECT_DOUBLE_EQ(loaded[0].yield, 0.93);
    EXPECT_EQ(loaded[0].stock, 120);
    EXPECT_EQ(loaded[1].sampleId, "S-002");
}

TEST_F(JsonIoTest, SavedSampleFile_UsesCamelCaseWrappedSchema)
{
    std::filesystem::path file = tempDir_ / "samples.json";
    SaveSamples(file, { { "S-001", "Wafer", 1.0, 0.5, 10 } });

    std::ifstream input(file);
    std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

    EXPECT_NE(content.find("\"samples\""), std::string::npos);
    EXPECT_NE(content.find("\"sampleId\""), std::string::npos);
    EXPECT_NE(content.find("\"avgProductionTime\""), std::string::npos);
}

TEST_F(JsonIoTest, LoadSamples_MalformedJson_ThrowsAndLeavesFileUntouched)
{
    std::filesystem::path file = tempDir_ / "samples.json";
    {
        std::ofstream output(file);
        output << "{ not valid json";
    }

    EXPECT_THROW(LoadSamples(file), DummyDataIoException);

    std::ifstream input(file);
    std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "{ not valid json");
}

TEST_F(JsonIoTest, LoadSamples_MissingSamplesKey_Throws)
{
    std::filesystem::path file = tempDir_ / "samples.json";
    {
        std::ofstream output(file);
        output << "{ \"somethingElse\": [] }";
    }

    EXPECT_THROW(LoadSamples(file), DummyDataIoException);
}

TEST_F(JsonIoTest, SaveThenLoadOrders_RoundTripsAllFields)
{
    std::filesystem::path file = tempDir_ / "orders.json";
    DummyOrder order{
        "ORD-20260416-0043", "S-001", "ACME", 50, OrderStatus::Reserved,
        MakeDateTime(2026, 4, 16, 9, 0, 0),
    };

    SaveOrders(file, { order });
    std::vector<DummyOrder> loaded = LoadOrders(file);

    ASSERT_EQ(loaded.size(), 1u);
    EXPECT_EQ(loaded[0].orderId, "ORD-20260416-0043");
    EXPECT_EQ(loaded[0].sampleId, "S-001");
    EXPECT_EQ(loaded[0].customerName, "ACME");
    EXPECT_EQ(loaded[0].quantity, 50);
    EXPECT_EQ(loaded[0].status, OrderStatus::Reserved);
    EXPECT_EQ(loaded[0].createdAt, order.createdAt);
}

TEST_F(JsonIoTest, LoadOrders_MalformedJson_ThrowsAndLeavesFileUntouched)
{
    std::filesystem::path file = tempDir_ / "orders.json";
    {
        std::ofstream output(file);
        output << "not json at all";
    }

    EXPECT_THROW(LoadOrders(file), DummyDataIoException);

    std::ifstream input(file);
    std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "not json at all");
}
