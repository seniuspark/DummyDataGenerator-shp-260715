# DummyDataGenerator-shp-260715

반도체 시료 생산주문관리 시스템의 **Dummy 데이터 생성 Tool PoC**다.
`Json/` PoC가 정한(또는 정할 예정인) JSON 저장 스키마에 맞춰, 도메인 제약을
만족하는 시료(Sample)/주문(Order) 더미 데이터를 생성하고 실제 JSON 파일에
**추가(append)** 하는 콘솔 CLI 도구를 검증한다.

이 저장소는 독립된 Git Repository이며, 상위 프로젝트(`producing/`)의
`MVC/`, `Json/`, `Monitor/`, `Main/` 디렉토리와는 코드 공유 없이 완전히
분리되어 있다. `Json/`의 스키마는 사람이 읽고 이 저장소 안에서 최소한의
JSON 읽기/쓰기 로직을 다시 구현했다(코드 import 없음).

## 디렉토리 구조

```
Dummy/
├── CLAUDE.md                       # 이 PoC의 목표/제약/TDD 방침
├── README.md                       # 이 문서
├── docs/
│   ├── Plan.md                     # Phase 0~5 전체 계획 및 완료 요약
│   ├── phase0-schema-assumption.md # Json/ 확정 스키마 가정(파일 경로/필드/타입)
│   ├── phase1-sample-gen.md        # Sample 순수 생성 함수 계획
│   ├── phase2-order-gen.md         # Order 순수 생성 함수 계획
│   ├── phase3-id-allocation.md     # ID 채번 전략 계획
│   ├── phase4-file-append.md       # JSON 파일 append 계획
│   └── phase5-cli.md               # 콘솔 CLI 계획
└── Project1/                       # Visual Studio 솔루션 (C++20)
    ├── Project1.slnx
    ├── Project1/                   # 실행 파일 프로젝트
    │   ├── Project1.vcxproj
    │   ├── packages.config         # nlohmann.json 3.11.3
    │   ├── DummySample.h           # Sample 값 객체
    │   ├── SampleGenerator.h/.cpp  # 시드 -> 유효한 Sample (순수 함수)
    │   ├── OrderStatus.h/.cpp      # 주문 상태 5종 enum + 문자열 변환
    │   ├── DummyOrder.h            # Order 값 객체
    │   ├── OrderGenerator.h/.cpp   # 시드+Sample목록 -> 유효한 Order (순수 함수)
    │   ├── IdAllocator.h/.cpp      # SampleId/OrderId 채번 (기존 최대값+1)
    │   ├── JsonIo.h/.cpp           # samples.json/orders.json 읽기/쓰기
    │   ├── DummyDataAppender.h/.cpp# 생성+채번+파일반영 통합(append 기본)
    │   ├── CliOptions.h            # CLI 옵션 구조체(RunMode 등)
    │   ├── CliParser.h/.cpp        # argv -> CliOptions 파싱(순수 함수)
    │   ├── DummyGenerationRunner.h/.cpp # CliOptions -> 실제 실행 조립 계층
    │   └── main.cpp                # 콘솔 진입점(View 계층)
    └── Project1Tests/              # gtest 프로젝트
        ├── Project1Tests.vcxproj
        ├── packages.config         # Microsoft.googletest.v140... , nlohmann.json
        ├── SampleGeneratorTest.cpp
        ├── OrderGeneratorTest.cpp
        ├── IdAllocatorTest.cpp
        ├── JsonIoTest.cpp
        ├── DummyDataAppenderTest.cpp
        ├── CliParserTest.cpp
        └── DummyGenerationRunnerTest.cpp
```

(`.vs/`, `x64/`, `packages/` 등 빌드 산출물/IDE 캐시 디렉토리는 `.gitignore`로
제외되어 저장소에는 커밋되지 않는다.)

## 핵심 기능

### 1. Sample 순수 생성 함수 — `SampleGenerator.h/.cpp`

```cpp
DummySample GenerateSample(std::mt19937& rng);
```

시드가 주입된 `std::mt19937` 엔진 하나만 받아 `DummySample`(`DummySample.h`)
값 객체를 만드는 순수 함수다. 파일 I/O나 콘솔 출력 같은 부작용이 없으며,
`sampleId`는 채우지 않는다(빈 문자열로 남겨 Phase 3의 채번 함수가 채운다).
생성되는 값은 항상 도메인 제약을 만족한다.

- `yield`: 0.0 초과, 1.0 이하
- `avgProductionTime`: 0 초과(양수)
- `stock`: 0 이상

`SampleGeneratorTest.cpp`에서 다수의 시드에 대해 위 제약이 항상 성립하는지
property-based 스타일로 검증한다.

### 2. Order 순수 생성 함수 — `OrderGenerator.h/.cpp`

