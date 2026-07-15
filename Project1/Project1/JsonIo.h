#pragma once

#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "DummyOrder.h"
#include "DummySample.h"

// Json/ PoC(DataPersistence-shp-260715)가 확정한 스키마(camelCase,
// {"samples":[...]} / {"orders":[...]} 래핑)를 그대로 재구현한 최소 JSON
// 읽기/쓰기 계층. Dummy는 Json/의 코드를 import할 수 없으므로 이 파일 안에서
// 필요한 만큼만 다시 구현한다.
//
// - Sample: data/samples.json, {"samples": [{sampleId, name,
//   avgProductionTime, yield, stock}, ...]}
// - Order: data/orders.json, {"orders": [{orderId, sampleId, customerName,
//   quantity, status, createdAt(ISO8601 "YYYY-MM-DDTHH:MM:SS")}, ...]}
//
// 손상된(파싱 불가/스키마 불일치) 파일을 읽으면 DummyDataIoException을
// 던진다. 저장은 임시 파일에 쓴 뒤 원자적으로 rename하여 원본 파일이 쓰기
// 도중 손상되지 않도록 한다.

class DummyDataIoException : public std::runtime_error
{
public:
    explicit DummyDataIoException(const std::string& message) : std::runtime_error(message)
    {
    }
};

std::vector<DummySample> LoadSamples(const std::filesystem::path& file);
void SaveSamples(const std::filesystem::path& file, const std::vector<DummySample>& samples);

std::vector<DummyOrder> LoadOrders(const std::filesystem::path& file);
void SaveOrders(const std::filesystem::path& file, const std::vector<DummyOrder>& orders);

// JsonIo.cpp 내부에서도 쓰이는 ISO8601 변환 헬퍼를 테스트에서 직접 검증할 수
// 있도록 공개한다. 초 단위까지만 보존한다(밀리초 이하는 버림).
std::string FormatIso8601(std::chrono::system_clock::time_point tp);
std::chrono::system_clock::time_point ParseIso8601(const std::string& text);
