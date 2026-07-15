#pragma once

#include <filesystem>

// CLI 인자 파싱 결과. 기본 동작은 Append(기존 데이터 보존 + 추가)이며,
// Clear는 --clear 플래그가 명시적으로 있을 때만 선택된다.
enum class RunMode
{
    Append,
    Clear,
};

struct CliOptions
{
    RunMode mode = RunMode::Append;
    int sampleCount = 5;
    int orderCount = 5;
    std::filesystem::path dataDir = "data";
};
