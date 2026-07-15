# Phase 3 — ID 채번 전략

[← Plan.md](./Plan.md) · 선행: [Phase 1](./phase1-sample-gen.md), [Phase 2](./phase2-order-gen.md)

## 목표

기존 데이터가 있을 때/없을 때 모두 중복 없는 `SampleId`, `OrderId`를 생성하는
전략을 정하고 검증한다. 전략: **기존 최대 번호 + 1** (파일에서 로드한 기존
ID 목록을 파싱해 숫자 부분의 최댓값을 구하고 그 다음 번호부터 순차 부여).

## 작성할 테스트 목록 (TDD Red 단계)

### SampleId 채번 (`S-###`)

1. `NextSampleId_EmptyExistingIds_StartsAtS001`
   - 기존 ID 목록이 비어 있으면 `S-001`부터 시작.
2. `NextSampleId_NonEmptyExistingIds_ReturnsMaxPlusOne`
   - 기존 목록 `{S-001, S-003, S-007}` → 다음 ID는 `S-008`.
3. `NextSampleId_GeneratingMultipleIdsInOneRun_NoDuplicates`
   - 한 번 실행에서 N개를 연속 채번할 때 서로 중복되지 않음(내부적으로
     "이번 실행에서 이미 부여한 ID"도 최댓값 추적에 반영해야 함 — 이 케이스가
     빠지면 흔한 버그).
4. `NextSampleId_MalformedExistingIdIsIgnoredNotCrashed`
   - 기존 목록에 형식이 어긋나는 값(예: 빈 문자열, `SAMPLE-X`)이 섞여 있어도
     크래시하지 않고 나머지 유효한 값 기준으로 최댓값을 구함(파싱 실패 항목은
     무시 정책 — 테스트로 고정).

### OrderId 채번 (`ORD-YYYYMMDD-####`)

5. `NextOrderId_EmptyExistingIds_StartsAt0001ForGivenDate`
   - 특정 날짜(`now` 주입) 기준 기존 ID가 없으면 `ORD-YYYYMMDD-0001`.
6. `NextOrderId_NonEmptyExistingIds_SameDate_ReturnsMaxPlusOne`
   - 같은 날짜의 기존 ID 중 최댓값 + 1.
7. `NextOrderId_ExistingIdsFromDifferentDate_StartsAt0001ForNewDate`
   - 날짜가 바뀌면(`now`가 다른 날) 그 날짜 접두어로 `0001`부터 시작
     (날짜별로 채번 카운터가 독립적임을 검증).
8. `NextOrderId_GeneratingMultipleIdsInOneRun_NoDuplicates`
   - Sample과 동일하게 동일 실행 내 중복 방지.

## 구현할 클래스/함수/파일

- `Dummy/Project1/Project1/IdAllocator.h` / `.cpp`
  - `std::string NextSampleId(const std::vector<std::string>& existingSampleIds);`
  - `std::string NextOrderId(const std::vector<std::string>& existingOrderIds, std::chrono::system_clock::time_point now);`
  - 내부 헬퍼: `std::optional<int> ParseSampleIdNumber(const std::string&)`,
    `std::optional<std::pair<std::string /*date*/, int>> ParseOrderIdNumber(const std::string&)`.
  - "한 번의 실행에서 여러 개 채번" 요구를 만족하기 위해, 호출부(Phase 5 CLI 또는
    Phase 4 통합 로직)는 매번 새 ID를 기존 목록에 append한 뒤 다시
    `NextSampleId`/`NextOrderId`를 호출하거나, `IdAllocator`가 상태를 갖는
    간단한 클래스(`SampleIdAllocator`)로 시퀀스를 미리 여러 개 뽑아내는 두 방식
    중 하나를 선택한다. 테스트 4/8번이 통과하도록 구현 방식을 결정한다(권장:
    상태 없는 순수 함수 + 호출부에서 로컬 벡터에 즉시 append하는 방식 — 부작용
    없는 함수 원칙 유지가 더 쉬움).

## 완료 기준

- 위 8개 테스트 모두 통과.
- 기존 데이터 있음/없음 두 경우가 각각 최소 1개 이상의 테스트로 커버됨.
- 동일 실행 내 다건 채번 시 중복 없음이 명시적으로 검증됨.

## 다음 phase와의 연결점

- Phase 4의 통합 로직이 "파일에서 기존 ID 목록 로드 → N개 채번 → Sample/Order
  값 채우기 → 파일에 append 저장"의 순서로 이 phase의 함수들을 호출한다.
