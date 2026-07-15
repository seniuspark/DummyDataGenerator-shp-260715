#pragma once

#include <random>

#include "DummySample.h"

// 시드(엔진 상태) -> 유효한 DummySample 값을 만드는 순수 함수.
// 파일 I/O, 콘솔 입출력 등 부작용이 없다. sampleId는 채우지 않는다.
DummySample GenerateSample(std::mt19937& rng);
