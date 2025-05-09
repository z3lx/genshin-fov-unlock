![Game Version](https://img.shields.io/badge/version-%205.6-brightgreen)
![GitHub all releases](https://img.shields.io/github/downloads/z3lx/genshin-fov-unlock/total)

# Genshin Impact FOV Unlocker
A plugin for Genshin Impact that unlocks the camera's field of view (FOV) to values greater than the default of 45. It includes a DLL designed to be used in conjunction with [**genshin-fps-unlock**](https://github.com/34736384/genshin-fps-unlock). If you find the plugin useful, consider starring the repository 🌟!

https://github.com/user-attachments/assets/56a11762-ebd2-4093-8c1d-768f913bd063

## Disclaimer
While I have personally used this plugin without any issues and have not been banned, it is important to note that third-party plugins like this one are against the terms of use of the game. You are encouraged to review the source code and build it yourself to ensure transparency and understand the potential risks.

Furthermore, while the plugin itself is lightweight, increasing the field of view causes the game to render more objects on the screen, which will noticeably impact performance. Some graphical artifacts may also occur at extreme values.

## Installation
1. Download the latest release of the **genshin-fps-unlock** tool.
   1. Navigate to the project's [**latest release**](https://github.com/34736384/genshin-fps-unlock/releases/latest).
   2. Download the **unlockfps_nc.exe** executable from the **Assets** section.
   3. Place the executable in a folder of your choice.
2. Download the latest release of the **genshin-fov-unlock** plugin.
   1. Navigate to the project's [**latest release**](https://github.com/z3lx/genshin-fov-unlock/releases/latest).
   2. Download the **plugin.zip** archive from the **Assets** section.
   3. Unzip the contents of the archive to a folder of your choice **other than the root directory of the genshin-fps-unlock tool**. The archive includes the **genshin_fov_unlock.dll** library and the **fov_config.json** configuration.
3. Configure the **genshin-fps-unlock** tool to work with the plugin.
   1. Open the **unlockfps_nc.exe** executable.
   2. Navigate to **Options** → **Settings** → **DLLs** → **Add**.
   3. Add the path to the **genshin_fov_unlock.dll** file.
4. (Optional) Modify the plugin's settings.
   1. Open the **fov_config.json** file in a text editor.
   2. Modify the settings as desired. Refer to the [**Configuration**](#Configuration) section for more information.

The plugin should now be loaded the next time the game is launched with the **genshin-fps-unlock** tool by running the **unlockfps_nc.exe** executable. For more information on how to use the tool, refer to the project's [**README**](https://github.com/34736384/genshin-fps-unlock/blob/netcore/README.md).

## Usage
By default, the <kbd>left arrow</kbd> and <kbd>right arrow</kbd> keys cycle through the preset field of view values, and the <kbd>down arrow</kbd> key enables or disables the plugin.

## Configuration
The plugin's behavior and settings can optionally be customized through the **fov_config.json** file located in the same directory as the **genshin_fov_unlock.dll** library. The following settings are available for configuration:

- `enabled` (bool): Default state of the plugin when the game starts.
- `fov` (int): Default FOV to use when the game starts.
- `fov_presets` (array of int): List of FOV values to cycle through using keybindings.
- `smoothing` (float): Time constant in seconds for the exponential filter. Lower values make the FOV changes more responsive. Setting this value too high may trigger the game's integrity check in some cases.
- `enable_key` (int): Key to enable or disable the plugin.
- `next_key` (int): Key to cycle to the next FOV preset.
- `prev_key` (int): Key to cycle to the previous FOV preset.
- `dump_key` (int): Key to dump the current plugin state to the log. Does nothing if plugin is compiled without logging.

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
    "smoothing": 0.125,
    "enable_key": 40,
    "next_key": 39,
    "prev_key": 37,
    "dump_key": 123
}
```

## Building
If you just want to use the plugin, you should follow the instructions in the [Installation](#Installation) section. To build the plugin from source, follow these steps:

1. Ensure you are on Windows and have git, CMake, and MSVC installed.
2. Clone the repository and navigate to the project directory:
```bash
git clone https://github.com/z3lx/genshin-fov-unlock.git
cd genshin-fov-unlock
```
3. Configure the project (internet connection required for dependencies):
```bash
cmake . -G "Visual Studio 17 2022" -DENABLE_LOGGING=OFF
```
4. Build the project:
```bash
cmake --build . --config Release
```

The compiled **genshin_fov_unlock.dll** library will be located in the **Release** directory. If you encounter issues with the precompiled MinHook library, consider recompiling it from source.

## Attributions
- The [**minhook**](https://github.com/TsudaKageyu/minhook) library is used under the BSD-2-Clause.
- The [**nlohmann/json**](https://github.com/nlohmann/json) library is used under the MIT License.
- Originally inspired from [**genshin-utility**](https://github.com/lanylow/genshin-utility).

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more information.
