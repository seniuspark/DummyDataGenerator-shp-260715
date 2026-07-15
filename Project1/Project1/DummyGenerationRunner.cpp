#include "DummyGenerationRunner.h"

DummyGenerationResult RunDummyGeneration(
    const CliOptions& options,
    std::mt19937& rng,
    std::chrono::system_clock::time_point now)
{
    const bool clearExisting = options.mode == RunMode::Clear;
    const std::filesystem::path samplesFile = options.dataDir / "samples.json";
    const std::filesystem::path ordersFile = options.dataDir / "orders.json";

    std::filesystem::create_directories(options.dataDir);

    DummyGenerationResult result;
    result.samples = AppendDummySamples(samplesFile, options.sampleCount, rng, clearExisting);
    result.orders = AppendDummyOrders(samplesFile, ordersFile, options.orderCount, rng, now, clearExisting);
    return result;
}
