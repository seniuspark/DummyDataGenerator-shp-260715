#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// 기존 ID 목록(파일에서 로드했다고 가정) -> 다음에 부여할 SampleId/OrderId를
// 계산하는 순수 함수. 파일 I/O, 콘솔 입출력 등 부작용이 없다.
//
// 전략: 기존 최대 번호 + 1. 형식이 어긋나는(파싱 불가능한) 기존 ID는 무시하고
// 나머지 유효한 값 기준으로 최댓값을 구한다. 유효한 값이 하나도 없으면 1부터
// 시작한다.
//
// 한 번의 실행에서 여러 개를 연속 채번하려면, 호출부가 매번 새로 만든 ID를
// existingSampleIds/existingOrderIds에 append한 뒤 다시 호출한다(상태를 갖지
// 않는 순수 함수 설계를 유지하기 위함).

// "S-###" 형식에서 숫자 부분을 파싱한다. 형식이 어긋나면 std::nullopt.
std::optional<int> ParseSampleIdNumber(const std::string& sampleId);

// existingSampleIds 중 파싱 가능한 값의 최댓값 + 1로 다음 SampleId를 만든다.
// 파싱 가능한 값이 없으면 "S-001".
std::string NextSampleId(const std::vector<std::string>& existingSampleIds);

// "ORD-YYYYMMDD-####" 형식에서 (날짜 문자열, 번호) 쌍을 파싱한다.
// 형식이 어긋나면 std::nullopt.
std::optional<std::pair<std::string, int>> ParseOrderIdNumber(const std::string& orderId);

// existingOrderIds 중 now와 같은 날짜(YYYYMMDD)를 가진 값의 최댓값 + 1로
// 다음 OrderId를 만든다. 같은 날짜의 값이 없으면 해당 날짜의 "0001"부터
// 시작한다.
std::string NextOrderId(
    const std::vector<std::string>& existingOrderIds,
    std::chrono::system_clock::time_point now);
