// 제어판 홈 화면 (800x480). 지금은 코드 안의 가짜 값으로 움직인다.
// 나중에 와이파이로 받은 진짜 값이 g_view 에 들어오면 ui_home_refresh() 만 부르면 된다.
// 이 파일은 LVGL 함수만 쓴다 (Arduino 함수 없음) → PC에서도 같은 코드로 화면을 미리 그려 볼 수 있다.
//
// 화면 색의 약속
//   버튼: "지금 상태"를 글자로 적고, 켜진 것은 색을 채우고 꺼진 것은 회색 테두리로 보여 준다.
//   맨 위 줄: 평소엔 바탕색, 수동 모드는 주황 띠, 물탱크 부족·허브 끊김은 빨간 띠.
#pragma once
#include <lvgl.h>
#include <stdio.h>
#include "farm_model.h"

extern "C" {
LV_FONT_DECLARE(font_kr_20);
LV_FONT_DECLARE(font_num_44);
}

// ---- 색 (밝은 테마) ----
#define C_BG       lv_color_hex(0xF4F3EE)
#define C_CARD     lv_color_hex(0xFFFFFF)
#define C_LINE     lv_color_hex(0xD3D1C7)
#define C_TEXT     lv_color_hex(0x2C2C2A)
#define C_SUB      lv_color_hex(0x5F5E5A)
#define C_TEAL     lv_color_hex(0x1D9E75)
#define C_AMBER    lv_color_hex(0xBA7517)
#define C_AMBER_TX lv_color_hex(0x854F0B)
#define C_RED      lv_color_hex(0xE24B4A)

enum ChipKind : uint8_t { CHIP_OK = 0, CHIP_NORMAL, CHIP_WARN, CHIP_BAD, CHIP_INFO };

struct HomeUi {
    lv_obj_t *topbar;
    lv_obj_t *title_label;
    lv_obj_t *time_label;
    lv_obj_t *hub_dot;
    lv_obj_t *hub_label;
    lv_obj_t *msg_label;     // 안내 문구(눌렀을 때 3초) 또는 맨 위 줄 띠 문구
    lv_obj_t *temp_val;
    lv_obj_t *hum_val;
    lv_obj_t *lux_val;
    lv_obj_t *soil_bar[3];
    lv_obj_t *soil_pct[3];
    lv_obj_t *chip[5];
    lv_obj_t *chip_label[5];
    lv_obj_t *btn[5];
    lv_obj_t *btn_label[5];
};

static FarmView g_view = {
    true,        // hub_online
    235, 55, 8200,
    {41, 39, 44},
    true,        // water_ok
    2, 6,        // pump_today, pump_limit
    false, 0,    // pump_on, pump_left_s
    true,        // mode_auto
    true,        // light_on
    FAN_AUTO, 40
};
static HomeUi g_ui;
static char   g_toast_text[64] = "";
static int    g_toast_left = 0;

// ------------------------------------------------------------------ 만들기 도우미
static lv_obj_t *make_card(lv_obj_t *parent, int x, int y, int w, int h)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(o, C_CARD, 0);
    lv_obj_set_style_border_color(o, C_LINE, 0);
    lv_obj_set_style_border_width(o, 1, 0);
    lv_obj_set_style_radius(o, 8, 0);
    lv_obj_set_style_pad_all(o, 0, 0);
    lv_obj_set_style_shadow_width(o, 0, 0);
    return o;
}

static lv_obj_t *make_label(lv_obj_t *parent, const char *text, int x, int y, lv_color_t color)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, text);
    lv_obj_set_pos(l, x, y);
    lv_obj_set_style_text_color(l, color, 0);
    return l;
}

