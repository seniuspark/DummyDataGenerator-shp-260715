# Phase 4 — 파일 반영(Repository/append)

[← Plan.md](./Plan.md) · 선행: [Phase 0](./phase0-schema-assumption.md), [Phase 3](./phase3-id-allocation.md)

## 착수 전 확인 사항 (필수)

이 phase를 시작하기 전에 `Json/docs/Plan.md`(및 관련 확정 스키마 문서)가
작성되었는지 다시 확인한다.

- 확정되었다면: `Json/`이 정한 실제 필드명/파일 경로/날짜 포맷으로
  [`phase0-schema-assumption.md`](./phase0-schema-assumption.md)를 갱신하고
  이 phase를 진행한다.
- 여전히 미확정이라면: Phase 0의 가정 스키마로 진행하되, 커밋 메시지와 이
  문서 하단의 "가정 스키마로 진행함" 메모에 그 사실을 남긴다.

## 목표

Phase 1~3에서 만든 순수 함수들을 조합해, 실제 JSON 파일에 **기존 데이터를
보존하면서 새 데이터를 추가(append)**하는 통합 로직을 구현하고 검증한다.
`Json/`의 Repository 코드를 import할 수 없으므로, 이 디렉토리 안에서 최소한의
JSON 읽기/쓰기 로직을 직접 재구현한다(NuGet JSON 라이브러리 사용 가능하면
그것을 사용해도 됨 — Json/과 동일 라이브러리를 사람이 보고 맞추는 방식).

## 작성할 테스트 목록 (TDD Red 단계)

1. `AppendSamples_ToNonExistentFile_CreatesFileWithNewSamplesOnly`
   - 파일이 없는 상태에서 N개 생성 후 저장 → 파일 생성됨, N개 모두 포함.
2. `AppendSamples_ToExistingFile_PreservesOldEntriesAndAddsNew`
   - 사전에 2개의 Sample이 든 파일을 준비 → 3개 추가 생성 후 저장 → 파일에
     총 5개(기존 2 + 신규 3)가 존재하고 기존 2개의 값이 변경되지 않음(이것이
     핵심 통합 테스트).
3. `AppendSamples_NewIdsDoNotCollideWithExisting`
   - 기존 파일의 `sampleId` 최댓값 이후로 신규 ID가 이어짐(Phase 3 연동 검증).
4. `AppendOrders_ToExistingFile_PreservesOldEntriesAndAddsNew`
   - Sample과 동일한 구조로 Order에 대해 반복.
5. `AppendOrders_ReferencesOnlyExistingOrJustGeneratedSamples`
   - 새로 생성한 Order의 `sampleId`가 "이번에 새로 만든 Sample" 또는 "파일에
     이미 있던 Sample" 중 하나를 참조함(참조 무결성 통합 검증).
6. `AppendSamples_MalformedExistingFile_ReportsErrorWithoutDataLoss`
   - 손상된 JSON 파일에 대한 동작을 명확히 정의(예: 예외를 던지고 원본 파일은
     건드리지 않음)하고 테스트로 고정.
7. (선택) `AppendSamples_ClearOptionReplacesFileInsteadOfAppending`
   - 초기화(clear) 옵션을 켰을 때만 기존 데이터가 사라짐 — 옵션 미지정 시
     append가 기본임을 대조 테스트로 확인.

## 구현할 클래스/함수/파일

- `Dummy/Project1/Project1/JsonIo.h` / `.cpp`
  - `std::vector<DummySample> LoadSamples(const std::filesystem::path& file);`
  - `void SaveSamples(const std::filesystem::path& file, const std::vector<DummySample>& samples);`
  - `std::vector<DummyOrder> LoadOrders(const std::filesystem::path& file);`
  - `void SaveOrders(const std::filesystem::path& file, const std::vector<DummyOrder>& orders);`
  - 손상 파일 처리: 파싱 실패 시 `DummyDataIoException`(사용자 정의 예외)을
    던진다. 저장은 임시 파일에 쓴 뒤 rename하는 방식으로 원본 파일 손상을
    방지한다(원자적 쓰기).
- `Dummy/Project1/Project1/DummyDataAppender.h` / `.cpp`
  - `AppendResult AppendDummySamples(const std::filesystem::path& file, int count, std::mt19937& rng);`
  - `AppendResult AppendDummyOrders(const std::filesystem::path& samplesFile, const std::filesystem::path& ordersFile, int count, std::mt19937& rng, std::chrono::system_clock::time_point now);`
  - 내부에서 Load → (Phase 3 채번 + Phase 1/2 생성 반복) → 병합 → Save 순서로
    수행.
- `Dummy/Project1/ProjectTest/DummyDataAppenderTest.cpp`
  - 테스트는 `std::filesystem::temp_directory_path()` 하위에 고유 임시
    디렉토리를 만들어 실제 파일 I/O를 수행하고, 테스트 종료 후 정리한다.

## 완료 기준

- 위 6~7개 테스트가 모두 통과.
- "기존 데이터 보존 + 신규 추가" 통합 테스트(2, 4번)가 실제 파일 시스템에
  대해 수행됨(mock 없이 실제 임시 파일 사용).
- 손상 파일에 대한 동작이 명시적으로 문서화 및 테스트됨.

## 가정 스키마로 진행함 메모

- (Phase 4 착수 시점에 실제로 기록: Json/ 스키마 확정 여부와 최종 채택한
  필드/경로를 여기에 남긴다.)

## 다음 phase와의 연결점

- Phase 5의 CLI가 `AppendDummySamples`/`AppendDummyOrders`를 호출하는
  진입점 역할만 하고, 로직 자체는 이 phase에서 이미 검증된 것을 그대로 사용한다.
