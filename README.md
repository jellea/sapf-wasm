# sapf, cross-platform edition

this is a highly work-in-progress fork of James McCartney's [sapf](https://github.com/lfnoise/sapf) (Sound As Pure Form) which aims to implement cross-platform alternatives for the various macOS libraries used in the original codebase. for the time being, the top priority platform is Linux.

[original README](README.txt)

## building

a Nix flake is included. simply run:

```shell
nix develop
meson setup build
meson compile -C build
```

and you should get a binary at `./build/sapf`.

if not using Nix, you will need to install dependencies manually instead of the `nix develop`. the mandatory dependencies for a portable build are currently:

- libedit
- libsndfile
- fftw
- rtaudio

for installing dependencies, you can refer to the CI scripts in this repo:

- [install-debian-deps.sh](.github/scripts/install-debian-deps.sh) (Debian, Ubuntu, Mint, etc.)
- [install-macos-deps.sh](.github/scripts/install-macos-deps.sh) (macOS with Homebrew)

## Windows Usage Caveats

Windows support is currently WIP. The following current limitations apply:

- In order to paste multiline contents, you should make sure the line ending is LF only (not Windows-style CRLF). CRLF pasting isn't working right currently.
- It will use ASIO and will use the "first" enabled ASIO device, which may not be what you want. Opening the ASIO4ALL panel (in the taskbar tray) and disabling
    all inputs other than your preferred input, then restarting SAPF, should allow it to switch.

## Windows Build + Development