static void style_chip(int i, ChipKind k)
{
    lv_color_t bg, fg, bd;
    switch (k) {
    case CHIP_OK:   bg = lv_color_hex(0xE1F5EE); fg = lv_color_hex(0x085041); bd = lv_color_hex(0x0F6E56); break;
    case CHIP_WARN: bg = lv_color_hex(0xFAEEDA); fg = lv_color_hex(0x633806); bd = lv_color_hex(0x854F0B); break;
    case CHIP_BAD:  bg = lv_color_hex(0xFCEBEB); fg = lv_color_hex(0x791F1F); bd = lv_color_hex(0xA32D2D); break;
    case CHIP_INFO: bg = lv_color_hex(0xE6F1FB); fg = lv_color_hex(0x0C447C); bd = lv_color_hex(0x185FA5); break;
    default:        bg = lv_color_hex(0xF1EFE8); fg = lv_color_hex(0x444441); bd = lv_color_hex(0xB4B2A9); break;
    }
    lv_obj_set_style_bg_color(g_ui.chip[i], bg, 0);
    lv_obj_set_style_border_color(g_ui.chip[i], bd, 0);
    lv_obj_set_style_text_color(g_ui.chip_label[i], fg, 0);
}

static void style_btn(int i, uint32_t bg, uint32_t border, uint32_t fg)
{
    lv_obj_set_style_bg_color(g_ui.btn[i], lv_color_hex(bg), 0);
    lv_obj_set_style_border_color(g_ui.btn[i], lv_color_hex(border), 0);
    lv_obj_set_style_text_color(g_ui.btn[i], lv_color_hex(fg), 0);
}

// ------------------------------------------------------------------ 버튼 처리
static void toast(const char *msg)
{
    snprintf(g_toast_text, sizeof(g_toast_text), "%s", msg);
    g_toast_left = 3;
}

static int fan_pct_for(FanMode m, int hum)
{
    switch (m) {
    case FAN_LOW:  return 30;
    case FAN_MID:  return 60;
    case FAN_HIGH: return 100;
    case FAN_OFF:  return 0;
    default:       return hum >= 58 ? 80 : 40;   // 자동: 습도가 높으면 세게
    }
}

static void ui_home_refresh(void);

static void on_button(lv_event_t *e)
{
    int id = (int)(intptr_t)lv_event_get_user_data(e);
    switch (id) {
    case 0:  // 물 주기 5초 (진짜 제어판에서는 서버 API로 명령을 보내고, 노드가 거부하면 그 이유를 보여 준다)
        if (g_view.pump_on) {
            toast("이미 급수 중입니다");
        } else if (!g_view.water_ok) {
            toast("물탱크가 비었습니다");
        } else if (g_view.pump_today >= g_view.pump_limit) {
            toast("오늘 급수 횟수를 다 썼습니다");
        } else {
            g_view.pump_today++;
            g_view.pump_on = true;
            g_view.pump_left_s = 5;
            toast("물 주기를 시작했습니다");
        }
        break;
    case 1:  // 급수 정지
        if (g_view.pump_on) toast("급수를 멈췄습니다");
        g_view.pump_on = false;
        g_view.pump_left_s = 0;
        break;
    case 2:  // 조명 (누를 때마다 켜짐 <-> 꺼짐)
        g_view.light_on = !g_view.light_on;
        break;
    case 3:  // 환풍팬: 자동 → 약 → 중 → 강 → 끔
        g_view.fan_mode = (FanMode)((g_view.fan_mode + 1) % 5);
        g_view.fan_pct = fan_pct_for(g_view.fan_mode, g_view.hum);
        break;
    case 4:  // 자동 모드 <-> 수동 모드
        g_view.mode_auto = !g_view.mode_auto;
        break;
    }
    ui_home_refresh();
}

static lv_obj_t *make_button(lv_obj_t *parent, int idx, int id, const char *text)
{
    lv_obj_t *b = lv_btn_create(parent);
    lv_obj_set_pos(b, 16 + 156 * idx, 406);
    lv_obj_set_size(b, 144, 56);
    lv_obj_set_style_border_width(b, 1, 0);
    lv_obj_set_style_radius(b, 8, 0);
    lv_obj_set_style_shadow_width(b, 0, 0);
    lv_obj_set_style_opa(b, LV_OPA_60, LV_STATE_PRESSED);        // 누르는 동안 연해져서 '눌렸다'는 걸 알려 준다
    lv_obj_add_event_cb(b, on_button, LV_EVENT_CLICKED, (void *)(intptr_t)id);
    lv_obj_t *l = lv_label_create(b);
    lv_label_set_text(l, text);
    lv_obj_center(l);
    g_ui.btn[idx] = b;
    g_ui.btn_label[idx] = l;
    return b;
}