```cpp
DummyOrder GenerateOrder(
    std::mt19937& rng,
    const std::vector<DummySample>& samples,
    std::chrono::system_clock::time_point now);
```

시드 + 기존 Sample 목록 + 기준 시각을 받아 `DummyOrder`(`DummyOrder.h`) 값
객체를 만드는 순수 함수다. `orderId`는 채우지 않는다. `samples`가 비어 있으면
참조할 SampleId가 없으므로 `std::invalid_argument`를 던진다.

- `sampleId`는 항상 `samples` 목록 중 하나를 참조한다(참조 무결성 보장).
- `status`는 `OrderStatus.h`에 정의된 5종(`Reserved/Rejected/Producing/
  Confirmed/Release`) 중 하나이며, 상태별로 `createdAt`이 그럴듯한 값이 되도록
  구성한다(예: `Release`/`Confirmed`류는 과거 시각).
- `quantity`는 1 이상.

`OrderGeneratorTest.cpp`에서 다수의 시드에 대해 SampleId 참조 무결성과
상태별 `createdAt` 규칙이 항상 성립하는지 검증한다.

### 3. ID 채번 — `IdAllocator.h/.cpp`

```cpp
std::optional<int> ParseSampleIdNumber(const std::string& sampleId);
std::string NextSampleId(const std::vector<std::string>& existingSampleIds);
std::optional<std::pair<std::string, int>> ParseOrderIdNumber(const std::string& orderId);
std::string NextOrderId(
    const std::vector<std::string>& existingOrderIds,
    std::chrono::system_clock::time_point now);
```

기존 ID 목록(파일에서 로드했다고 가정) → 다음에 부여할 ID를 계산하는 순수
함수다.

- **SampleId**: `S-###` 형식에서 숫자를 파싱해 기존 최댓값 + 1. 파싱 가능한
  값이 하나도 없으면 `S-001`부터 시작한다.
- **OrderId**: `ORD-YYYYMMDD-####` 형식에서 (날짜, 번호)를 파싱해, `now`와
  같은 날짜(YYYYMMDD)를 가진 값들 중 최댓값 + 1. 같은 날짜의 값이 없으면 해당
  날짜의 `0001`부터 시작한다.
- 형식이 어긋나는 기존 ID는 무시하고 나머지 유효한 값만으로 최댓값을 구한다.
- 한 번의 실행에서 여러 개를 연속 채번할 때는, 호출부가 매번 새로 만든 ID를
  기존 목록에 append한 뒤 다시 호출하는 방식으로 상태 없는(stateless) 순수
  함수 설계를 유지한다.

`IdAllocatorTest.cpp`에서 기존 데이터가 없는 경우(`S-001`/`0001`부터 시작)와
있는 경우(최댓값 + 1) 양쪽을 모두 검증한다.

### 4. JSON 파일 반영 — `JsonIo.h/.cpp`, `DummyDataAppender.h/.cpp`

`JsonIo.h/.cpp`는 `Json/` PoC가 확정한 스키마(camelCase 필드명, `{"samples":
[...]}` / `{"orders": [...]}` 래핑)를 그대로 재구현한 최소 읽기/쓰기 계층이다.

```cpp
std::vector<DummySample> LoadSamples(const std::filesystem::path& file);
void SaveSamples(const std::filesystem::path& file, const std::vector<DummySample>& samples);
std::vector<DummyOrder> LoadOrders(const std::filesystem::path& file);
void SaveOrders(const std::filesystem::path& file, const std::vector<DummyOrder>& orders);
std::string FormatIso8601(std::chrono::system_clock::time_point tp);
std::chrono::system_clock::time_point ParseIso8601(const std::string& text);
```

- 손상되었거나(파싱 불가) 스키마가 맞지 않는 파일을 읽으면
  `DummyDataIoException`을 던진다.
- 저장 시 임시 파일에 먼저 쓴 뒤 원자적으로 rename하여, 쓰기 도중 원본 파일이
  손상되지 않도록 한다.
- `createdAt`은 ISO 8601(`YYYY-MM-DDTHH:MM:SS`, 초 단위까지)로 직렬화한다.

`DummyDataAppender.h/.cpp`는 Phase 1~3의 순수 함수(`GenerateSample`,
`GenerateOrder`, `NextSampleId`, `NextOrderId`)와 `JsonIo`의 `Load`/`Save`를
조합해, **기존 데이터를 보존하면서** 새 더미 데이터를 파일에 추가하는 통합
로직이다.

```cpp
AppendResult AppendDummySamples(
    const std::filesystem::path& samplesFile,
    int count,
    std::mt19937& rng,
    bool clearExisting = false);

AppendResult AppendDummyOrders(
    const std::filesystem::path& samplesFile,
    const std::filesystem::path& ordersFile,
    int count,
    std::mt19937& rng,
    std::chrono::system_clock::time_point now,
    bool clearExisting = false);
```

