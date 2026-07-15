#pragma once

#include <chrono>
#include <filesystem>
#include <random>
#include <string>
#include <vector>

// Phase 1~3의 순수 함수(GenerateSample/GenerateOrder/NextSampleId/NextOrderId)와
// Phase 4의 JsonIo(Load/Save)를 조합해, 실제 JSON 파일에 기존 데이터를
// 보존하면서 새 더미 데이터를 추가(append)하는 통합 로직.
struct AppendResult
{
    // 이번 호출로 새로 추가된 항목의 ID(sampleId 또는 orderId)를 생성 순서대로 담는다.
    std::vector<std::string> addedIds;
};

// samplesFile에 count개의 더미 Sample을 생성해 추가한다.
// clearExisting이 true면 기존 데이터를 버리고 새로 생성한 데이터로 덮어쓴다
// (기본값은 false = append).
AppendResult AppendDummySamples(
    const std::filesystem::path& samplesFile,
    int count,
    std::mt19937& rng,
    bool clearExisting = false);

// ordersFile에 count개의 더미 Order를 생성해 추가한다. Order의 sampleId는
// samplesFile에 이미 존재하는 Sample만 참조한다(이번 호출에서 Sample을 새로
// 만들지는 않는다 - Sample은 AppendDummySamples로 별도 생성한다).
// samplesFile에 Sample이 하나도 없으면 std::invalid_argument를 던진다.
AppendResult AppendDummyOrders(
    const std::filesystem::path& samplesFile,
    const std::filesystem::path& ordersFile,
    int count,
    std::mt19937& rng,
    std::chrono::system_clock::time_point now,
    bool clearExisting = false);
