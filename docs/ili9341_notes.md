# ILI9341 notes

PRG32 uses the ILI9341 as the reference display because 320x240 maps naturally to a 320x200 retro game viewport plus 40 vertical pixels for border/status information.

The driver uses SPI host `SPI2_HOST`, ESP-IDF DMA allocation via `SPI_DMA_CH_AUTO`, a 40 MHz SPI clock by default, and dirty rectangle tracking.

The physical ILI9341 backend is selected by default with
`CONFIG_PRG32_DISPLAY_ILI9341`. Desktop QEMU builds use
`CONFIG_PRG32_DISPLAY_QEMU_RGB` and the separate file
`components/prg32/prg32_display_qemu_rgb.c`.

Use the physical backend for final hardware validation because QEMU does not
exercise LCD wiring, SPI timing, backlight control, or ILI9341 byte order.
