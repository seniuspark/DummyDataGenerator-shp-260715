#pragma once

#include <string>

// Json/ 확정 스키마(camelCase, data/samples.json, {"samples":[...]})와
// 1:1 대응하는 값 객체. 이 phase에서는 sampleId 채번을 하지 않는다
// (Phase 3의 ID 채번 함수가 채운다).
struct DummySample
{
    std::string sampleId;
    std::string name;
    double avgProductionTime;
    double yield;
    int stock;
};
