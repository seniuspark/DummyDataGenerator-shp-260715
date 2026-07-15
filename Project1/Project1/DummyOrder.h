#pragma once

#include <chrono>
#include <string>

#include "OrderStatus.h"

// Json/ 확정 스키마와 1:1 대응하는 값 객체. 이 phase에서는 orderId 채번을
// 하지 않는다(Phase 3의 ID 채번 함수가 채운다).
struct DummyOrder
{
    std::string orderId;
    std::string sampleId;
    std::string customerName;
    int quantity;
    OrderStatus status;
    std::chrono::system_clock::time_point createdAt;
};
