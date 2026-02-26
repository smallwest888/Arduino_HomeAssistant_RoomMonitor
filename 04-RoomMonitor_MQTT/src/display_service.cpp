#include "display_service.h"

#include <math.h>
#include <stdio.h>

#include "carrier_platform.h"
#include "config.h"

namespace {
struct DashboardView {
  const char* title;
  const char* unit;
  float value;
  float min_value;
  float max_value;
  uint8_t decimals;
  uint16_t needle_color;
};

struct DisplayLayout {
  int16_t cx;
  int16_t cy;
  int16_t radius;
  int16_t panel_w;
  int16_t panel_h;
  int16_t panel_x;
  int16_t panel_y;
  int16_t left_col_x;
  int16_t right_col_x;
};

float ClampFloat(const float value, const float min_value, const float max_value) {
  if (value < min_value) {
    return min_value;
  }
  if (value > max_value) {
    return max_value;
  }
  return value;
}

float MapFloat(
    const float value,
    const float from_min,
    const float from_max,
    const float to_min,
    const float to_max) {
  const float scale = (value - from_min) / (from_max - from_min);
  return to_min + (scale * (to_max - to_min));
}

int16_t GetDisplayRadius(MKRIoTCarrier* carrier) {
  const int16_t width = carrier->display.width();
  const int16_t height = carrier->display.height();
  const int16_t min_side = (width < height) ? width : height;
  return (min_side / 2) - RoomMonitorConfig::DISPLAY_OUTER_MARGIN;
}

void SetDefaultTextStyle() {
  MKRIoTCarrier* carrier = CarrierPlatform_Get();
  if (carrier == nullptr) {
    return;
  }

  carrier->display.fillScreen(RoomMonitorConfig::DISPLAY_COLOR_BLACK);
  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_CYAN);
  carrier->display.setTextSize(RoomMonitorConfig::DISPLAY_TEXT_SIZE_SMALL);
}

void DrawDashboardTicks(MKRIoTCarrier* carrier, const int16_t cx, const int16_t cy, const int16_t radius) {
  for (uint8_t i = 0U; i <= RoomMonitorConfig::DISPLAY_GAUGE_TICK_COUNT; i++) {
    const float angle_deg = MapFloat(
        static_cast<float>(i),
        0.0F,
        static_cast<float>(RoomMonitorConfig::DISPLAY_GAUGE_TICK_COUNT),
        RoomMonitorConfig::DISPLAY_GAUGE_START_DEG,
        RoomMonitorConfig::DISPLAY_GAUGE_END_DEG);
    const float angle_rad = angle_deg * RoomMonitorConfig::DISPLAY_DEG_TO_RAD;

    const int16_t x1 = static_cast<int16_t>(
        static_cast<float>(cx) + cosf(angle_rad) * static_cast<float>(radius - RoomMonitorConfig::DISPLAY_GAUGE_TICK_INNER_OFFSET));
    const int16_t y1 = static_cast<int16_t>(
        static_cast<float>(cy) + sinf(angle_rad) * static_cast<float>(radius - RoomMonitorConfig::DISPLAY_GAUGE_TICK_INNER_OFFSET));
    const int16_t x2 = static_cast<int16_t>(
        static_cast<float>(cx) + cosf(angle_rad) * static_cast<float>(radius - RoomMonitorConfig::DISPLAY_GAUGE_TICK_OUTER_OFFSET));
    const int16_t y2 = static_cast<int16_t>(
        static_cast<float>(cy) + sinf(angle_rad) * static_cast<float>(radius - RoomMonitorConfig::DISPLAY_GAUGE_TICK_OUTER_OFFSET));

    carrier->display.drawLine(x1, y1, x2, y2, RoomMonitorConfig::DISPLAY_COLOR_GRAY);
  }
}

void DrawDashboardNeedle(
    MKRIoTCarrier* carrier,
    const int16_t cx,
    const int16_t cy,
    const int16_t radius,
    const float value,
    const float min_value,
    const float max_value,
    const uint16_t needle_color) {
  const float bounded_value = ClampFloat(value, min_value, max_value);
  const float angle_deg = MapFloat(
      bounded_value,
      min_value,
      max_value,
      RoomMonitorConfig::DISPLAY_GAUGE_START_DEG,
      RoomMonitorConfig::DISPLAY_GAUGE_END_DEG);
  const float angle_rad = angle_deg * RoomMonitorConfig::DISPLAY_DEG_TO_RAD;

  const int16_t x = static_cast<int16_t>(
      static_cast<float>(cx) + cosf(angle_rad) * static_cast<float>(radius - RoomMonitorConfig::DISPLAY_GAUGE_NEEDLE_OFFSET));
  const int16_t y = static_cast<int16_t>(
      static_cast<float>(cy) + sinf(angle_rad) * static_cast<float>(radius - RoomMonitorConfig::DISPLAY_GAUGE_NEEDLE_OFFSET));

  carrier->display.drawLine(cx, cy, x, y, needle_color);
  carrier->display.fillCircle(cx, cy, RoomMonitorConfig::DISPLAY_NEEDLE_CENTER_RADIUS, needle_color);
}

