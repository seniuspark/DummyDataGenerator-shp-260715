#include "IdAllocator.h"

#include <cctype>
#include <iomanip>
#include <sstream>

namespace
{
    // "prefix" + 숫자(3자리 이상 zero-pad)를 만든다.
    std::string FormatNumbered(const std::string& prefix, int number, int width)
    {
        std::ostringstream oss;
        oss << prefix << std::setw(width) << std::setfill('0') << number;
        return oss.str();
    }

    std::string FormatDate(std::chrono::system_clock::time_point tp)
    {
        using namespace std::chrono;
        year_month_day ymd{ floor<days>(tp) };

        std::ostringstream oss;
        oss << std::setw(4) << std::setfill('0') << static_cast<int>(ymd.year())
            << std::setw(2) << std::setfill('0') << static_cast<unsigned>(ymd.month())
            << std::setw(2) << std::setfill('0') << static_cast<unsigned>(ymd.day());
        return oss.str();
    }
}

std::optional<int> ParseSampleIdNumber(const std::string& sampleId)
{
    const std::string prefix = "S-";
    if (sampleId.size() <= prefix.size() || sampleId.compare(0, prefix.size(), prefix) != 0)
    {
        return std::nullopt;
    }

    const std::string digits = sampleId.substr(prefix.size());
    if (digits.empty())
    {
        return std::nullopt;
    }
    for (char c : digits)
    {
        if (!std::isdigit(static_cast<unsigned char>(c)))
        {
            return std::nullopt;
        }
    }

    try
    {
        return std::stoi(digits);
    }
    catch (...)
    {
        return std::nullopt;
    }
}

std::string NextSampleId(const std::vector<std::string>& existingSampleIds)
{
    int maxNumber = 0;
    for (const std::string& id : existingSampleIds)
    {
        std::optional<int> parsed = ParseSampleIdNumber(id);
        if (parsed.has_value() && parsed.value() > maxNumber)
        {
            maxNumber = parsed.value();
        }
    }

    return FormatNumbered("S-", maxNumber + 1, 3);
}

std::optional<std::pair<std::string, int>> ParseOrderIdNumber(const std::string& orderId)
{
    const std::string prefix = "ORD-";
    const size_t datePos = prefix.size();
    const size_t dateLen = 8;
    const size_t separatorPos = datePos + dateLen;

    if (orderId.size() <= separatorPos + 1)
    {
        return std::nullopt;
    }
    if (orderId.compare(0, prefix.size(), prefix) != 0)
    {
        return std::nullopt;
    }
    if (orderId[separatorPos] != '-')
    {
        return std::nullopt;
    }

    const std::string date = orderId.substr(datePos, dateLen);
    for (char c : date)
    {
        if (!std::isdigit(static_cast<unsigned char>(c)))
        {
            return std::nullopt;
        }
    }

    const std::string numberPart = orderId.substr(separatorPos + 1);
    if (numberPart.empty())
    {
        return std::nullopt;
    }
    for (char c : numberPart)
    {
        if (!std::isdigit(static_cast<unsigned char>(c)))
        {
            return std::nullopt;
        }
    }

    try
    {
        return std::make_pair(date, std::stoi(numberPart));
    }
    catch (...)
    {
        return std::nullopt;
    }
}

std::string NextOrderId(
    const std::vector<std::string>& existingOrderIds,
    std::chrono::system_clock::time_point now)
{
    const std::string today = FormatDate(now);

    int maxNumber = 0;
    for (const std::string& id : existingOrderIds)
    {
        std::optional<std::pair<std::string, int>> parsed = ParseOrderIdNumber(id);
        if (parsed.has_value() && parsed->first == today && parsed->second > maxNumber)
        {
            maxNumber = parsed->second;
        }
    }

    return "ORD-" + today + "-" + FormatNumbered("", maxNumber + 1, 4);
}
