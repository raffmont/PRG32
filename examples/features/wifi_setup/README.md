# Wi-Fi Setup Demo

This demo shows the framework-level Wi-Fi setup mode.

- The resident firmware enters setup after the startup splash when Wi-Fi is
  enabled.
- Custom builds can disable that policy and still hold the setup GPIO during
  boot to enter setup automatically.
- Or run this demo and press A to open the setup UI from a game-like program.
- Use the joystick keyboard to enter STA or AP credentials.

Use entry prefix `wifi_setup_c`.
