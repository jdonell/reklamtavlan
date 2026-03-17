#include "display.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
 
// ── I2C / LCD konfiguration ─────────────────────────────────────────────────
#define I2C_MASTER_NUM      I2C_NUM_0
#define I2C_MASTER_SDA_IO   21
#define I2C_MASTER_SCL_IO   22
#define I2C_MASTER_FREQ_HZ  100000
#define LCD_ADDR            0x27
 
// PCF8574 bitmaskar
#define LCD_BACKLIGHT   0x08
#define LCD_ENABLE      0x04
#define LCD_RW          0x02
#define LCD_RS          0x01
 
// LCD-kommandon
#define LCD_CLEAR           0x01
#define LCD_HOME            0x02
#define LCD_ENTRY_MODE      0x06
#define LCD_DISPLAY_ON      0x0C
#define LCD_DISPLAY_OFF     0x08
#define LCD_FUNCTION_SET    0x28   // 4-bit, 2 rader, 5x8
#define LCD_SET_DDRAM       0x80
 
#define LCD_COLS 16
#define LCD_ROWS 2
 
// ── Lågnivå I2C-hjälpfunktioner ─────────────────────────────────────────────
 
static void i2c_write_byte(uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LCD_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(10));
    i2c_cmd_link_delete(cmd);
}
 
static void lcd_pulse_enable(uint8_t data)
{
    i2c_write_byte(data | LCD_ENABLE | LCD_BACKLIGHT);
    vTaskDelay(pdMS_TO_TICKS(1));
    i2c_write_byte((data & ~LCD_ENABLE) | LCD_BACKLIGHT);
    vTaskDelay(pdMS_TO_TICKS(1));
}
 
static void lcd_write_nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t data = (nibble & 0xF0) | rs | LCD_BACKLIGHT;
    lcd_pulse_enable(data);
}
 
static void lcd_send_byte(uint8_t byte, uint8_t rs)
{
    lcd_write_nibble(byte & 0xF0, rs);
    lcd_write_nibble((byte << 4) & 0xF0, rs);
}
 
static void lcd_command(uint8_t cmd)
{
    lcd_send_byte(cmd, 0x00);   // RS = 0 → kommando
    vTaskDelay(pdMS_TO_TICKS(2));
}
 
static void lcd_char(char c)
{
    lcd_send_byte((uint8_t)c, LCD_RS);   // RS = 1 → data
}
 
// ── Publik hjälp ─────────────────────────────────────────────────────────────
 
static void lcd_set_cursor(uint8_t col, uint8_t row)
{
    uint8_t offsets[] = {0x00, 0x40};
    lcd_command(LCD_SET_DDRAM | (col + offsets[row % LCD_ROWS]));
}
 
static void lcd_clear()
{
    lcd_command(LCD_CLEAR);
    vTaskDelay(pdMS_TO_TICKS(2));
}
 
static void lcd_print(const char* str)
{
    while (*str) {
        lcd_char(*str++);
    }
}
 
// Skriver ut en sträng på en specifik rad, paddar med mellanslag
static void lcd_print_row(const char* str, uint8_t row)
{
    lcd_set_cursor(0, row);
    char buf[LCD_COLS + 1];
    snprintf(buf, sizeof(buf), "%-16s", str);  // vänsterjustera, padda till 16
    lcd_print(buf);
}
 
// ── displayInit ──────────────────────────────────────────────────────────────
 
void displayInit()
{
    // Konfigurera I2C
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
 
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
 
    vTaskDelay(pdMS_TO_TICKS(50));   // vänta på LCD-uppstart
 
    // Initieringssekvens (4-bit läge)
    lcd_write_nibble(0x30, 0);  vTaskDelay(pdMS_TO_TICKS(5));
    lcd_write_nibble(0x30, 0);  vTaskDelay(pdMS_TO_TICKS(1));
    lcd_write_nibble(0x30, 0);  vTaskDelay(pdMS_TO_TICKS(1));
    lcd_write_nibble(0x20, 0);  vTaskDelay(pdMS_TO_TICKS(1));  // byt till 4-bit
 
    lcd_command(LCD_FUNCTION_SET);  // 4-bit, 2 rader
    lcd_command(LCD_DISPLAY_ON);    // display på, cursor av
    lcd_command(LCD_CLEAR);
    lcd_command(LCD_ENTRY_MODE);    // vänster→höger
 
    vTaskDelay(pdMS_TO_TICKS(5));
    printf("Display init klar\n");
}
 
// ── Visningslägen ────────────────────────────────────────────────────────────
 
// Statisk text — wrappas till rad 2 om längre än 16 tecken
static void display_text(const char* text)
{
    lcd_clear();
    int len = strlen(text);
 
    if (len <= LCD_COLS) {
        lcd_print_row(text, 0);
    } else {
        char row1[LCD_COLS + 1] = {};
        char row2[LCD_COLS + 1] = {};
        strncpy(row1, text, LCD_COLS);
        strncpy(row2, text + LCD_COLS, LCD_COLS);
        lcd_print_row(row1, 0);
        lcd_print_row(row2, 1);
    }
}
 
// Scrollande text — rullar texten från höger till vänster på rad 0
static void display_scroll(const char* text)
{
    int len = strlen(text);
 
    // Bygg en buffert med mellanslag + text + mellanslag
    int bufLen = LCD_COLS + len + LCD_COLS;
    char* buf = new char[bufLen + 1];
    memset(buf, ' ', LCD_COLS);
    memcpy(buf + LCD_COLS, text, len);
    memset(buf + LCD_COLS + len, ' ', LCD_COLS);
    buf[bufLen] = '\0';
 
    for (int i = 0; i <= len + LCD_COLS; i++) {
        char window[LCD_COLS + 1];
        strncpy(window, buf + i, LCD_COLS);
        window[LCD_COLS] = '\0';
        lcd_print_row(window, 0);
        vTaskDelay(pdMS_TO_TICKS(300));   // scrollhastighet
    }
 
    delete[] buf;
}
 
// Blinkande text — visar/gömmer texten 6 gånger
static void display_blink(const char* text)
{
    for (int i = 0; i < 6; i++) {
        lcd_clear();
        if (i % 2 == 0) {
            display_text(text);
        }
        vTaskDelay(pdMS_TO_TICKS(700));
    }
}
 
// ── displayMessage ───────────────────────────────────────────────────────────
 
void displayMessage(const Message& msg)
{
    printf("Visar [%s]: %s\n", msg.type.c_str(), msg.text.c_str());
 
    const char* text = msg.text.c_str();
 
    if (msg.type == "scroll") {
        display_scroll(text);
    } else if (msg.type == "blink") {
        display_blink(text);
    } else {
        display_text(text);   // "text" är default
    }
}