- `clearExisting` 기본값은 `false`(append). `true`일 때만 기존 데이터를
  버리고 새로 생성한 데이터로 덮어쓴다.
- `AppendDummyOrders`는 `samplesFile`에 이미 저장된 Sample만 참조한다(같은
  호출에서 Sample을 새로 만들지 않음). `samplesFile`에 Sample이 하나도 없으면
  `std::invalid_argument`를 던진다.
- `AppendResult::addedIds`에 이번 호출로 새로 추가된 ID를 생성 순서대로 담아
  반환한다.

`JsonIoTest.cpp`(읽기/쓰기 왕복, 손상 파일 예외, ISO8601 변환)와
`DummyDataAppenderTest.cpp`(append 시 기존 데이터 보존 + 신규 추가, clear 시
초기화)에서 검증한다.

### 5. 콘솔 CLI — `CliOptions.h`, `CliParser.h/.cpp`, `DummyGenerationRunner.h/.cpp`, `main.cpp`

```cpp
enum class RunMode { Append, Clear };

struct CliOptions
{
    RunMode mode = RunMode::Append;
    int sampleCount = 5;
    int orderCount = 5;
    std::filesystem::path dataDir = "data";
};

CliOptions ParseCliArgs(const std::vector<std::string>& args);
```

`ParseCliArgs`는 argv를 옮겨 담은 `std::vector<std::string>`을 파싱해
`CliOptions`를 만드는 순수 함수다(콘솔 입출력 없음 → gtest로 직접 검증
가능). 옵션이 전혀 없으면 `samples=5`, `orders=5`, `data-dir=data`,
`mode=Append`가 기본값이다. `--samples`/`--orders`가 정수로 파싱되지 않거나
음수이면 `std::invalid_argument`를 던진다.

`DummyGenerationRunner.h/.cpp`는 `CliOptions`를 받아 `AppendDummySamples`/
`AppendDummyOrders`를 호출해 실제 파일에 반영하는 조립 계층이다.
`mode == Clear`면 Sample/Order 파일을 모두 초기화한 뒤 생성하고, 기본값인
`mode == Append`면 기존 데이터를 보존한 채 추가한다.

`main.cpp`는 인자 파싱 → 실행 → 결과 출력만 담당하는 View 계층이며, 도메인
로직(`CliParser`, `DummyGenerationRunner` 등)과 분리되어 있다.

`CliParserTest.cpp`, `DummyGenerationRunnerTest.cpp`에서 검증한다.

## JSON 스키마 (Json/ PoC와 동일 가정)

이 도구가 읽고 쓰는 스키마는 `docs/phase0-schema-assumption.md`에 고정한
가정 스키마이며, `Json/` PoC가 확정한 스키마와 동일하다는 전제로 구현했다.
파일 경로: `<data-dir>/samples.json`, `<data-dir>/orders.json` (기본
`data-dir`은 `data`).

`data/samples.json`:

```json
{
  "samples": [
    {
      "sampleId": "S-001",
      "name": "SiC-Wafer-A",
      "avgProductionTime": 12.5,
      "yield": 0.93,
      "stock": 120
    }
  ]
}
```

`data/orders.json`:

```json
{
  "orders": [
    {
      "orderId": "ORD-20260416-0043",
      "sampleId": "S-001",
      "customerName": "ACME",
      "quantity": 50,
      "status": "RESERVED",
      "createdAt": "2026-04-16T09:00:00"
    }
  ]
}
```

필드명은 camelCase, `status`는 `RESERVED|REJECTED|PRODUCING|CONFIRMED|
RELEASE` 중 하나, `createdAt`은 ISO 8601(`YYYY-MM-DDTHH:MM:SS`) 문자열이다.
상세 필드/타입/제약은 `docs/phase0-schema-assumption.md`를 참고한다.

## 사용 라이브러리

- [nlohmann/json](https://github.com/nlohmann/json) 3.11.3 (NuGet:
  `nlohmann.json`) — JSON 파싱/직렬화
- [GoogleTest](https://github.com/google/googletest) (NuGet:
  `Microsoft.googletest.v140.windesktop.msvcstl.dyn.rt-dyn` 1.8.1.8) — 단위
  테스트

두 패키지 모두 NuGet으로 관리되며(`packages.config`), 빌드 시 NuGet
패키지 복원이 필요하다.

## 빌드 방법

Visual Studio 2022 이상(C++20, WindowsTargetPlatformVersion 10.0)에서
`Project1/Project1.slnx`를 열거나, MSBuild로 직접 빌드한다.

