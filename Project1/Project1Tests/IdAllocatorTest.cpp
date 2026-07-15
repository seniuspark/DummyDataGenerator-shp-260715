#include "gtest/gtest.h"

#include <chrono>
#include <set>
#include <string>
#include <vector>

#include "IdAllocator.h"

namespace
{
    std::chrono::system_clock::time_point MakeDate(int year, unsigned month, unsigned day)
    {
        using namespace std::chrono;
        return sys_days{ std::chrono::year{ year } / month / day };
    }
}

TEST(NextSampleId, EmptyExistingIds_StartsAtS001)
{
    std::vector<std::string> existing;

    EXPECT_EQ(NextSampleId(existing), "S-001");
}

TEST(NextSampleId, NonEmptyExistingIds_ReturnsMaxPlusOne)
{
    std::vector<std::string> existing = { "S-001", "S-003", "S-007" };

    EXPECT_EQ(NextSampleId(existing), "S-008");
}

TEST(NextSampleId, GeneratingMultipleIdsInOneRun_NoDuplicates)
{
    std::vector<std::string> existing;
    std::set<std::string> generated;

    for (int i = 0; i < 5; ++i)
    {
        std::string id = NextSampleId(existing);
        EXPECT_TRUE(generated.insert(id).second) << "duplicate id=" << id;
        existing.push_back(id);
    }
}

TEST(NextSampleId, MalformedExistingIdIsIgnoredNotCrashed)
{
    std::vector<std::string> existing = { "", "SAMPLE-X", "S-005", "S-abc" };

    EXPECT_EQ(NextSampleId(existing), "S-006");
}

TEST(NextOrderId, EmptyExistingIds_StartsAt0001ForGivenDate)
{
    std::vector<std::string> existing;
    auto now = MakeDate(2026, 7, 15);

    EXPECT_EQ(NextOrderId(existing, now), "ORD-20260715-0001");
}

TEST(NextOrderId, NonEmptyExistingIds_SameDate_ReturnsMaxPlusOne)
{
    std::vector<std::string> existing = {
        "ORD-20260715-0001",
        "ORD-20260715-0003",
    };
    auto now = MakeDate(2026, 7, 15);

    EXPECT_EQ(NextOrderId(existing, now), "ORD-20260715-0004");
}

TEST(NextOrderId, ExistingIdsFromDifferentDate_StartsAt0001ForNewDate)
{
    std::vector<std::string> existing = {
        "ORD-20260715-0001",
        "ORD-20260715-0002",
    };
    auto now = MakeDate(2026, 7, 16);

    EXPECT_EQ(NextOrderId(existing, now), "ORD-20260716-0001");
}

TEST(NextOrderId, GeneratingMultipleIdsInOneRun_NoDuplicates)
{
    std::vector<std::string> existing;
    std::set<std::string> generated;
    auto now = MakeDate(2026, 7, 15);

    for (int i = 0; i < 5; ++i)
    {
        std::string id = NextOrderId(existing, now);
        EXPECT_TRUE(generated.insert(id).second) << "duplicate id=" << id;
        existing.push_back(id);
    }
}
