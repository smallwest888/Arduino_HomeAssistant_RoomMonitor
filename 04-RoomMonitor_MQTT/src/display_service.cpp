#include "display_service.h"

#include <math.h>
#include <stdio.h>

#include "carrier_platform.h"
#include "config.h"

namespace {
constexpr uint32_t GAUGE_MODE_SWITCH_MS = 1000UL;
constexpr float SOIL_GAUGE_MIN = 0.0F;
constexpr float SOIL_GAUGE_MAX = 100.0F;
constexpr float PRESSURE_GAUGE_MIN = 950.0F;
constexpr float PRESSURE_GAUGE_MAX = 1050.0F;

struct GaugeView {
  const char* title;
  const char* unit;
  float value;
  float min_value;
  float max_value;
  uint8_t decimals;
  uint16_t needle_color;
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

void DrawGaugeTicks(MKRIoTCarrier* carrier, const int16_t cx, const int16_t cy, const int16_t radius) {
  for (uint8_t i = 0U; i <= 10U; i++) {
    const float angle_deg = MapFloat(
        static_cast<float>(i),
        0.0F,
        10.0F,
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

void DrawGaugeNeedle(
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
  carrier->display.fillCircle(cx, cy, 5, needle_color);
}

GaugeView BuildGaugeView(const SensorData* data) {
  const uint32_t mode = (millis() / GAUGE_MODE_SWITCH_MS) % 3UL;
  if (mode == 0UL) {
    GaugeView view = {
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
    GaugeView view = {
        "SOIL",
        "%",
        soil_avg,
        SOIL_GAUGE_MIN,
        SOIL_GAUGE_MAX,
        0U,
        RoomMonitorConfig::DISPLAY_COLOR_GREEN};
    return view;
  }

  GaugeView view = {
      "PRESS",
      "hPa",
      data->pressure_hpa,
      PRESSURE_GAUGE_MIN,
      PRESSURE_GAUGE_MAX,
      0U,
      RoomMonitorConfig::DISPLAY_COLOR_CYAN};
  return view;
}
}  // namespace

void DisplayService_Init() {
  SetDefaultTextStyle();
}

void DisplayService_ShowBootText() {
  MKRIoTCarrier* carrier = CarrierPlatform_Get();
  if (carrier == nullptr) {
    return;
  }

  carrier->display.fillScreen(RoomMonitorConfig::DISPLAY_COLOR_BLACK);
  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_CYAN);
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

  const int16_t cx = carrier->display.width() / 2;
  const int16_t cy = carrier->display.height() / 2;
  const int16_t radius = GetDisplayRadius(carrier);
  const GaugeView gauge = BuildGaugeView(data);
  const bool heartbeat_on = ((millis() / 500UL) % 2UL) == 0UL;

  carrier->display.fillScreen(RoomMonitorConfig::DISPLAY_COLOR_BLACK);
  carrier->display.drawCircle(cx, cy, radius, RoomMonitorConfig::DISPLAY_COLOR_CYAN);
  carrier->display.drawCircle(
      cx,
      cy,
      radius - RoomMonitorConfig::DISPLAY_GAUGE_RING_OFFSET,
      RoomMonitorConfig::DISPLAY_COLOR_BLUE);
  DrawGaugeTicks(carrier, cx, cy, radius);
  DrawGaugeNeedle(
      carrier,
      cx,
      cy,
      radius,
      gauge.value,
      gauge.min_value,
      gauge.max_value,
      gauge.needle_color);
  carrier->display.fillCircle(
      carrier->display.width() - 12,
      12,
      4,
      heartbeat_on ? RoomMonitorConfig::DISPLAY_COLOR_GREEN : RoomMonitorConfig::DISPLAY_COLOR_GRAY);

  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_GRAY);
  carrier->display.setTextSize(1);
  carrier->display.setCursor(cx - 16, cy - 38);
  carrier->display.print(gauge.title);

  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_WHITE);
  carrier->display.setTextSize(3);
  carrier->display.setCursor(cx - 50, cy - 18);
  carrier->display.print(gauge.value, gauge.decimals);

  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_CYAN);
  carrier->display.setTextSize(1);
  carrier->display.setCursor(cx + 36, cy - 4);
  carrier->display.print(gauge.unit);

  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_GRAY);
  carrier->display.setTextSize(1);
  carrier->display.setCursor(cx - 42, cy + 8);
  carrier->display.print(gauge.min_value, 0);
  carrier->display.print(gauge.unit);
  carrier->display.setCursor(cx + 20, cy + 8);
  carrier->display.print(gauge.max_value, 0);
  carrier->display.print(gauge.unit);

  const int16_t panel_w = carrier->display.width() - 52;
  const int16_t panel_h = 44;
  const int16_t panel_x = 26;
  const int16_t panel_y = carrier->display.height() - panel_h - 10;
  const int16_t left_col_x = panel_x + 14;
  const int16_t right_col_x = panel_x + (panel_w / 2) + 4;

  carrier->display.fillRoundRect(
      panel_x,
      panel_y,
      panel_w,
      panel_h,
      8,
      RoomMonitorConfig::DISPLAY_COLOR_BLACK);
  carrier->display.drawRoundRect(
      panel_x,
      panel_y,
      panel_w,
      panel_h,
      8,
      RoomMonitorConfig::DISPLAY_COLOR_CYAN);

  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_WHITE);
  carrier->display.setTextSize(1);
  carrier->display.setCursor(left_col_x, panel_y + 9);
  carrier->display.print("H:");
  carrier->display.print(data->humidity_pct, 0);
  carrier->display.print("%");

  carrier->display.setCursor(right_col_x, panel_y + 9);
  carrier->display.print("P:");
  carrier->display.print(data->pressure_hpa, 0);
  carrier->display.print("hPa");

  carrier->display.setCursor(left_col_x, panel_y + 25);
  carrier->display.print("S1:");
  carrier->display.print(data->soil1_pct);
  carrier->display.print("%");

  carrier->display.setCursor(right_col_x, panel_y + 25);
  carrier->display.print("S2:");
  carrier->display.print(data->soil2_pct);
  carrier->display.print("%");
}
