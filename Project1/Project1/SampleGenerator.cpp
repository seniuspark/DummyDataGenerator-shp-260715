#include "SampleGenerator.h"

namespace
{
    const char* const kNamePrefixes[] = {
        "SiC-Wafer", "GaN-Substrate", "Si-Ingot", "SiGe-Epi", "GaAs-Wafer",
    };

    constexpr size_t kNamePrefixCount = sizeof(kNamePrefixes) / sizeof(kNamePrefixes[0]);
}

DummySample GenerateSample(std::mt19937& rng)
{
    std::uniform_int_distribution<size_t> prefixDist(0, kNamePrefixCount - 1);
    std::uniform_int_distribution<int> suffixDist(1, 999);
    std::uniform_real_distribution<double> yieldDist(0.0001, 1.0);
    std::uniform_real_distribution<double> productionTimeDist(1.0, 120.0);
    std::uniform_int_distribution<int> stockDist(0, 500);

    DummySample sample;
    sample.name = std::string(kNamePrefixes[prefixDist(rng)]) + "-" + std::to_string(suffixDist(rng));
    sample.avgProductionTime = productionTimeDist(rng);
    sample.yield = yieldDist(rng);
    sample.stock = stockDist(rng);

    return sample;
}