```powershell
# NuGet 패키지 복원
nuget restore Project1\Project1.slnx

# 실행 파일 빌드 (x64, Release 예시)
msbuild Project1\Project1.slnx /t:Project1 /p:Configuration=Release /p:Platform=x64

# 테스트 빌드
msbuild Project1\Project1.slnx /t:Project1Tests /p:Configuration=Release /p:Platform=x64
```

## 테스트 실행 방법

빌드 후 생성된 `Project1Tests.exe`(예: `Project1\Project1Tests\x64\Release\
Project1Tests.exe` 또는 `Debug\` 경로)를 직접 실행한다.

```powershell
Project1\Project1Tests\x64\Release\Project1Tests.exe
```

또는 Visual Studio의 테스트 탐색기(Test Explorer)에서 실행할 수 있다.
2026-07-15 기준 gtest 50건이 모두 통과한다(Phase 0~5 전체 완료).

## CLI 사용법

```
Project1.exe [--samples N] [--orders N] [--clear] [--data-dir PATH]
```

| 옵션 | 설명 | 기본값 |
|---|---|---|
| `--samples N` | 생성할 Sample 개수(0 이상 정수) | `5` |
| `--orders N` | 생성할 Order 개수(0 이상 정수) | `5` |
| `--clear` | 있으면 기존 `samples.json`/`orders.json`을 버리고 새로 생성(mode=Clear) | 없음(mode=Append) |
| `--data-dir PATH` | `samples.json`/`orders.json`이 위치한 디렉토리(없으면 생성) | `data` |

- 옵션이 전혀 없으면 `samples=5`, `orders=5`, `data-dir=data`, append 모드로
  실행된다(인자 없이 실행해도 기본 동작은 append이며 기존 데이터를 보존한다).
- `--samples`/`--orders`가 0 이상의 정수가 아니면 `--samples must be a
  non-negative integer, got '...'` 형태의 에러 메시지를 출력하고 종료코드
  1을 반환한다(이 경우 파일은 변경되지 않는다).
- `--clear`가 있을 때만 기존 파일을 초기화한 뒤 생성한다. `--clear`가 없으면
  항상 append(기존 데이터 보존 + 신규 데이터 추가).

## TDD 진행 과정 (Phase 0 ~ 5)

Red → Green → Refactor를 지키며 아래 순서로 진행했다(`docs/Plan.md` 참고).

| Phase | 목표 | 산출물 | 테스트 |
|---|---|---|---|
| 0 | 가정 스키마 고정(Json/ 미확정 대비) | `docs/phase0-schema-assumption.md` | 문서만(테스트 없음) |
| 1 | 시드 → 유효한 Sample 순수 생성 함수 | `DummySample.h`, `SampleGenerator.h/.cpp` | `SampleGeneratorTest.cpp` |
| 2 | 시드+Sample목록 → 유효한 Order 순수 생성 함수 | `DummyOrder.h`, `OrderStatus.h/.cpp`, `OrderGenerator.h/.cpp` | `OrderGeneratorTest.cpp` |
| 3 | ID 채번(기존 최대값+1, 유/무 양쪽 케이스) | `IdAllocator.h/.cpp` | `IdAllocatorTest.cpp` |
| 4 | JSON 파일 append 반영(기존 데이터 보존) | `JsonIo.h/.cpp`, `DummyDataAppender.h/.cpp` | `JsonIoTest.cpp`, `DummyDataAppenderTest.cpp` |
| 5 | 콘솔 CLI(인자 파싱 + 실행 조립) | `CliOptions.h`, `CliParser.h/.cpp`, `DummyGenerationRunner.h/.cpp`, `main.cpp` | `CliParserTest.cpp`, `DummyGenerationRunnerTest.cpp` |

각 phase 시작 전 실패하는 테스트를 먼저 작성한 뒤 구현했고, 기능 단위로
작게 커밋했다. 2026-07-15 기준 gtest 50건이 모두 통과하며, 실행 파일 기준
수동 스모크 테스트(신규 생성/append 유지/clear 초기화/잘못된 인자 처리)도
`docs/Plan.md`에 기록되어 있다.

## 범위 밖 (다른 PoC의 책임)

- 데이터 조회/모니터링 UI는 이 저장소의 책임이 아니다(`Monitor/` PoC의
  책임).
- 영속성 계층 자체의 일반적인 CRUD 검증은 `Json/` PoC의 책임이다. 이
  저장소는 `Json/`의 확정 스키마를 가정해 더미 데이터 생성/추가만 검증한다.
- 이 PoC에서 검증한 생성 로직(순수 함수 설계, ID 채번, append 전략)은
  사람이 읽고 `Main/`의 초기 데이터 세팅 또는 테스트 지원 도구로 다시
  구현/통합한다(코드 직접 재사용 없음).
