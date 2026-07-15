# Phase 5 — 콘솔 CLI

[← Plan.md](./Plan.md) · 선행: [Phase 4 — 파일 반영](./phase4-file-append.md)

## 목표

Phase 4까지 검증된 로직을 감싸는 콘솔 진입점(`main`)을 만든다. 생성할
시료/주문 개수를 커맨드라인 인자 또는 대화형 입력으로 받고, 기본 동작은
append이며 초기화(clear) 옵션은 별도 플래그로만 활성화된다.

## 작성할 테스트 목록 (TDD Red 단계)

CLI의 입출력 자체는 gtest로 직접 검증하기 어려우므로, "인자 파싱"과
"실행 로직 조합"을 분리하여 파싱 부분만 순수 함수로 테스트한다.

1. `ParseCliArgs_DefaultsToAppendModeWhenNoClearFlag`
   - 인자 없이 실행 시 `mode == Append`.
2. `ParseCliArgs_ClearFlagSwitchesToClearMode`
   - `--clear` 인자가 있을 때만 `mode == Clear`.
3. `ParseCliArgs_SampleCountAndOrderCountParsedFromArgs`
   - `--samples 10 --orders 5` → `sampleCount == 10`, `orderCount == 5`.
4. `ParseCliArgs_MissingCountsFallBackToDefaults`
   - 인자를 생략하면 문서화된 기본값(예: samples=5, orders=5)이 적용됨.
5. `ParseCliArgs_InvalidCountThrowsOrReportsError`
   - `--samples -1` 등 음수/비정수 입력에 대한 동작을 정의하고 테스트로 고정
     (예: `std::invalid_argument`).
6. `ParseCliArgs_DataDirOptionOverridesDefaultPath`
   - `--data-dir <path>` 지정 시 그 경로 하위의 `samples.json`/`orders.json`을
     사용.
7. (통합, 선택) `RunDummyGeneration_AppendModeAddsRequestedCounts`
   - 파싱된 옵션으로 Phase 4의 `AppendDummySamples`/`AppendDummyOrders`를
     실제로 호출해, 임시 디렉토리에 지정 개수만큼 추가되는지 확인(경량 통합
     테스트, Phase 4 테스트와 중복 최소화를 위해 개수만 검증).

## 구현할 클래스/함수/파일

- `Dummy/Project1/Project1/CliOptions.h`
  - `enum class RunMode { Append, Clear };`
  - `struct CliOptions { RunMode mode = RunMode::Append; int sampleCount = 5; int orderCount = 5; std::filesystem::path dataDir = "data"; };`
- `Dummy/Project1/Project1/CliParser.h` / `.cpp`
  - `CliOptions ParseCliArgs(int argc, char** argv);` (순수 함수에 가깝게,
    `argc`/`argv`를 `std::vector<std::string>`으로 변환한 뒤 파싱하는 내부
    함수를 별도로 두면 테스트가 더 쉬워짐 — `CliOptions ParseCliArgs(const std::vector<std::string>& args);`를 실제 테스트 대상으로 하고 `main`에서 래핑).
- `Dummy/Project1/Project1/main.cpp`
  - 인자 파싱 → (Clear 모드면 파일 초기화 후) → Phase 4 append 함수 호출 →
    결과 요약을 콘솔에 출력.
  - 대화형 모드(인자가 전혀 없을 때 개수를 입력받는 방식)를 추가할 경우, 입력
    받는 부분은 `main.cpp`(View 성격)에만 두고 `CliParser`/생성 로직과는
    분리한다.
- `Dummy/Project1/ProjectTest/CliParserTest.cpp`

## 완료 기준

- 위 6개(+선택 1개) 테스트가 모두 통과.
- `main.cpp`가 얇은 조립 계층으로 유지되고(테스트 가능한 로직은 모두
  `CliParser`/Phase 1~4 함수로 분리), gtest 대상이 아닌 코드는 최소화됨.
- 인자 없이 실행했을 때 append가 기본 동작임을 코드/테스트 양쪽에서 확인 가능.

## 다음 단계 (PoC 종료 후)

- 이 PoC에서 검증된 생성 로직/ID 채번 로직/CLI 옵션 설계를 `Main/`에 통합할 때
  참고 자료로 사람이 다시 읽고 재구현한다(코드 직접 복사 여부는 `Main/` 쪽
  CLAUDE.md 방침을 따른다).
