# ILI9341 notes

PRG32 uses the ILI9341 as the reference display because 320x240 maps naturally
to a 320x200 retro game viewport plus 40 vertical pixels for border/status
information.

The driver uses SPI host `SPI2_HOST`, ESP-IDF DMA allocation via
`SPI_DMA_CH_AUTO`, a conservative 27 MHz SPI clock by default, and dirty
rectangle tracking. The initialization sequence follows the common
Adafruit-compatible ILI9341 power, VCOM, pixel format, gamma, sleep-out, and
display-on setup used by many low-cost 2.8 inch modules.

Reference ESP32-C6 wiring:

| ESP32-C6 | ILI9341 |
|---|---|
| GPIO7 | MOSI |
| GPIO2 | MISO / touch DO |
| GPIO6 | SCLK |
| GPIO10 | CS |
| GPIO8 | DC |
| GPIO9 | RST |
| GPIO5 | BL |

The physical ILI9341 backend is selected by default with
`CONFIG_PRG32_DISPLAY_ILI9341`. Desktop QEMU builds use
`CONFIG_PRG32_DISPLAY_QEMU_RGB` and the separate file
`components/prg32/prg32_display_qemu_rgb.c`.

Use the physical backend for final hardware validation because QEMU does not
exercise LCD wiring, SPI timing, backlight control, or ILI9341 byte order.

If the real display is black:

1. Confirm the serial monitor shows `PRG32 boot: app_main entered` and then the
   `prg32_lcd` pin log. If PlatformIO Monitor only shows ROM text, delete the generated
   `sdkconfig.prg32-esp32c6` once and rebuild so the UART-primary,
   USB-secondary console defaults are applied. If the port is locked, close
   other monitor windows before opening a new one.
2. Confirm the serial monitor logs `prg32_lcd` and the pins above.
3. Confirm the build directory is `build-esp32c6`, not `build-qemu`.
4. Confirm `CONFIG_PRG32_DISPLAY_ILI9341=y` in
   `build-esp32c6/config/sdkconfig.h`.
5. If the backlight is off, try setting
   `PRG32_LCD_BACKLIGHT_ACTIVE_LEVEL` to `0` in `main/prg32_config.h`.
6. If the backlight is on but pixels are unstable, lower
   `PRG32_LCD_SPI_CLOCK_HZ` to `20000000`.
