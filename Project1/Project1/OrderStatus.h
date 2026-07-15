#pragma once

#include <string>

// Json/ 확정 스키마와 1:1 대응하는 주문 상태 5종.
// 흐름: RESERVED -> (거절: REJECTED) | (승인: CONFIRMED 또는 PRODUCING -> CONFIRMED) -> RELEASE
enum class OrderStatus
{
    Reserved,
    Rejected,
    Producing,
    Confirmed,
    Release,
};

// JSON 직렬화(Phase 4)에서 재사용할 문자열 변환 헬퍼.
std::string ToString(OrderStatus status);