DisplayLayout BuildLayout(MKRIoTCarrier* carrier) {
  DisplayLayout layout = {};
  layout.cx = carrier->display.width() / 2;
  layout.cy = carrier->display.height() / 2;
  layout.radius = GetDisplayRadius(carrier);
  layout.panel_w = carrier->display.width() - RoomMonitorConfig::DISPLAY_PANEL_WIDTH_REDUCTION;
  layout.panel_h = RoomMonitorConfig::DISPLAY_PANEL_HEIGHT;
  layout.panel_x = RoomMonitorConfig::DISPLAY_PANEL_X;
  layout.panel_y = carrier->display.height() - layout.panel_h - RoomMonitorConfig::DISPLAY_PANEL_BOTTOM_MARGIN;
  layout.left_col_x = layout.panel_x + RoomMonitorConfig::DISPLAY_PANEL_LEFT_COL_OFFSET;
  layout.right_col_x = layout.panel_x + (layout.panel_w / 2) + RoomMonitorConfig::DISPLAY_PANEL_RIGHT_COL_OFFSET;
  return layout;
}

void DrawDashboardFrame(MKRIoTCarrier* carrier, const DisplayLayout& layout) {
  carrier->display.drawCircle(layout.cx, layout.cy, layout.radius, RoomMonitorConfig::DISPLAY_COLOR_CYAN);
  carrier->display.drawCircle(
      layout.cx,
      layout.cy,
      layout.radius - RoomMonitorConfig::DISPLAY_GAUGE_RING_OFFSET,
      RoomMonitorConfig::DISPLAY_COLOR_BLUE);
  DrawDashboardTicks(carrier, layout.cx, layout.cy, layout.radius);
}

void DrawHeartbeatIndicator(MKRIoTCarrier* carrier, const bool heartbeat_on) {
  carrier->display.fillCircle(
      carrier->display.width() - RoomMonitorConfig::DISPLAY_HEARTBEAT_OFFSET_X,
      RoomMonitorConfig::DISPLAY_HEARTBEAT_OFFSET_Y,
      RoomMonitorConfig::DISPLAY_HEARTBEAT_RADIUS,
      heartbeat_on ? RoomMonitorConfig::DISPLAY_COLOR_GREEN : RoomMonitorConfig::DISPLAY_COLOR_GRAY);
}

void DrawGaugeReadout(MKRIoTCarrier* carrier, const DisplayLayout& layout, const DashboardView& gauge) {
  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_GRAY);
  carrier->display.setTextSize(1);
  carrier->display.setCursor(
      layout.cx - RoomMonitorConfig::DISPLAY_GAUGE_TITLE_OFFSET_X,
      layout.cy - RoomMonitorConfig::DISPLAY_GAUGE_TITLE_OFFSET_Y);
  carrier->display.print(gauge.title);

  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_WHITE);
  carrier->display.setTextSize(3);
  carrier->display.setCursor(
      layout.cx - RoomMonitorConfig::DISPLAY_GAUGE_VALUE_OFFSET_X,
      layout.cy - RoomMonitorConfig::DISPLAY_GAUGE_VALUE_OFFSET_Y);
  carrier->display.print(gauge.value, gauge.decimals);

  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_CYAN);
  carrier->display.setTextSize(1);
  carrier->display.setCursor(
      layout.cx + RoomMonitorConfig::DISPLAY_GAUGE_UNIT_OFFSET_X,
      layout.cy - RoomMonitorConfig::DISPLAY_GAUGE_UNIT_OFFSET_Y);
  carrier->display.print(gauge.unit);

  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_GRAY);
  carrier->display.setTextSize(1);
  carrier->display.setCursor(
      layout.cx - RoomMonitorConfig::DISPLAY_GAUGE_MINLABEL_OFFSET_X,
      layout.cy + RoomMonitorConfig::DISPLAY_GAUGE_MINMAX_OFFSET_Y);
  carrier->display.print(gauge.min_value, 0);
  carrier->display.print(gauge.unit);
  carrier->display.setCursor(
      layout.cx + RoomMonitorConfig::DISPLAY_GAUGE_MAXLABEL_OFFSET_X,
      layout.cy + RoomMonitorConfig::DISPLAY_GAUGE_MINMAX_OFFSET_Y);
  carrier->display.print(gauge.max_value, 0);
  carrier->display.print(gauge.unit);
}

