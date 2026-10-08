// 제어판 화면이 보여 주는 값을 담는 틀.
// 구조체는 반드시 .ino가 아니라 이 헤더에 둔다 (아두이노가 함수 선언을 .ino 앞쪽에 자동으로 끼워 넣어서,
// .ino 안에 구조체를 두면 컴파일 에러가 나는 일을 바디캠 0.2.2에서 겪었다).
#pragma once
#include <stdint.h>

// 순서 = 버튼을 누를 때 바뀌는 순서: 자동 → 약 → 중 → 강 → 끔
enum FanMode : uint8_t { FAN_AUTO = 0, FAN_LOW = 1, FAN_MID = 2, FAN_HIGH = 3, FAN_OFF = 4 };

struct FarmView {
    bool    hub_online;     // 허브(서버)와 연결됨
    int     temp_x10;       // 온도 x10 (235 = 23.5℃). 소수를 쓰지 않고 정수로 다룬다
    int     hum;            // 습도 %
    int     lux;            // 조도 lx
    int     soil[3];        // 화분 수분 % (3개)
    bool    water_ok;       // 물탱크 충분
    int     pump_today;     // 오늘 급수 횟수
    int     pump_limit;     // 하루 한도 (노드가 지키는 값과 같음: 6)
    bool    pump_on;        // 펌프 동작 중
    int     pump_left_s;    // 남은 급수 초
    bool    mode_auto;      // 자동 모드
    bool    light_on;       // 조명 켜짐
    FanMode fan_mode;       // 환풍팬 모드
    int     fan_pct;        // 환풍팬 세기 %
};
