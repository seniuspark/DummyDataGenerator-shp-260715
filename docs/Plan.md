# Dummy 데이터 생성 Tool PoC — 전체 계획

이 문서는 `Dummy/` (DummyDataGenerator-shp-260715) PoC의 phase별 전체 설계다.
목표와 도메인 제약은 [`../CLAUDE.md`](../CLAUDE.md)와 상위
[`../../CLAUDE.md`](../../CLAUDE.md)를 따른다.

## 전제 조건 (중요)

- 이 PoC는 Json 저장소(별도 Git repository)가 정하는 저장 포맷(파일 경로,
  필드명, 배열 구조 등)에 맞춰 데이터를 파일에 써야 한다.
- 착수 시점(2026-07-15)에는 Json 저장소가 아직 스키마를 확정하지 않은
  상태였다. 스키마 미확정 상태에서는 Phase 1~2(순수 함수, 메모리 상의 엔티티
  생성)까지만 진행하고, 파일 스키마를 가정해야 하는 부분(필드명, 날짜 포맷,
  파일 경로)은 이 문서와 phase 문서에 "가정(assumption)"으로 명시해 추후
  스키마 확정 시 손쉽게 교체할 수 있게 했다. Phase 3 이후(실제 파일 I/O가
  필요한 단계) 착수 전에 Json 저장소의 확정 스키마와 이 가정을 사람이 직접
  대조했다(결과는 Phase 4 문서의 "가정 스키마로 진행함" 메모 참고).
- Dummy는 Json 저장소의 코드를 import할 수 없으므로(별도 repo), 그 저장소가
  확정한 스키마를 사람이 읽고 이 디렉토리 안에서 최소한의 JSON 직렬화 로직을
  다시 구현한다(중복 허용, 공유 금지).

## Phase 목록

| Phase | 이름 | 목표 | 선행 조건 | 완료 기준(DoD) |
|---|---|---|---|---|
| 0 | 가정 스키마 고정 | Json/ 스키마 미확정에 대비해 Dummy 자체의 임시 JSON 스키마(파일 경로/필드명)를 문서로 못박는다 | 없음 | `docs/schema-assumption.md` 작성, 필드/타입/파일 경로 확정 |
| 1 | Sample 순수 생성 함수 | 시드 → 유효한 Sample 값 객체를 만드는 순수 함수 구현 및 property-based 검증 | Phase 0 | 다양한 시드(예: 200개)에 대해 Yield∈(0,1], AvgProductionTime>0, Stock≥0 항상 성립 |
| 2 | Order 순수 생성 함수 | 시드 + 기존 Sample 목록 → 유효한 Order 값 객체 생성, 상태별 그럴듯한 조합 | Phase 1 | SampleId 참조 무결성, 상태별 CreatedAt/Quantity 규칙 항상 성립 |
| 3 | ID 채번 전략 | 기존 데이터 유/무 모두에서 중복 없는 SampleId/OrderId 생성 | Phase 1, 2 | 빈 목록/비어있지 않은 목록 각각에서 새 ID가 기존과 중복되지 않음을 테스트로 검증 |
| 4 | 파일 반영(Repository) | Json/ 가정 스키마에 맞춰 JSON 파일에 append 방식으로 실제 반영 | Phase 0, 3, (Json/ 스키마 재확인) | 기존 파일 데이터 보존 + 신규 데이터 추가가 통합 테스트로 검증됨 |
| 5 | 콘솔 CLI | 생성 개수를 인자/입력으로 받아 실행하는 콘솔 진입점, 초기화 옵션(비기본) | Phase 4 | CLI 실행 시 기본 동작이 append이며, 초기화 옵션은 별도 플래그로만 동작함을 확인 |

## Phase 간 의존관계

```
Phase 0 (스키마 가정)
   │
   ▼
Phase 1 (Sample 생성) ──► Phase 2 (Order 생성)
   │                          │
   └────────────┬─────────────┘
                ▼
        Phase 3 (ID 채번)
                │
                ▼
        Phase 4 (파일 반영) ◄── Json/ 스키마 재확인 필요
                │
                ▼
        Phase 5 (콘솔 CLI)
```

- Phase 1과 Phase 2는 둘 다 순수 함수이므로 로직상 독립적으로 작성 가능하지만,
  Order 생성이 Sample 목록을 입력으로 받으므로 Phase 2 테스트 작성 시 Phase 1의
  Sample 생성 함수(또는 테스트용 고정 Sample 목록)를 사용한다.
- Phase 4를 시작하기 전 Json 저장소의 확정 스키마 문서와 Phase 0의 가정
  스키마를 사람이 직접 대조했다(결과는 [Phase 4](./phase4-file-append.md)
  문서의 "가정 스키마로 진행함" 메모 참고). 이후 통합 시점에 스키마가
  달라졌다면 통합 담당자가 다시 대조해 갱신한다.

## 세부 계획 문서

