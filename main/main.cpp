/*
    Simple Demo with WT32-SC01 + LovyanGFX + LVGL8.x
*/
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"

#define LGFX_WT32_SC01 // Seeed WT32-SC01
#define LGFX_USE_V1

//#define LGFX_AUTODETECT
#include <LovyanGFX.h>
#include <LGFX_AUTODETECT.hpp>
#include <vector>

#define LINE_COUNT 6

static LGFX lcd;

static std::vector<int> points[LINE_COUNT];
static int colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_CYAN, TFT_MAGENTA, TFT_YELLOW};
static int xoffset, yoffset, point_count;

int getBaseColor(int x, int y)
{
    return ((x ^ y) & 3 || ((x - xoffset) & 31 && y & 31) ? TFT_BLACK : ((!y || x == xoffset) ? TFT_WHITE : TFT_DARKGREEN));
}

extern "C"
{
    void app_main(void)
    {
        lcd.init();
        if (lcd.width() < lcd.height())
            lcd.setRotation(lcd.getRotation() ^ 1);

        yoffset = lcd.height() >> 1;
        xoffset = lcd.width() >> 1;
        point_count = lcd.width() + 1;

        for (int i = 0; i < LINE_COUNT; i++)
        {
            points[i].resize(point_count);
        }

        lcd.startWrite();
        lcd.setAddrWindow(0, 0, lcd.width(), lcd.height());
        for (int y = 0; y < lcd.height(); y++)
        {
            for (int x = 0; x < lcd.width(); x++)
            {
                lcd.writeColor(getBaseColor(x, y - yoffset), 1);
            }
        }
        lcd.endWrite();

        static int32_t x, y;
        while (1)
        {
            vTaskDelay(1);
            // lcd.printf("TEST LCD!\n");

            /* FPS calculation */
            static int prev_sec;
            static int fps;
            ++fps;
            int sec = 1000 / portTICK_PERIOD_MS;
            if (prev_sec != sec)
            {
                prev_sec = sec;
                lcd.setCursor(0, 0);
                lcd.printf("fps:%03d", fps);
                fps = 0;
            }

            static int count;

            // Setting up random data for graph
            for (int i = 0; i < LINE_COUNT; i++)
            {
                points[i][count % point_count] = (sinf((float)count / (10 + 30 * i)) + sinf((float)count / (13 + 37 * i))) * (lcd.height() >> 2);
            }

            ++count;

            // startWrite/endWrite is used to draw buffer at once=smooth graphics
            lcd.startWrite();
            int index1 = count % point_count;
            for (int x = 0; x < point_count - 1; x++)
            {
                int index0 = index1;
                index1 = (index0 + 1) % point_count;
                for (int i = 0; i < LINE_COUNT; i++)
                {
                    int y = points[i][index0];
                    if (y != points[i][index1])
                    {
                        lcd.writePixel(x, y + yoffset, getBaseColor(x, y));
                    }
                }

                for (int i = 0; i < LINE_COUNT; i++)
                {
                    int y = points[i][index1];
                    lcd.writePixel(x, y + yoffset, colors[i]);
                }
            }
            lcd.endWrite();

            /* Touch draw with rectangles
             * Have only this if you just want to only draw on screen
             */
            if (lcd.getTouch(&x, &y))
            {
                lcd.fillRect(x - 2, y - 2, 5, 5, count * 7);
                lcd.setCursor(380, 0);
                lcd.printf("Touch:(%03d,%03d)", x, y);
            }
        }
    }
}