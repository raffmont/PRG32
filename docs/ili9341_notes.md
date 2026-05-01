# ILI9341 notes

PRG32 uses the ILI9341 as the reference display because 320x240 maps naturally to a 320x200 retro game viewport plus 40 vertical pixels for border/status information.

The driver uses SPI host `SPI2_HOST`, ESP-IDF DMA allocation via `SPI_DMA_CH_AUTO`, a 40 MHz SPI clock by default, and dirty rectangle tracking.
