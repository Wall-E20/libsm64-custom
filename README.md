
Libsm64 but i added some custom stuffs and QOL to it!
For now we got:
- Camera, including functions to change it's mode.
- some of Mario Body State + Level Of Detail

known bugs:
- the current way of exporting the camera doesn't allow for more marios (they fight for the camera)


original from:

https://github.com/libsm64/libsm64




leaving the original build instructions just in case:

## Building on Mac and Linux

- Ensure python3 is installed.
- Ensure the SDL2 and GLEW libraries are installed if you're building the test program (on Ubuntu: libsdl2-dev, libglew-dev).
- Run `make` to build. If you want to link musl libc instead of glibc run `LIBSM64_MUSL=1 make` instead.
- To run the test program you'll need a SM64 US ROM in the root of the repository with the name `baserom.us.z64`.

## Building on Windows
- [Follow steps 1-4 for setting up MSYS2 MinGW 64 here](https://github.com/sm64-port/sm64-port#windows), but replace the repository URL with `https://github.com/libsm64/libsm64.git`
- Ensure the SDL2 and GLEW libraries are installed if you're building the test program.
  - 64 bits: `pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-glew`.
  - 32 bits: `pacman -S mingw-w64-i686-SDL2 mingw-w64-i686-glew`.
- Run `make` to build
- To run the test program you'll need a SM64 US ROM in the root of the repository with the name `baserom.us.z64`.

## Building for Emscripten (Web) (WIP)

Run `emmake make CC=emcc`.

If you want libsm64 standalone to call from JS (may not be that
useful?), then run `emcc dist/libsm64.so -o libsm64.js` afterward to
emit `libsm64.wasm` and `libsm64.js`.

Otherwise, `dist/libsm64.so` should be a compiled WebAssembly object
that you can link into your own C project.

## Make targets (all platforms)

- `make lib`: (Default) Build the `dist` directory, containing the shared object or DLL and public-facing header.
- `make test`: Builds the library `dist` directory as well as the test program.
- `make run`: Build and run the SDL+OpenGL test program.
