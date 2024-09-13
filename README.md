![Game Version](https://img.shields.io/badge/release-version%205.0-brightgreen)
![GitHub all releases](https://img.shields.io/github/downloads/z3lx/genshin-fov-unlock/total)

# Genshin Impact FOV Unlocker
A plugin for Genshin Impact that unlocks the camera's field of view (FOV) to values greater than the default of 45. It includes a DLL designed to be used in conjunction with [`genshin-fps-unlock`](https://github.com/34736384/genshin-fps-unlock).

https://github.com/user-attachments/assets/56a11762-ebd2-4093-8c1d-768f913bd063

## Disclaimer
While I have personally used this plugin without any issues and have not been banned, it is important to note that third-party plugins like this one are against the terms of use of the game. You are encouraged to review the source code and build it yourself to ensure transparency and understand the potential risks.

Furthermore, while the plugin itself is lightweight, increasing the field of view causes the game to render more objects on the screen, which will noticeably impact performance. Some graphical artifacts may also occur at extreme values.

## Features
- Unlocks the camera's FOV to values greater than the default of 45.
- Provides keybindings to change the FOV or disable the plugin while in-game.
- Offers more advanced configurable settings in a JSON file.
- Integrates almost seamlessly with the game, correctly handling character bursts that manipulate the camera's FOV. However, the plugin needs to be manually disabled for cutscenes.

## Usage
1. Download the latest release of [`genshin-fps-unlock`](https://github.com/34736384/genshin-fps-unlock/releases) if you haven't already.
2. Download the `genshin-fov-unlock` plugin from the [releases page](https://github.com/z3lx/genshin-fov-unlock/releases) or build it from source. 
3. Extract the contents of the downloaded archive to a folder of your choice.
4. Open the `genshin-fps-unlock` executable (`unlockfps_nc.exe`).
5. Navigate to `Options` → `Settings` → `DLLs` and add the path to the `genshin-fov-unlock.dll` file.

The plugin should now be loaded the next time the game is launched with `genshin-fps-unlock`. By default, the <kbd>left arrow</kbd> and <kbd>right arrow</kbd> keys cycle through the preset field of view values, and the <kbd>down arrow</kbd> key enables or disables the plugin.

## Configuration
The plugin's behavior and settings can optionally be customized through the `fov_config.json` file located in the same directory as the DLL. The following keys are available for configuration:

- `enabled` (bool): Default state of the plugin when the game starts.
- `fov` (int): Default FOV to use when the game starts.
- `fov_presets` (array of int): List of FOV values to cycle through using keybindings.
- `smoothing` (float): Time constant in seconds for the exponential filter. Lower values makes the FOV changes more responsive.
- `threshold` (float): Time in milliseconds to use as a threshold for frame counting. An incorrect value may cause the FOV to fluctuate or not change at all. Default of 4 ms should work for most systems as long as the FPS cap is not above 200.
- `enable_key` (int): Key to enable or disable the plugin.
- `next_key` (int): Key to cycle to the next FOV preset.
- `prev_key` (int): Key to cycle to the previous FOV preset.

Note: Key codes should be in decimal format. Refer to the [virtual key codes documentation](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes) for valid values.

The default configuration is as follows:

```json
{
    "enabled": true,
    "fov": 75,
    "fov_presets": [
        30,
        45,
        60,
        75,
        90,
        110
    ],
    "smoothing": 0.2,
    "threshold": 4.0,
    "enable_key": 40,
    "next_key": 39,
    "prev_key": 37
}
```

## Building
If you just want to use the plugin, you should follow the instructions in the [Usage](#usage) section. To build the plugin from source, follow these steps:

1. Ensure you are on Windows and have git, CMake, and MSVC installed.
2. Clone the repository and navigate to the project directory:
```bash
git clone https://github.com/z3lx/genshin-fov-unlock.git
cd genshin-fov-unlock
```
3. Configure the project (internet connection required for dependencies):
```bash
cmake .
```
4. Build the project:
```bash
cmake --build . --config Release
```

The compiled DLL will be located in the `Release` directory. If you encounter issues with the precompiled MinHook library, consider recompiling it from source.

## Attributions
- The offsets used in this project ([line 87 of `main.cpp`](https://github.com/z3lx/genshin-fov-unlock/blob/main/src/main.cpp#L87)) were obtained from the [`genshin-utility`](https://github.com/lanylow/genshin-utility) project, licensed under the GPL-3.0 License.
- The [`minhook`](https://github.com/TsudaKageyu/minhook) library is used under the BSD-2-Clause.
- The [`nlohmann/json`](https://github.com/nlohmann/json) library is used under the MIT License.

## License
This project is licensed under the GPL-3.0 License. See the [LICENSE](LICENSE) file for more information.
