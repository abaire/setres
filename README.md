# setres

Utility to set screen resolution to any of the modes reported by CoreGraphics.

## Usage

Display the supported modes:
```sh
setres
```

Change to a supported mode:
```sh
setres <width> <height>
```

## Build

Build the binary:
```sh
brew install cmake

cmake -S . -B build && cmake --build build
```
