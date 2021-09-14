#pragma once
#include <esp_log.h>
#include <epd_driver.h>
#include <epd_highlevel.h>
#include "firasans.h"
#include "Renderer.h"

class EpdRenderer : public Renderer
{
private:
  const EpdFont *m_font;
  EpdiyHighlevelState m_hl;
  uint8_t *m_frame_buffer;
  EpdFontProperties m_font_props;
  // temp buffer for measuring and rendering text
  char buffer[100];

  void get_text(const char *src, int start_index, int end_index)
  {
    // make a copy of the string from the src to the temp buffer
    // TODO handle html entities in the string
    int index = 0;
    for (int i = start_index; i < end_index && index < 99; i++)
    {
      buffer[index++] = src[i];
    }
    buffer[index] = 0;
  }

public:
  EpdRenderer()
  {
    m_font_props = epd_font_properties_default();
    // start up the EPD
    epd_init(EPD_OPTIONS_DEFAULT);
    m_hl = epd_hl_init(EPD_BUILTIN_WAVEFORM);
    // first set full screen to white
    epd_hl_set_all_white(&m_hl);
    epd_set_rotation(EPD_ROT_INVERTED_PORTRAIT);
    m_frame_buffer = epd_hl_get_framebuffer(&m_hl);
    m_font = &FiraSans;
    epd_clear();
  }
  ~EpdRenderer() {}
  int get_text_width(const char *src, int start_index, int end_index, bool italic = false, bool bold = false)
  {
    get_text(src, start_index, end_index);
    int x = 0, y = 0, x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    epd_get_text_bounds(m_font, buffer, &x, &y, &x1, &y1, &x2, &y2, &m_font_props);
    return x2 - x1;
  }
  void draw_text(int x, int y, const char *src, int start_index, int end_index, bool italic = false, bool bold = false)
  {
    int ypos = y + this->get_line_height();
    get_text(src, start_index, end_index);
    epd_write_string(m_font, buffer, &x, &ypos, m_frame_buffer, &m_font_props);
  }
  void draw_rect(int x, int y, int width, int height)
  {
    epd_draw_rect({.x = x, .y = y, .width = width, .height = height}, 0, m_frame_buffer);
  }

  void clear_display()
  {
    epd_hl_set_all_white(&m_hl);
  }
  void flush_display()
  {
    ESP_LOGI("EPD", "Flushing display");
    epd_poweron();
    // ESP_LOGI(TAG, "epd_ambient_temperature=%f", epd_ambient_temperature());
    epd_hl_update_screen(&m_hl, MODE_GC16, 20);
    vTaskDelay(500);
    epd_poweroff();
  }
  virtual int get_page_width()
  {
    // TODO: get this from the EPD driver
    return 540;
  }
  virtual int get_page_height()
  {
    // TODO: get this from the EPD driver
    return 960;
  }
  virtual int get_space_width()
  {
    // TODO: - look up the space character width
    return 11;
  }
  virtual int get_line_height()
  {
    return m_font->advance_y;
  }
};