(I'm new to this toolchain so some things may be wrong)

Windows support is achieved by building via [msys2](https://www.msys2.org/) (using mingw-w64-ucrt6) as opposed to building natively on Windows (probably possible but probably much more annoying).

I believe the project specifically requires clang.

If you're for some reason not on x86_64, you'll have to replace any of the below references to that architecture
with your own! You can find and view info on packages on [msys2 packages](https://packages.msys2.org/queue) to see
if the package exists for your architecture. Note these packages often also have "-clang" versions, but AFAICT we
DON'T want to install those as they end up in a location that isn't found by default for this toolchain.

1. Install [msys2](https://www.msys2.org/). Make sure to keep track of where you installed it as this will be where your
"root directory" is for your msys2 / mingw64 shells. For this guide we will assume the default of `C:\msys64`
1. If you haven't yet, clone or copy this repo somewhere inside the msys2 install. For example within the msys2 shell you could install git via
`pacman -S git` and then git clone this repo into your "home" folder.
1. Open a msys2 (ucrt) shell and install some needed development dependencies
   ```shell
   # press ENTER when prompted to choose "all"
   pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain
   pacman -S \
    mingw-w64-ucrt-x86_64-meson \
    mingw-w64-ucrt-x86_64-clang \
    mingw-w64-ucrt-x86_64-fftw \
    mingw-w64-ucrt-x86_64-libsndfile \
    mingw-w64-ucrt-x86_64-pkgconf \
    mingw-w64-ucrt-x86_64-rtaudio \
    mingw-w64-ucrt-x86_64-readline
   ```
1. Close and reopen the shell to ensure it loads everything you just installed. 
1. Now we can try to build in the msys2 (ucrt) shell. We need to always pass our `--native-file` which forces meson to use clang.
Navigate to the root directory of this repo.
1.  ```shell
    # remove --buildtype debug to build without debug symbols (I think?)
    meson setup --buildtype debug --native-file clang.ini build 
    meson compile -C build
    ```
1. You should see `sapf.exe` is created under the `${workspaceFolder}/build` directory.
1. You need all the required DLLs in order to run it via Windows. Go to your msys2 folder `C:\msys64\ucrt64\bin`
and copy all of the dlls that look like `lib*.dll` (i.e. libreadline8.dll, libogg-0.dll, etc...). This is
more than needed but I'm not sure the exact subset of dlls needed yet.
1. Now you can run the exe directly by clicking or via your preferred command prompt.
1. Test if its all working with a simple command (you should hear audio)
`15 .0 sinosc 200 * 300 + .0 sinosc .1 * play`
1. If you didn't hear anything check if ASIO4ALL launched (in the taskbar icons) and
open it up and enable your preferred output device. Then restart sapf. 
    - We plan to eventually make it possible to select an audio device, which is kind of necessary for Windows but not so much for other systems.

### Windows VSCode Development Setup
Since it's not exactly straightforward to get everything working nicely in VSCode under Windows, here's 
some guidance.

The trick to making it work is to make sure VSCode is using the ucrt64 binaries for the development toolchain,
NOT anything installed natively to Windows.

1. Install C/C++ extensions for VSCode.
1. Install Meson extension for VSCode.
1. In the settings for Meson, set Meson build path to `C:\msys64\ucrt64\bin\meson` and
set the Build folder to `build`.
1. Add your `C:\msys64\ucrt\bin` folder to your Path (via Environment Variables).
1. Open a windows terminal and make sure that `gcc --version` and `g++ --version` and `gdb --version` return
a version string. You can also confirm with `where gcc` that it's using the binary within msys2.
1. Before opening the folder in vscode, create a `.vscode` subdolder and populate it with some files. See the below "Config files" section for some example config files. Tweak the paths to match your own system.
1. Open a cpp file to make sure the extensions activate.
1. You should now have intellisense working (you should be able to "Go to definition" and "find references, etc...").
1. You can now build using the Meson build task instead of msys2 shell if you prefer.
1. You can debug via selecting the "Attach (sapf)" configuration (bottom left), manually
running sapf.exe, and then presing F5 and attaching to the sapf.exe process. (Currently haven't figured out
how to get the "launch" version working - it runs but the text is garbled - likely an encoding issue).
    - If it times out waiting to attach, try restarting VSCode and maybe also
    closing any msys2 shells if you have any open (not sure what's going on here)?



#### Config files
Example `.vscode/c_cpp_properties.json`
```json
{
    "configurations": [
        {
            "name": "ucrt64",
            "includePath": [
                "C:/msys64/ucrt64/include/**",
                "${workspaceFolder}/**"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE"
            ],
            "compilerPath": "C:/msys64/ucrt64/bin/clang++.exe",
            "windowsSdkVersion": "10.0.22621.0",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "windows-clang-x64",
            "configurationProvider": "mesonbuild.mesonbuild"
        }
    ],
    "version": 4
}
```

Example `.vscode/launch.json`
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "(gdb) Attach",
            "type": "cppdbg",
            "request": "attach",
            "program": "${workspaceRoot}/build/sapf.exe",
            "MIMode": "gdb",
            "miDebuggerPath": "C:\\msys64\\ucrt64\\bin\\gdb.exe",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                },
                {
                    "description": "Set Disassembly Flavor to Intel",
                    "text": "-gdb-set disassembly-flavor intel",
                    "ignoreFailures": true
                }
            ]
        },
        {
            "name": "(gdb) Launch (chairbender note - not working ATM, use attach instead - everything is garbled)",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceRoot}/build/sapf.exe",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceRoot}/build",
            "environment": [
                { "name": "MSYSTEM", "value": "UCRT64" },
                { "name": "MSYS2_PATH_TYPE", "value": "inherit" },
                { "name": "PATH", "value": "C:\\msys64\\ucrt64\\bin;${env:PATH}" }
            ],
            "externalConsole": false,
            "MIMode": "gdb",
            "miDebuggerPath": "C:\\msys64\\ucrt64\\bin\\gdb.exe",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                },
                {
                    "description": "Set Disassembly Flavor to Intel",
                    "text": "-gdb-set disassembly-flavor intel",
                    "ignoreFailures": true
                }
            ]
        }
    ]
}
```

Example `.vscode/settings.json`
```json
{
    "C_Cpp.default.compileCommands": "c:\\msys64\\home\\kwhip\\sapf\\build/compile_commands.json",
    "C_Cpp.default.configurationProvider": "mesonbuild.mesonbuild"
}
```

Example `.vscode/tasks.json`
```json
{
	"version": "2.0.0",
	"tasks": [
		{
			"type": "meson",
			"target": "sapf:executable",
			"mode": "build",
			"problemMatcher": [
				"$meson-gcc"
			],
			"group": "build",
			"label": "Meson: Build sapf:executable"
		}
	]
}
```