// ------------------------------------------------------------------ 화면 갱신
static void ui_home_refresh(void)
{
    char buf[64];

    // ---- 맨 위 줄: 색과 띠 문구 (허브 끊김 > 물탱크 부족 > 수동 모드 순서로 우선)
    uint32_t bar_bg = 0xF4F3EE, bar_fg = 0x2C2C2A;
    const char *banner = "";
    if (!g_view.hub_online) {
        bar_bg = 0xFCEBEB; bar_fg = 0x791F1F; banner = "허브 끊김 · 화분 관리는 계속";
    } else if (!g_view.water_ok) {
        bar_bg = 0xFCEBEB; bar_fg = 0x791F1F; banner = "물탱크가 비었습니다 · 급수 중지";
    } else if (!g_view.mode_auto) {
        bar_bg = 0xFAEEDA; bar_fg = 0x633806; banner = "수동 모드 · 자동 운전 꺼짐";
    }
    lv_color_t fg = lv_color_hex(bar_fg);
    lv_obj_set_style_bg_color(g_ui.topbar, lv_color_hex(bar_bg), 0);
    lv_obj_set_style_text_color(g_ui.title_label, fg, 0);
    lv_obj_set_style_text_color(g_ui.time_label, fg, 0);
    lv_obj_set_style_text_color(g_ui.hub_label, fg, 0);
    lv_obj_set_style_text_color(g_ui.msg_label, (bar_bg == 0xF4F3EE) ? C_AMBER_TX : fg, 0);
    lv_label_set_text(g_ui.msg_label, g_toast_left > 0 ? g_toast_text : banner);

    // 시각 (가짜: 14:32부터 시작해서 흘러감)
    uint32_t mins = 14 * 60 + 32 + lv_tick_get() / 60000;
    snprintf(buf, sizeof(buf), "%02u:%02u", (unsigned)((mins / 60) % 24), (unsigned)(mins % 60));
    lv_label_set_text(g_ui.time_label, buf);

    // 허브 연결
    lv_obj_set_style_bg_color(g_ui.hub_dot, g_view.hub_online ? C_TEAL : C_RED, 0);
    lv_label_set_text(g_ui.hub_label, g_view.hub_online ? "허브 연결됨" : "허브 연결 끊김");

    // ---- 큰 숫자
    snprintf(buf, sizeof(buf), "%d.%d℃", g_view.temp_x10 / 10, g_view.temp_x10 % 10);
    lv_label_set_text(g_ui.temp_val, buf);
    snprintf(buf, sizeof(buf), "%d%%", g_view.hum);
    lv_label_set_text(g_ui.hum_val, buf);
    if (g_view.lux >= 1000) snprintf(buf, sizeof(buf), "%d,%03d lx", g_view.lux / 1000, g_view.lux % 1000);
    else                    snprintf(buf, sizeof(buf), "%d lx", g_view.lux);
    lv_label_set_text(g_ui.lux_val, buf);

    // ---- 화분 수분
    for (int i = 0; i < 3; i++) {
        int v = g_view.soil[i];
        lv_bar_set_value(g_ui.soil_bar[i], v, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(g_ui.soil_bar[i], v < 35 ? C_AMBER : C_TEAL, LV_PART_INDICATOR);
        snprintf(buf, sizeof(buf), "%d%%", v);
        lv_label_set_text(g_ui.soil_pct[i], buf);
    }

    // ---- 상태 칩 5개 (버튼과 같은 색 약속)
    lv_label_set_text(g_ui.chip_label[0], g_view.water_ok ? "물탱크 충분" : "물탱크 부족");
    style_chip(0, g_view.water_ok ? CHIP_OK : CHIP_BAD);

    snprintf(buf, sizeof(buf), "급수 %d/%d회", g_view.pump_today, g_view.pump_limit);
    lv_label_set_text(g_ui.chip_label[1], buf);
    style_chip(1, g_view.pump_today >= g_view.pump_limit ? CHIP_WARN : CHIP_NORMAL);

    lv_label_set_text(g_ui.chip_label[2], g_view.mode_auto ? "모드 자동" : "모드 수동");
    style_chip(2, g_view.mode_auto ? CHIP_NORMAL : CHIP_WARN);

    if (g_view.pump_on) {
        snprintf(buf, sizeof(buf), "급수 중 %d초", g_view.pump_left_s);
        lv_label_set_text(g_ui.chip_label[3], buf);
        style_chip(3, CHIP_INFO);
    } else {
        lv_label_set_text(g_ui.chip_label[3], "펌프 대기");
        style_chip(3, CHIP_NORMAL);
    }

    // 환풍팬 글자: 모드 + 세기
    const char *fan_name = "환풍 자동";
    switch (g_view.fan_mode) {
    case FAN_LOW:  fan_name = "환풍 약"; break;
    case FAN_MID:  fan_name = "환풍 중"; break;
    case FAN_HIGH: fan_name = "환풍 강"; break;
    default: break;
    }
    char fan_text[32];
    if (g_view.fan_mode == FAN_OFF) snprintf(fan_text, sizeof(fan_text), "환풍 꺼짐");
    else                            snprintf(fan_text, sizeof(fan_text), "%s %d%%", fan_name, g_view.fan_pct);
    lv_label_set_text(g_ui.chip_label[4], fan_text);
    style_chip(4, g_view.fan_pct > 0 ? CHIP_OK : CHIP_NORMAL);

    // ---- 버튼 5개: 지금 상태를 글자와 색으로
    // 0 물 주기: 평소 청록 / 급수 중 파랑 / 못 누를 때 회색+이유
    if (g_view.pump_on) {
        snprintf(buf, sizeof(buf), "급수 중 %d초", g_view.pump_left_s);
        lv_label_set_text(g_ui.btn_label[0], buf);
        style_btn(0, 0x185FA5, 0x0C447C, 0xFFFFFF);
    } else if (!g_view.water_ok) {
        lv_label_set_text(g_ui.btn_label[0], "물탱크 부족");
        style_btn(0, 0xF1EFE8, 0xB4B2A9, 0xA32D2D);
    } else if (g_view.pump_today >= g_view.pump_limit) {
        lv_label_set_text(g_ui.btn_label[0], "오늘 한도 끝");
        style_btn(0, 0xF1EFE8, 0xB4B2A9, 0x5F5E5A);
    } else {
        lv_label_set_text(g_ui.btn_label[0], "물 주기 5초");
        style_btn(0, 0x0F6E56, 0x085041, 0xFFFFFF);
    }

    // 1 급수 정지: 급수 중에만 빨강으로 채워지고, 평소엔 흐림
    if (g_view.pump_on) style_btn(1, 0xA32D2D, 0x791F1F, 0xFFFFFF);
    else                style_btn(1, 0xF1EFE8, 0xD3D1C7, 0x888780);

    // 2 조명: 켜짐은 노랑으로 채움, 꺼짐은 회색
    if (g_view.light_on) {
        lv_label_set_text(g_ui.btn_label[2], "조명 켜짐");
        style_btn(2, 0xEF9F27, 0x854F0B, 0x412402);
    } else {
        lv_label_set_text(g_ui.btn_label[2], "조명 꺼짐");
        style_btn(2, 0xF1EFE8, 0xB4B2A9, 0x5F5E5A);
    }

    // 3 환풍팬: 세기가 셀수록 진한 청록, 꺼짐은 회색
    lv_label_set_text(g_ui.btn_label[3], fan_text);
    if (g_view.fan_pct <= 0)       style_btn(3, 0xF1EFE8, 0xB4B2A9, 0x5F5E5A);   // 꺼짐
    else if (g_view.fan_pct <= 35) style_btn(3, 0x9FE1CB, 0x0F6E56, 0x04342C);   // 약
    else if (g_view.fan_pct <= 70) style_btn(3, 0x5DCAA5, 0x0F6E56, 0x04342C);   // 중
    else                           style_btn(3, 0x0F6E56, 0x085041, 0xFFFFFF);   // 강

    // 4 자동 · 수동: 자동은 청록 테두리, 수동은 주황으로 채움
    if (g_view.mode_auto) {
        lv_label_set_text(g_ui.btn_label[4], "자동 모드");
        style_btn(4, 0xE1F5EE, 0x0F6E56, 0x085041);
    } else {
        lv_label_set_text(g_ui.btn_label[4], "수동 모드");
        style_btn(4, 0xFAC775, 0x854F0B, 0x412402);
    }
}

// 1초마다 불린다. 가짜 값을 조금씩 움직여서 화면이 갱신되는 모습을 보여 준다.
static void ui_home_tick(lv_timer_t *t)
{
    (void)t;
    uint32_t sec = lv_tick_get() / 1000;
    uint32_t phase = sec % 120;
    int tri = (int)(phase < 60 ? phase : 120 - phase);       // 0~60 삼각파
    g_view.temp_x10 = 225 + tri / 2;                         // 22.5 ~ 25.5 ℃
    g_view.hum = 50 + tri / 6;                               // 50 ~ 60 %

    if (g_view.pump_on) {
        for (int i = 0; i < 3; i++) if (g_view.soil[i] < 100) g_view.soil[i] += 2;
        if (--g_view.pump_left_s <= 0) { g_view.pump_on = false; g_view.pump_left_s = 0; }
    } else if (sec % 10 == 0) {
        for (int i = 0; i < 3; i++) if (g_view.soil[i] > 0) g_view.soil[i] -= 1;
    }
    if (g_view.fan_mode == FAN_AUTO) g_view.fan_pct = fan_pct_for(FAN_AUTO, g_view.hum);
    if (g_toast_left > 0) g_toast_left--;

    ui_home_refresh();
}

// ------------------------------------------------------------------ 홈 화면 만들기
static void ui_home_create(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, C_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_text_font(scr, &font_kr_20, 0);      // 모든 글씨에 한글 글꼴
    lv_obj_set_style_text_color(scr, C_TEXT, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // 맨 위 줄 (바탕색이 상태에 따라 바뀐다)
    g_ui.topbar = lv_obj_create(scr);
    lv_obj_set_pos(g_ui.topbar, 0, 0);
    lv_obj_set_size(g_ui.topbar, 800, 56);
    lv_obj_clear_flag(g_ui.topbar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(g_ui.topbar, C_BG, 0);
    lv_obj_set_style_border_width(g_ui.topbar, 0, 0);
    lv_obj_set_style_radius(g_ui.topbar, 0, 0);
    lv_obj_set_style_pad_all(g_ui.topbar, 0, 0);
    lv_obj_set_style_shadow_width(g_ui.topbar, 0, 0);

    g_ui.title_label = make_label(scr, "스마트팜 · 선반 1", 16, 14, C_TEXT);

    g_ui.msg_label = make_label(scr, "", 190, 14, C_AMBER_TX);
    lv_obj_set_width(g_ui.msg_label, 330);
    lv_label_set_long_mode(g_ui.msg_label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(g_ui.msg_label, LV_TEXT_ALIGN_CENTER, 0);

    g_ui.time_label = make_label(scr, "14:32", 530, 14, C_SUB);
    lv_obj_set_width(g_ui.time_label, 70);
    lv_obj_set_style_text_align(g_ui.time_label, LV_TEXT_ALIGN_RIGHT, 0);

    g_ui.hub_dot = lv_obj_create(scr);
    lv_obj_set_pos(g_ui.hub_dot, 620, 22);
    lv_obj_set_size(g_ui.hub_dot, 12, 12);
    lv_obj_set_style_radius(g_ui.hub_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(g_ui.hub_dot, 0, 0);
    lv_obj_clear_flag(g_ui.hub_dot, LV_OBJ_FLAG_SCROLLABLE);

    g_ui.hub_label = make_label(scr, "허브 연결됨", 640, 14, C_TEXT);
    lv_obj_set_width(g_ui.hub_label, 144);

    lv_obj_t *line = lv_obj_create(scr);
    lv_obj_set_pos(line, 0, 56);
    lv_obj_set_size(line, 800, 1);
    lv_obj_set_style_bg_color(line, C_LINE, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_radius(line, 0, 0);

    // 큰 숫자 카드 3개
    const int cx[3] = {16, 277, 538};
    const char *names[3] = {"온도", "습도", "조도"};
    lv_obj_t **vals[3] = {&g_ui.temp_val, &g_ui.hum_val, &g_ui.lux_val};
    for (int i = 0; i < 3; i++) {
        lv_obj_t *c = make_card(scr, cx[i], 68, 245, 100);
        make_label(c, names[i], 16, 8, C_SUB);
        *vals[i] = make_label(c, "-", 16, 38, C_TEXT);
        lv_obj_set_style_text_font(*vals[i], &font_num_44, 0);
    }

    // 화분 수분 카드
    lv_obj_t *soil = make_card(scr, 16, 182, 768, 152);
    make_label(soil, "화분 수분", 16, 8, C_SUB);
    lv_obj_t *note = make_label(soil, "주황 선 = 급수 시작 35%", 452, 8, C_SUB);
    lv_obj_set_width(note, 300);
    lv_obj_set_style_text_align(note, LV_TEXT_ALIGN_RIGHT, 0);
    for (int i = 0; i < 3; i++) {
        int y = 46 + 34 * i;
        char nm[16];
        snprintf(nm, sizeof(nm), "화분 %d", i + 1);
        make_label(soil, nm, 16, y - 3, C_TEXT);
        lv_obj_t *bar = lv_bar_create(soil);
        lv_obj_set_pos(bar, 104, y);
        lv_obj_set_size(bar, 520, 20);
        lv_bar_set_range(bar, 0, 100);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0xDEDCD3), LV_PART_MAIN);
        lv_obj_set_style_radius(bar, 4, LV_PART_MAIN);
        lv_obj_set_style_radius(bar, 4, LV_PART_INDICATOR);
        g_ui.soil_bar[i] = bar;
        g_ui.soil_pct[i] = make_label(soil, "-", 644, y - 3, C_TEXT);
    }
    lv_obj_t *mark = lv_obj_create(soil);                 // 급수 시작선 (35%)
    lv_obj_set_pos(mark, 104 + 520 * 35 / 100 - 1, 40);
    lv_obj_set_size(mark, 3, 100);
    lv_obj_set_style_bg_color(mark, C_AMBER, 0);
    lv_obj_set_style_border_width(mark, 0, 0);
    lv_obj_set_style_radius(mark, 0, 0);

    // 상태 칩 5개
    for (int i = 0; i < 5; i++) {
        lv_obj_t *c = make_card(scr, 16 + 156 * i, 346, 144, 44);
        g_ui.chip[i] = c;
        lv_obj_t *l = lv_label_create(c);
        lv_label_set_text(l, "-");
        lv_label_set_long_mode(l, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(l, 136);
        lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(l, LV_ALIGN_CENTER, 0, 0);
        g_ui.chip_label[i] = l;
    }

    // 버튼 5개 (색과 글자는 ui_home_refresh 가 상태에 맞게 정한다)
    make_button(scr, 0, 0, "물 주기 5초");
    make_button(scr, 1, 1, "급수 정지");
    make_button(scr, 2, 2, "조명 켜짐");
    make_button(scr, 3, 3, "환풍팬");
    make_button(scr, 4, 4, "자동 모드");

    g_view.fan_pct = fan_pct_for(g_view.fan_mode, g_view.hum);
    ui_home_refresh();
    lv_timer_create(ui_home_tick, 1000, NULL);
}
