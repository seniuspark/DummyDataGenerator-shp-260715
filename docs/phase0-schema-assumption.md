# Phase 0 — 가정 스키마 고정

[← Plan.md](./Plan.md)

## 목표

JSON 저장 스키마(파일 경로, 필드명, 타입, 날짜 포맷)를 이 저장소 자체가
가정하는 형태로 문서에 못박는다. Phase 4(파일 반영) 착수 시점에 이 가정
스키마를 채택한 실제 구현과 대조해 확정했다(결과는 `Plan.md`의 "가정
스키마로 진행함" 메모 참고).

이 phase는 순수 문서 작업이며 테스트가 없다(코드 산출물 없음).

## 가정 스키마 (2026-07-15 기준, Json/ 미확정)

### 파일 경로

- `data/samples.json`
- `data/orders.json`
- 실행 위치에 독립적이도록, 실행 파일 기준 상대경로가 아니라 exe 인자 또는
  환경변수로 데이터 루트를 지정할 수 있게 한다(Phase 5에서 CLI 옵션으로 노출).

### Sample (`data/samples.json`)

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

| 필드 | 타입 | 제약 |
|---|---|---|
| sampleId | string | `S-###` 형식, 유일 |
| name | string | 비어있지 않음 |
| avgProductionTime | number(double) | 0 초과 |
| yield | number(double) | 0.0 초과, 1.0 이하 |
| stock | integer | 0 이상 |

### Order (`data/orders.json`)

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

| 필드 | 타입 | 제약 |
|---|---|---|
| orderId | string | `ORD-YYYYMMDD-####` 형식, 유일 |
| sampleId | string | `samples.json`에 존재하는 sampleId 참조 |
| customerName | string | 비어있지 않음 |
| quantity | integer | 1 이상 |
| status | string | `RESERVED\|REJECTED\|PRODUCING\|CONFIRMED\|RELEASE` 중 하나 |
| createdAt | string(ISO 8601, `YYYY-MM-DDTHH:MM:SS`) | RELEASE/CONFIRMED류는 과거 시각이 더 그럴듯함 |

## 완료 기준

- 이 md 파일이 존재하고 위 필드/타입/경로가 명시됨.
- Phase 1~4 착수 전 이 문서를 참조 지점으로 삼는다.

## 다음 phase와의 연결점

- Phase 1, 2의 순수 함수가 반환하는 값 객체(C++ struct)는 이 표의 필드를
  1:1로 대응하도록 설계한다.
- Phase 4 착수 시점에 이 가정 스키마를 채택한 실제 구현과 대조해 확정했다(자세한
  내용은 `Plan.md`의 "가정 스키마로 진행함" 메모 참고). 이후 스키마 변경이
  필요해지면(예: 통합 담당자가 Json 저장소의 최신 스키마 문서와 사람이 직접
  비교해 차이를 발견한 경우) 이 문서와 Phase 1~4 구현을 함께 갱신한다.
