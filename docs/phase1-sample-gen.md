# Phase 1 — Sample 더미 생성

[← Plan.md](./Plan.md) · 전제: [Phase 0 — 가정 스키마](./phase0-schema-assumption.md)

## 목표

"난수 시드 → 유효한 Sample 값 객체" 순수 함수를 만든다. 부작용(파일 I/O,
전역 상태) 없이, 같은 입력(엔진 상태/시드)에 대해 항상 도메인 제약을 만족하는
`DummySample` 값을 생성한다.

## 작성할 테스트 목록 (TDD Red 단계)

1. `GenerateSample_YieldAlwaysInValidRange`
   - 시드 0~199(200개)에 대해 반복 생성 → 모든 결과의 `yield`가 `0.0 < yield <= 1.0`.
2. `GenerateSample_AvgProductionTimeAlwaysPositive`
   - 동일 200개 시드 → `avgProductionTime > 0.0`.
3. `GenerateSample_StockAlwaysNonNegative`
   - `stock >= 0` (integer).
4. `GenerateSample_NameIsNeverEmpty`
   - `name`이 빈 문자열이 아님.
5. `GenerateSample_IsDeterministicForSameSeed`
   - 같은 시드(같은 난수 엔진 초기 상태)로 두 번 생성하면 완전히 동일한 값이
     나옴(순수성/재현성 검증).
6. `GenerateSample_DifferentSeedsProduceVariation`
   - 여러 시드에 걸쳐 생성했을 때 모든 결과가 동일하지 않음(단조로운 고정값
     생성으로 테스트를 통과하는 꼼수를 방지).
7. (경계값) `GenerateSample_BoundarySeeds`
   - 시드 0, `UINT32_MAX` 등 극단값에서도 위 제약이 깨지지 않음.

## 구현할 클래스/함수/파일

- `Dummy/Project1/Project1/DummySample.h`
  - `struct DummySample { std::string sampleId; std::string name; double avgProductionTime; double yield; int stock; };`
  - `sampleId`는 이 phase에서는 채번하지 않고 비워두거나(placeholder), 호출부가
    Phase 3의 채번 함수와 조합해 채운다 (관심사 분리: 이 함수는 ID를 모른다).
- `Dummy/Project1/Project1/SampleGenerator.h` / `.cpp`
  - `DummySample GenerateSample(std::mt19937& rng);`
  - 내부에서 이름 후보 목록(예: 접두어 + 난수 접미어 조합)을 사용해 항상
    비어있지 않은 이름을 생성.
  - `yield`는 `std::uniform_real_distribution<double>(0.0001, 1.0)`처럼 0을
    배제하는 구간에서 생성(부동소수 경계 처리 유의 — 상한 포함/하한 배제).
  - `avgProductionTime`은 `std::uniform_real_distribution<double>(1.0, 120.0)`
    등 양수 범위에서 생성.
  - `stock`은 `std::uniform_int_distribution<int>(0, 500)`.
- `Dummy/Project1/ProjectTest/SampleGeneratorTest.cpp` (테스트 프로젝트 이름은
  기존 vcxproj 구조 확인 후 맞춤; 없다면 gtest 러너 프로젝트를 추가)

## 완료 기준

- 위 7개 테스트 케이스가 모두 통과.
- 200회 이상의 반복 루프로 property-based 스타일 검증이 실제로 수행됨(단일
  고정 케이스만으로 테스트를 작성하지 않는다).
- `SampleGenerator`가 파일 I/O나 콘솔 입출력을 전혀 참조하지 않음(순수 함수
  원칙 준수, 정적 분석 대신 코드 리뷰로 확인).

## 다음 phase와의 연결점

- Phase 2의 Order 생성 함수는 이 phase가 만든 `DummySample` 목록(특히
  `sampleId`)을 입력받아 참조 무결성을 보장해야 한다.
- Phase 3의 ID 채번 함수가 `DummySample.sampleId`를 채워주는 역할을 맡는다.
