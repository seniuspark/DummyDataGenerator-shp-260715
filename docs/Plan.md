# Dummy 데이터 생성 Tool PoC — 전체 계획

이 문서는 `Dummy/` (DummyDataGenerator-shp-260715) PoC의 phase별 전체 설계다.
목표와 도메인 제약은 [`../CLAUDE.md`](../CLAUDE.md)와 상위
[`../../CLAUDE.md`](../../CLAUDE.md)를 따른다.

## 전제 조건 (중요)

- 이 PoC는 `Json/` PoC가 정하는 저장 포맷(파일 경로, 필드명, 배열 구조 등)에
  맞춰 데이터를 파일에 써야 한다.
- 확인 시점(2026-07-15) 기준 `Json/docs/`에 아직 `Plan.md`/스키마 문서가
  없다(디렉토리만 존재, 파일 없음). 따라서 **Phase 3 이후(실제 파일 I/O가
  필요한 단계)를 시작하기 전에 `Json/docs/Plan.md` 및 관련 구현이 확정되었는지
  반드시 재확인해야 한다.**
- 스키마 미확정 상태에서는 Phase 1~2(순수 함수, 메모리 상의 엔티티 생성)까지만
  진행하고, 파일 스키마를 가정해야 하는 부분(필드명, 날짜 포맷, 파일 경로)은
  이 문서와 phase 문서에 "가정(assumption)"으로 명시해 추후 스키마 확정 시
  손쉽게 교체할 수 있게 한다.
- Dummy는 Json/의 코드를 import할 수 없으므로(별도 repo), Json/이 확정한
  스키마를 사람이 읽고 이 디렉토리 안에서 최소한의 JSON 직렬화 로직을
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
- Phase 4를 시작하기 전 반드시 `Json/docs/Plan.md`(또는 그에 준하는 확정 스키마
  문서)를 다시 확인한다. 스키마가 여전히 미확정이면 Phase 0에서 정한 가정
  스키마로 진행하되, 이 사실을 커밋 메시지/문서에 남긴다.

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