void DrawBottomPanel(MKRIoTCarrier* carrier, const DisplayLayout& layout, const SensorData* data) {
  carrier->display.fillRoundRect(
      layout.panel_x,
      layout.panel_y,
      layout.panel_w,
      layout.panel_h,
      RoomMonitorConfig::DISPLAY_PANEL_CORNER_RADIUS,
      RoomMonitorConfig::DISPLAY_COLOR_BLACK);
  carrier->display.drawRoundRect(
      layout.panel_x,
      layout.panel_y,
      layout.panel_w,
      layout.panel_h,
      RoomMonitorConfig::DISPLAY_PANEL_CORNER_RADIUS,
      RoomMonitorConfig::DISPLAY_COLOR_CYAN);

  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_WHITE);
  carrier->display.setTextSize(1);
  carrier->display.setCursor(layout.left_col_x, layout.panel_y + RoomMonitorConfig::DISPLAY_PANEL_ROW1_OFFSET_Y);
  carrier->display.print("H:");
  carrier->display.print(data->humidity_pct, 0);
  carrier->display.print("%");

  carrier->display.setCursor(layout.right_col_x, layout.panel_y + RoomMonitorConfig::DISPLAY_PANEL_ROW1_OFFSET_Y);
  carrier->display.print("P:");
  carrier->display.print(data->pressure_hpa, 0);
  carrier->display.print("hPa");

  carrier->display.setCursor(layout.left_col_x, layout.panel_y + RoomMonitorConfig::DISPLAY_PANEL_ROW2_OFFSET_Y);
  carrier->display.print("S1:");
  carrier->display.print(data->soil1_pct);
  carrier->display.print("%");

  carrier->display.setCursor(layout.right_col_x, layout.panel_y + RoomMonitorConfig::DISPLAY_PANEL_ROW2_OFFSET_Y);
  carrier->display.print("S2:");
  carrier->display.print(data->soil2_pct);
  carrier->display.print("%");
}

DashboardView BuildDashboardView(const SensorData* data) {
  const uint32_t mode = (millis() / RoomMonitorConfig::DISPLAY_GAUGE_MODE_SWITCH_MS) % 3UL;
  if (mode == 0UL) {
    DashboardView view = {
        "TEMP",
        "C",
        data->temperature_c,
        RoomMonitorConfig::DISPLAY_TEMP_MIN_C,
        RoomMonitorConfig::DISPLAY_TEMP_MAX_C,
        RoomMonitorConfig::FLOAT_DECIMALS,
        RoomMonitorConfig::DISPLAY_COLOR_ORANGE};
    return view;
  }
  if (mode == 1UL) {
    const float soil_avg = (static_cast<float>(data->soil1_pct) + static_cast<float>(data->soil2_pct)) / 2.0F;
    DashboardView view = {
        "SOIL",
        "%",
        soil_avg,
        RoomMonitorConfig::DISPLAY_SOIL_MIN_PCT,
        RoomMonitorConfig::DISPLAY_SOIL_MAX_PCT,
        0U,
        RoomMonitorConfig::DISPLAY_COLOR_GREEN};
    return view;
  }

  DashboardView view = {
      "PRESS",
      "hPa",
      data->pressure_hpa,
      RoomMonitorConfig::DISPLAY_PRESSURE_MIN_HPA,
      RoomMonitorConfig::DISPLAY_PRESSURE_MAX_HPA,
      0U,
      RoomMonitorConfig::DISPLAY_COLOR_CYAN};
  return view;
}
}

void DisplayService_Init() {
  SetDefaultTextStyle();
}

void DisplayService_ShowBootText() {
  MKRIoTCarrier* carrier = CarrierPlatform_Get();
  if (carrier == nullptr) {
    return;
  }

  carrier->display.fillScreen(RoomMonitorConfig::DISPLAY_COLOR_WHITE);
  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_RED);
  carrier->display.setTextSize(RoomMonitorConfig::DISPLAY_TEXT_SIZE_LARGE);
  carrier->display.setCursor(RoomMonitorConfig::DISPLAY_BOOT_X, RoomMonitorConfig::DISPLAY_BOOT_Y);
  carrier->display.print("Hello");
}

void DisplayService_ShowData(const SensorData* data) {
  if (data == nullptr) {
    return;
  }

  MKRIoTCarrier* carrier = CarrierPlatform_Get();
  if (carrier == nullptr) {
    return;
  }

  const DisplayLayout layout = BuildLayout(carrier);
  const DashboardView gauge = BuildDashboardView(data);
  const bool heartbeat_on = ((millis() / RoomMonitorConfig::DISPLAY_HEARTBEAT_BLINK_MS) % 2UL) == 0UL;

  carrier->display.fillScreen(RoomMonitorConfig::DISPLAY_COLOR_BLACK);
  DrawDashboardFrame(carrier, layout);
  DrawDashboardNeedle(
      carrier,
      layout.cx,
      layout.cy,
      layout.radius,
      gauge.value,
      gauge.min_value,
      gauge.max_value,
      gauge.needle_color);
  DrawHeartbeatIndicator(carrier, heartbeat_on);
  DrawGaugeReadout(carrier, layout, gauge);
  DrawBottomPanel(carrier, layout, data);
}
