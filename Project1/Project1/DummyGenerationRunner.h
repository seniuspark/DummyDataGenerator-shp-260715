#pragma once

#include <chrono>
#include <random>

#include "CliOptions.h"
#include "DummyDataAppender.h"

// CliOptions로 파싱된 실행 옵션을 바탕으로 Phase 4의 AppendDummySamples/
// AppendDummyOrders를 호출해 실제 파일에 반영하는 얇은 조립 계층.
// - dataDir/samples.json, dataDir/orders.json 파일을 사용한다.
// - mode == Clear면 Sample/Order 파일을 모두 초기화한 뒤 생성한다.
// - mode == Append(기본)면 기존 데이터를 보존한 채 추가한다.
struct DummyGenerationResult
{
    AppendResult samples;
    AppendResult orders;
};

DummyGenerationResult RunDummyGeneration(
    const CliOptions& options,
    std::mt19937& rng,
    std::chrono::system_clock::time_point now);
