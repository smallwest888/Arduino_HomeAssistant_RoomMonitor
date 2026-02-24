#include "display_service.h"

#include <math.h>

#include "carrier_platform.h"
#include "config.h"

namespace {
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

void DrawTempNeedle(MKRIoTCarrier* carrier, const int16_t cx, const int16_t cy, const int16_t radius, const float temp_c) {
  const float bounded_temp = ClampFloat(
      temp_c,
      RoomMonitorConfig::DISPLAY_TEMP_MIN_C,
      RoomMonitorConfig::DISPLAY_TEMP_MAX_C);
  const float angle_deg = MapFloat(
      bounded_temp,
      RoomMonitorConfig::DISPLAY_TEMP_MIN_C,
      RoomMonitorConfig::DISPLAY_TEMP_MAX_C,
      RoomMonitorConfig::DISPLAY_GAUGE_START_DEG,
      RoomMonitorConfig::DISPLAY_GAUGE_END_DEG);
  const float angle_rad = angle_deg * RoomMonitorConfig::DISPLAY_DEG_TO_RAD;

  const int16_t x = static_cast<int16_t>(
      static_cast<float>(cx) + cosf(angle_rad) * static_cast<float>(radius - RoomMonitorConfig::DISPLAY_GAUGE_NEEDLE_OFFSET));
  const int16_t y = static_cast<int16_t>(
      static_cast<float>(cy) + sinf(angle_rad) * static_cast<float>(radius - RoomMonitorConfig::DISPLAY_GAUGE_NEEDLE_OFFSET));

  carrier->display.drawLine(cx, cy, x, y, RoomMonitorConfig::DISPLAY_COLOR_ORANGE);
  carrier->display.fillCircle(cx, cy, 5, RoomMonitorConfig::DISPLAY_COLOR_ORANGE);
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

  carrier->display.fillScreen(RoomMonitorConfig::DISPLAY_COLOR_BLACK);
  carrier->display.drawCircle(cx, cy, radius, RoomMonitorConfig::DISPLAY_COLOR_CYAN);
  carrier->display.drawCircle(
      cx,
      cy,
      radius - RoomMonitorConfig::DISPLAY_GAUGE_RING_OFFSET,
      RoomMonitorConfig::DISPLAY_COLOR_BLUE);
  DrawGaugeTicks(carrier, cx, cy, radius);
  DrawTempNeedle(carrier, cx, cy, radius, data->temperature_c);

  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_WHITE);
  carrier->display.setTextSize(3);
  carrier->display.setCursor(cx - 40, cy - 24);
  carrier->display.print(data->temperature_c, RoomMonitorConfig::FLOAT_DECIMALS);
  carrier->display.print("C");

  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_GRAY);
  carrier->display.setTextSize(1);
  carrier->display.setCursor(cx - 42, cy + 8);
  carrier->display.print(RoomMonitorConfig::DISPLAY_TEMP_MIN_C, 0);
  carrier->display.print("C");
  carrier->display.setCursor(cx + 20, cy + 8);
  carrier->display.print(RoomMonitorConfig::DISPLAY_TEMP_MAX_C, 0);
  carrier->display.print("C");

  carrier->display.fillRoundRect(
      RoomMonitorConfig::DISPLAY_INFO_PANEL_X,
      RoomMonitorConfig::DISPLAY_INFO_PANEL_Y,
      RoomMonitorConfig::DISPLAY_INFO_PANEL_W,
      RoomMonitorConfig::DISPLAY_INFO_PANEL_H,
      8,
      RoomMonitorConfig::DISPLAY_COLOR_BLUE);
  carrier->display.drawRoundRect(
      RoomMonitorConfig::DISPLAY_INFO_PANEL_X,
      RoomMonitorConfig::DISPLAY_INFO_PANEL_Y,
      RoomMonitorConfig::DISPLAY_INFO_PANEL_W,
      RoomMonitorConfig::DISPLAY_INFO_PANEL_H,
      8,
      RoomMonitorConfig::DISPLAY_COLOR_CYAN);

  carrier->display.setTextColor(RoomMonitorConfig::DISPLAY_COLOR_WHITE);
  carrier->display.setTextSize(1);
  carrier->display.setCursor(18, RoomMonitorConfig::DISPLAY_INFO_ROW_Y);
  carrier->display.print("H ");
  carrier->display.print(data->humidity_pct, 0);
  carrier->display.print("%");

  carrier->display.setCursor(68, RoomMonitorConfig::DISPLAY_INFO_ROW_Y);
  carrier->display.print("P ");
  carrier->display.print(data->pressure_hpa, 0);

  carrier->display.setCursor(140, RoomMonitorConfig::DISPLAY_INFO_ROW_Y);
  carrier->display.print("S1 ");
  carrier->display.print(data->soil1_pct);
  carrier->display.print("%");

  carrier->display.setCursor(190, RoomMonitorConfig::DISPLAY_INFO_ROW_Y);
  carrier->display.print("S2 ");
  carrier->display.print(data->soil2_pct);
  carrier->display.print("%");
}
