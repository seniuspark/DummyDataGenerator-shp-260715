#include "CliParser.h"

#include <stdexcept>

namespace
{
    int ParseNonNegativeInt(const std::string& value, const std::string& optionName)
    {
        try
        {
            size_t consumed = 0;
            int parsed = std::stoi(value, &consumed);
            if (consumed != value.size() || parsed < 0)
            {
                throw std::invalid_argument("invalid value");
            }
            return parsed;
        }
        catch (const std::exception&)
        {
            throw std::invalid_argument(optionName + " must be a non-negative integer, got '" + value + "'");
        }
    }

    const std::string& RequireValue(const std::vector<std::string>& args, size_t index, const std::string& optionName)
    {
        if (index + 1 >= args.size())
        {
            throw std::invalid_argument(optionName + " requires a value");
        }
        return args[index + 1];
    }
}

CliOptions ParseCliArgs(const std::vector<std::string>& args)
{
    CliOptions options;

    for (size_t i = 0; i < args.size(); ++i)
    {
        const std::string& arg = args[i];

        if (arg == "--clear")
        {
            options.mode = RunMode::Clear;
        }
        else if (arg == "--samples")
        {
            options.sampleCount = ParseNonNegativeInt(RequireValue(args, i, "--samples"), "--samples");
            ++i;
        }
        else if (arg == "--orders")
        {
            options.orderCount = ParseNonNegativeInt(RequireValue(args, i, "--orders"), "--orders");
            ++i;
        }
        else if (arg == "--data-dir")
        {
            options.dataDir = RequireValue(args, i, "--data-dir");
            ++i;
        }
        else
        {
            throw std::invalid_argument("unknown option: " + arg);
        }
    }

    return options;
}