- [Phase 0 — 가정 스키마 고정](./phase0-schema-assumption.md)
- [Phase 1 — Sample 더미 생성](./phase1-sample-gen.md)
- [Phase 2 — Order 더미 생성](./phase2-order-gen.md)
- [Phase 3 — ID 채번 전략](./phase3-id-allocation.md)
- [Phase 4 — 파일 반영(Repository/append)](./phase4-file-append.md)
- [Phase 5 — 콘솔 CLI](./phase5-cli.md)

## 공통 원칙 (모든 phase 공통)

- TDD Red → Green → Refactor를 지킨다. 각 phase 문서의 "작성할 테스트 목록"을
  실패하는 테스트로 먼저 작성한 뒤 구현한다.
- 난수 생성과 도메인 규칙(순수 함수)을 분리한다: `std::mt19937` 등 엔진은
  호출부에서 주입하고, "시드값 → 엔티티" 변환 자체는 부작용 없는 함수로 만들어
  같은 시드에 대해 같은 결과가 나오는지도 검증한다(결정론 테스트).
- 도메인 제약은 상위 `../../CLAUDE.md`의 값을 그대로 따른다(Yield 0.0 초과 1.0
  이하, AvgProductionTime 양수, 주문 상태 5종 등).
- 기능 단위로 작게 커밋한다.

## PoC 완료 요약 (Phase 0~5, 2026-07-15)

Phase 0~5 전 단계가 TDD(Red → Green → Refactor)로 완료됐다. gtest 50건이
모두 통과한다(`Project1Tests.exe`).

| Phase | 산출물 | 테스트 |
|---|---|---|
| 0 | `docs/phase0-schema-assumption.md` (Json/ 스키마 가정 고정) | - |
| 1 | `DummySample.h`, `SampleGenerator.h/.cpp` | `SampleGeneratorTest.cpp` |
| 2 | `DummyOrder.h`, `OrderStatus.h/.cpp`, `OrderGenerator.h/.cpp` | `OrderGeneratorTest.cpp` |
| 3 | `IdAllocator.h/.cpp` | `IdAllocatorTest.cpp` |
| 4 | `JsonIo.h/.cpp`, `DummyDataAppender.h/.cpp` | `JsonIoTest.cpp`, `DummyDataAppenderTest.cpp` |
| 5 | `CliOptions.h`, `CliParser.h/.cpp`, `DummyGenerationRunner.h/.cpp`, `main.cpp` | `CliParserTest.cpp`, `DummyGenerationRunnerTest.cpp` |

### Phase 5 콘솔 CLI 사용법

```
Project1.exe [--samples N] [--orders N] [--clear] [--data-dir PATH]
```

- 옵션이 전혀 없으면 `samples=5`, `orders=5`, `data-dir=data`, append 모드로
  실행된다(인자 없이 실행해도 기본 동작은 append).
- `--samples`/`--orders`는 0 이상의 정수만 허용하며, 그렇지 않으면
  `--samples must be a non-negative integer, got '...'` 형태의 에러 메시지를
  출력하고 종료코드 1을 반환한다(파일은 변경되지 않는다).
- `--clear`가 있을 때만 기존 `samples.json`/`orders.json`을 버리고 새로
  생성한다. `--clear`가 없으면 항상 append.
- `--data-dir`로 지정한 디렉토리 하위의 `samples.json`, `orders.json`을
  읽고 쓴다(디렉토리가 없으면 생성).

### 수동 스모크 테스트 결과 (2026-07-15)

- `Project1.exe --samples 5 --orders 10 --data-dir <tmp>`: `samples.json`에
  5건, `orders.json`에 10건이 생성됨을 확인(Yield 0 초과 1 이하,
  AvgProductionTime 양수, SampleId 참조 무결성 확인).
- 동일 디렉토리에 `--samples 2 --orders 3`을 다시 실행 → 기존 7/13건 유지,
  ID가 이어서 채번됨(`S-006`, `S-007`, `ORD-...-0011~0013`)을 확인(append
  기본 동작 검증).
- `--clear --samples 1 --orders 1` 실행 → 기존 데이터가 사라지고 각각 1건씩
  (`S-001`, `ORD-...-0001`)만 남음을 확인.
- `--samples -1` (잘못된 인자) 실행 → 종료코드 1, 에러 메시지 출력, 파일
  변경 없음을 확인.
- 인자 없이 실행 → 기본값(`samples=5`, `orders=5`, `data-dir=data`)으로
  append 동작함을 확인.

### CLAUDE.md 목표 대비 최종 점검

- 도메인 제약을 지키는 더미 데이터 생성: Phase 1~2의 property-based
  테스트(Yield∈(0,1], AvgProductionTime>0, SampleId 참조 무결성, 상태별
  CreatedAt 규칙)로 검증됨.
- 실제 파일 반영(append 기본, clear는 옵션): Phase 4/5 통합 테스트 +
  수동 스모크 테스트로 검증됨.
- ID 채번(충돌 방지, 기존 데이터 유무 모두): Phase 3 테스트로 검증됨.
- 콘솔 CLI 실행: Phase 5에서 `main.cpp`(View 계층) + `CliParser`(순수 함수)
  분리로 구현, 실행 파일로 스모크 테스트 완료.
- 이 PoC는 목표를 충족했다고 판단하며 완료 상태로 종료한다.
