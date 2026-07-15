#include "JsonIo.h"

#include <fstream>
#include <iomanip>
#include <sstream>

#include <nlohmann/json.hpp>

namespace
{
    nlohmann::json ToJson(const DummySample& sample)
    {
        return nlohmann::json{
            {"sampleId", sample.sampleId},
            {"name", sample.name},
            {"avgProductionTime", sample.avgProductionTime},
            {"yield", sample.yield},
            {"stock", sample.stock},
        };
    }

    DummySample SampleFromJson(const nlohmann::json& json)
    {
        return DummySample{
            json.at("sampleId").get<std::string>(),
            json.at("name").get<std::string>(),
            json.at("avgProductionTime").get<double>(),
            json.at("yield").get<double>(),
            json.at("stock").get<int>(),
        };
    }

    nlohmann::json ToJson(const DummyOrder& order)
    {
        return nlohmann::json{
            {"orderId", order.orderId},
            {"sampleId", order.sampleId},
            {"customerName", order.customerName},
            {"quantity", order.quantity},
            {"status", ToString(order.status)},
            {"createdAt", FormatIso8601(order.createdAt)},
        };
    }

    OrderStatus ParseOrderStatus(const std::string& text)
    {
        if (text == "RESERVED") return OrderStatus::Reserved;
        if (text == "REJECTED") return OrderStatus::Rejected;
        if (text == "PRODUCING") return OrderStatus::Producing;
        if (text == "CONFIRMED") return OrderStatus::Confirmed;
        if (text == "RELEASE") return OrderStatus::Release;
        throw DummyDataIoException("Unknown order status string: " + text);
    }

    DummyOrder OrderFromJson(const nlohmann::json& json)
    {
        return DummyOrder{
            json.at("orderId").get<std::string>(),
            json.at("sampleId").get<std::string>(),
            json.at("customerName").get<std::string>(),
            json.at("quantity").get<int>(),
            ParseOrderStatus(json.at("status").get<std::string>()),
            ParseIso8601(json.at("createdAt").get<std::string>()),
        };
    }

    nlohmann::json ParseJsonFileOrThrow(const std::filesystem::path& file)
    {
        std::ifstream input(file);
        if (!input)
        {
            throw DummyDataIoException("Failed to open file for reading: " + file.string());
        }

        try
        {
            nlohmann::json json;
            input >> json;
            return json;
        }
        catch (const nlohmann::json::exception& e)
        {
            throw DummyDataIoException(
                "Malformed JSON file: " + file.string() + " (" + e.what() + ")");
        }
    }

    // 원자적 쓰기: 임시 파일에 먼저 쓴 뒤 목표 경로로 rename한다. 쓰기 도중
    // 실패해도 원본 파일은 그대로 남는다.
    void WriteJsonAtomically(const std::filesystem::path& file, const nlohmann::json& json)
    {
        std::filesystem::path parent = file.parent_path();
        if (!parent.empty())
        {
            std::filesystem::create_directories(parent);
        }

        std::filesystem::path tempFile = file;
        tempFile += ".tmp";

        {
            std::ofstream output(tempFile);
            if (!output)
            {
                throw DummyDataIoException("Failed to open temp file for writing: " + tempFile.string());
            }
            output << json.dump(2);
        }

        std::filesystem::rename(tempFile, file);
    }
}

std::string FormatIso8601(std::chrono::system_clock::time_point tp)
{
    using namespace std::chrono;

    auto secondsSinceEpoch = time_point_cast<seconds>(tp);
    year_month_day ymd{ floor<days>(secondsSinceEpoch) };
    hh_mm_ss<seconds> hms{ secondsSinceEpoch - floor<days>(secondsSinceEpoch) };

    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << static_cast<int>(ymd.year()) << "-"
        << std::setw(2) << std::setfill('0') << static_cast<unsigned>(ymd.month()) << "-"
        << std::setw(2) << std::setfill('0') << static_cast<unsigned>(ymd.day()) << "T"
        << std::setw(2) << std::setfill('0') << hms.hours().count() << ":"
        << std::setw(2) << std::setfill('0') << hms.minutes().count() << ":"
        << std::setw(2) << std::setfill('0') << hms.seconds().count();
    return oss.str();
}

std::chrono::system_clock::time_point ParseIso8601(const std::string& text)
{
    int year, month, day, hour, minute, second;
    char dash1, dash2, sep, colon1, colon2;
    std::istringstream iss(text);
    iss >> year >> dash1 >> month >> dash2 >> day >> sep >> hour >> colon1 >> minute >> colon2 >> second;

    if (iss.fail() || dash1 != '-' || dash2 != '-' || sep != 'T' || colon1 != ':' || colon2 != ':')
    {
        throw DummyDataIoException("Malformed ISO8601 timestamp: " + text);
    }

    using namespace std::chrono;
    sys_days days = year_month_day{ std::chrono::year{ year } / month / day };
    return system_clock::time_point{ days } + hours{ hour } + minutes{ minute } + seconds{ second };
}

std::vector<DummySample> LoadSamples(const std::filesystem::path& file)
{
    if (!std::filesystem::exists(file))
    {
        return {};
    }

    nlohmann::json json = ParseJsonFileOrThrow(file);

    try
    {
        std::vector<DummySample> samples;
        for (const auto& item : json.at("samples"))
        {
            samples.push_back(SampleFromJson(item));
        }
        return samples;
    }
    catch (const nlohmann::json::exception& e)
    {
        throw DummyDataIoException(
            "Malformed samples file schema: " + file.string() + " (" + e.what() + ")");
    }
}

void SaveSamples(const std::filesystem::path& file, const std::vector<DummySample>& samples)
{
    nlohmann::json json;
    json["samples"] = nlohmann::json::array();
    for (const auto& sample : samples)
    {
        json["samples"].push_back(ToJson(sample));
    }

    WriteJsonAtomically(file, json);
}

std::vector<DummyOrder> LoadOrders(const std::filesystem::path& file)
{
    if (!std::filesystem::exists(file))
    {
        return {};
    }

    nlohmann::json json = ParseJsonFileOrThrow(file);

    try
    {
        std::vector<DummyOrder> orders;
        for (const auto& item : json.at("orders"))
        {
            orders.push_back(OrderFromJson(item));
        }
        return orders;
    }
    catch (const nlohmann::json::exception& e)
    {
        throw DummyDataIoException(
            "Malformed orders file schema: " + file.string() + " (" + e.what() + ")");
    }
}

void SaveOrders(const std::filesystem::path& file, const std::vector<DummyOrder>& orders)
{
    nlohmann::json json;
    json["orders"] = nlohmann::json::array();
    for (const auto& order : orders)
    {
        json["orders"].push_back(ToJson(order));
    }

    WriteJsonAtomically(file, json);
}
