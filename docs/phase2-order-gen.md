# Phase 2 — Order 더미 생성

[← Plan.md](./Plan.md) · 선행: [Phase 1 — Sample 생성](./phase1-sample-gen.md)

## 목표

"난수 시드 + 기존 Sample 목록 → 유효한 Order 값 객체" 순수 함수를 만든다.
반드시 존재하는 `SampleId`를 참조하고, 상태(Status)에 따라 그럴듯한 필드
조합(특히 `createdAt`, `quantity`)을 생성한다.

## 작성할 테스트 목록 (TDD Red 단계)

1. `GenerateOrder_SampleIdAlwaysReferencesExistingSample`
   - 시드 200개 × Sample 목록(1개, 3개, 10개인 경우 각각) → 생성된
     `order.sampleId`가 항상 입력 Sample 목록의 `sampleId` 중 하나.
2. `GenerateOrder_QuantityAlwaysPositive`
   - `quantity >= 1`.
3. `GenerateOrder_StatusAlwaysOneOfFiveValidValues`
   - 상태가 `RESERVED/REJECTED/PRODUCING/CONFIRMED/RELEASE` 중 하나.
4. `GenerateOrder_ReleaseStatusHasPastCreatedAt`
   - status가 `RELEASE`(혹은 `CONFIRMED`)로 생성된 케이스에서 `createdAt`이
     "현재"(테스트에 주입된 기준 시각)보다 과거인지 확인.
   - 기준 시각을 테스트에서 주입할 수 있도록 함수 시그니처에
     `std::chrono::system_clock::time_point now`를 파라미터로 받는다(현재
     시각에 대한 하드 의존 제거 → 테스트 가능성 확보).
5. `GenerateOrder_ReservedStatusHasRecentOrPastCreatedAt`
   - `RESERVED`는 과거~현재 사이(미래 금지) 시각.
6. `GenerateOrder_EmptySampleListThrowsOrReturnsError`
   - 입력 Sample 목록이 비어 있으면 명시적으로 예외를 던지거나(`std::invalid_argument`)
     `std::optional<DummyOrder>`로 실패를 표현한다 — 동작을 테스트로 고정.
7. `GenerateOrder_IsDeterministicForSameSeed`
   - Phase 1과 동일한 결정론 검증.
8. `GenerateOrder_CustomerNameIsNeverEmpty`

## 구현할 클래스/함수/파일

- `Dummy/Project1/Project1/OrderStatus.h`
  - `enum class OrderStatus { Reserved, Rejected, Producing, Confirmed, Release };`
  - 문자열 변환 헬퍼(`ToString(OrderStatus)`) — Phase 4 JSON 직렬화에서 재사용.
- `Dummy/Project1/Project1/DummyOrder.h`
  - `struct DummyOrder { std::string orderId; std::string sampleId; std::string customerName; int quantity; OrderStatus status; std::chrono::system_clock::time_point createdAt; };`
- `Dummy/Project1/Project1/OrderGenerator.h` / `.cpp`
  - `DummyOrder GenerateOrder(std::mt19937& rng, const std::vector<DummySample>& samples, std::chrono::system_clock::time_point now);`
  - 상태별 `createdAt` 규칙:
    - `RELEASE`, `CONFIRMED`: `now - uniform(1일, 30일)` 범위의 과거.
    - `PRODUCING`: `now - uniform(0일, 3일)`.
    - `RESERVED`: `now - uniform(0시간, 6시간)` (최근).
    - `REJECTED`: `now - uniform(0일, 30일)` (모니터링 제외 대상이라 제약 느슨).
  - `orderId`는 이 phase에서 placeholder로 두고 Phase 3에서 채번(Sample과
    동일하게 관심사 분리).
- `Dummy/Project1/ProjectTest/OrderGeneratorTest.cpp`

## 완료 기준

- 위 8개 테스트가 모두 통과.
- 상태-시각 규칙이 200회 반복 property 테스트로 검증됨.
- 빈 Sample 목록 처리 방식이 문서화되고 테스트로 고정됨.

## 다음 phase와의 연결점

- Phase 3의 채번 함수가 `DummyOrder.orderId`를 채운다(`ORD-YYYYMMDD-####`).
- Phase 4에서 `OrderStatus` ↔ 문자열 변환 헬퍼를 JSON 직렬화에 그대로 재사용한다.
