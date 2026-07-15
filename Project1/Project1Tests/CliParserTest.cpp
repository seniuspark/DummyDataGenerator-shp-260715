#include "gtest/gtest.h"

#include <stdexcept>
#include <string>
#include <vector>

#include "CliParser.h"

TEST(CliParserTest, ParseCliArgs_DefaultsToAppendModeWhenNoClearFlag)
{
    CliOptions options = ParseCliArgs({});

    EXPECT_EQ(options.mode, RunMode::Append);
}

TEST(CliParserTest, ParseCliArgs_ClearFlagSwitchesToClearMode)
{
    CliOptions options = ParseCliArgs({ "--clear" });

    EXPECT_EQ(options.mode, RunMode::Clear);
}

TEST(CliParserTest, ParseCliArgs_SampleCountAndOrderCountParsedFromArgs)
{
    CliOptions options = ParseCliArgs({ "--samples", "10", "--orders", "5" });

    EXPECT_EQ(options.sampleCount, 10);
    EXPECT_EQ(options.orderCount, 5);
}

TEST(CliParserTest, ParseCliArgs_MissingCountsFallBackToDefaults)
{
    CliOptions options = ParseCliArgs({});

    EXPECT_EQ(options.sampleCount, 5);
    EXPECT_EQ(options.orderCount, 5);
}

TEST(CliParserTest, ParseCliArgs_InvalidCountThrowsOrReportsError)
{
    EXPECT_THROW(ParseCliArgs({ "--samples", "-1" }), std::invalid_argument);
    EXPECT_THROW(ParseCliArgs({ "--samples", "abc" }), std::invalid_argument);
    EXPECT_THROW(ParseCliArgs({ "--orders", "-3" }), std::invalid_argument);
}

TEST(CliParserTest, ParseCliArgs_DataDirOptionOverridesDefaultPath)
{
    CliOptions options = ParseCliArgs({ "--data-dir", "custom_data" });

    EXPECT_EQ(options.dataDir, std::filesystem::path("custom_data"));
}

TEST(CliParserTest, ParseCliArgs_DataDirDefaultsToDataWhenNotSpecified)
{
    CliOptions options = ParseCliArgs({});

    EXPECT_EQ(options.dataDir, std::filesystem::path("data"));
}
