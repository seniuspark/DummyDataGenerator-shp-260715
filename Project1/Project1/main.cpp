#include <chrono>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "CliParser.h"
#include "DummyGenerationRunner.h"

namespace
{
    std::vector<std::string> ToArgList(int argc, char** argv)
    {
        std::vector<std::string> args;
        args.reserve(static_cast<size_t>(argc > 0 ? argc - 1 : 0));
        for (int i = 1; i < argc; ++i)
        {
            args.emplace_back(argv[i]);
        }
        return args;
    }
}

int main(int argc, char** argv)
{
    CliOptions options;
    try
    {
        options = ParseCliArgs(ToArgList(argc, argv));
    }
    catch (const std::invalid_argument& e)
    {
        std::cerr << "잘못된 인자: " << e.what() << "\n";
        std::cerr << "사용법: Project1 [--samples N] [--orders N] [--clear] [--data-dir PATH]\n";
        return 1;
    }

    std::random_device seedSource;
    std::mt19937 rng(seedSource());
    auto now = std::chrono::system_clock::now();

    try
    {
        DummyGenerationResult result = RunDummyGeneration(options, rng, now);

        std::cout << (options.mode == RunMode::Clear ? "[clear] " : "[append] ")
            << "data-dir=" << options.dataDir.string() << "\n";
        std::cout << "Sample " << result.samples.addedIds.size() << "건 생성: ";
        for (const auto& id : result.samples.addedIds)
        {
            std::cout << id << " ";
        }
        std::cout << "\n";
        std::cout << "Order " << result.orders.addedIds.size() << "건 생성: ";
        for (const auto& id : result.orders.addedIds)
        {
            std::cout << id << " ";
        }
        std::cout << "\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "더미 데이터 생성 실패: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
