# Data Logger

I needed a data logger for my garden and to experiment with data collection.
I've decided to build a system based on an ESP32 platform writing to an SD card.

Still figuring out what gets embedded into the microcontroller and what would get put on an SD card or if the SD card should just be able to start blank.

# library changes

The SQLITE library by default has too small of a stack, it must be increased to at least 100 in components/sqlite/config_ext.h