# Genshin Impact FOV Unlocker
A plugin for Genshin Impact that unlocks the camera's field of view (FOV) to values greater than the default of 45. It includes a DLL designed to be used in conjunction with [`genshin-fps-unlock`](https://github.com/34736384/genshin-fps-unlock).

![preview](https://github.com/user-attachments/assets/02d750bc-becd-4c9d-bd18-f38c2c1c1023)

## Usage
1. Download the latest release of [`genshin-fps-unlock`](https://github.com/34736384/genshin-fps-unlock/releases) if you haven't already.
2. Download the `genshin-fov-unlock` plugin from the [releases page](https://github.com/z3lx/genshin-fov-unlock/releases) or build it from source. 
3. Extract the contents of the downloaded archive to a folder of your choice.
4. Open the `genshin-fps-unlock` executable (`unlockfps_nc.exe`).
5. Navigate to `Options` → `Settings` → `DLLs` and add the path to the `genshin-fov-unlock.dll` file.

The plugin should now be loaded the next time you start the game with `genshin-fps-unlock`. The FOV can be adjusted by modifying the `field_of_view` value in the `fov_config.json` file located in the same directory as the DLL.

## Building from source
To build the plugin from source, a CMake configuration file is provided.
- This project can only be built on Windows using MSVC.
- Ensure that you have an internet connection during the project configuration process as it downloads dependencies. 
- If you encounter issues with the precompiled MinHook library, consider recompiling it from source.

## Attributions
- The offsets used in this project ([line 53 of `main.cpp`](https://github.com/z3lx/genshin-fov-unlock/blob/main/src/main.cpp#L53)) were obtained from the [`genshin-utility`](https://github.com/lanylow/genshin-utility) project, licensed under the GPL-3.0 License.
- The [`minhook`](https://github.com/TsudaKageyu/minhook) library is used under the minhook license.
- The [`nlohmann/json`](https://github.com/nlohmann/json) library is used under the MIT License.

## License
This project is licensed under the GPL-3.0 License. See the [LICENSE](LICENSE) file for more information.
