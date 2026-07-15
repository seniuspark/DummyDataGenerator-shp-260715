#pragma once

#include <chrono>
#include <random>
#include <vector>

#include "DummyOrder.h"
#include "DummySample.h"

// 시드(엔진 상태) + 기존 Sample 목록 + 기준 시각 -> 유효한 DummyOrder 값을
// 만드는 순수 함수. 파일 I/O, 콘솔 입출력 등 부작용이 없다. orderId는
// 채우지 않는다.
//
// samples가 비어 있으면 std::invalid_argument를 던진다(참조할 SampleId가
// 없으므로 유효한 Order를 만들 수 없다).
DummyOrder GenerateOrder(
    std::mt19937& rng,
    const std::vector<DummySample>& samples,
    std::chrono::system_clock::time_point now);
