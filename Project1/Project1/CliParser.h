#pragma once

#include <string>
#include <vector>

#include "CliOptions.h"

// argv를 옮겨 담은 std::vector<std::string>을 파싱해 CliOptions를 만드는 순수
// 함수. 콘솔 입출력이 없으므로 gtest로 직접 검증할 수 있다.
//
// 지원 옵션:
//   --samples N     : 생성할 Sample 개수 (기본값 5)
//   --orders N       : 생성할 Order 개수 (기본값 5)
//   --clear          : 있으면 mode == Clear (기본은 Append)
//   --data-dir PATH  : samples.json/orders.json이 위치한 디렉토리 (기본값 "data")
//
// N이 정수로 파싱되지 않거나 음수이면 std::invalid_argument를 던진다.
CliOptions ParseCliArgs(const std::vector<std::string>& args);
