# console_img_viewer
Command line tool utility that allow you to display JPEG, PNG, GIF and WebP images directly into the console. Animated GIF and WebP are played in place. Images that are bigger than the terminal are automatically resized to fit inside it (the size of the font is taken into account to keep the right proportions), and JPEG photos are rotated following their EXIF orientation. 


<p align="center">
    <img src="img/README/result.png" width="100%" alt="Screenshot of the result on a 80x40 JPEG image." />
    <img src="img/README/result2.png" width="100%" alt="Screenshot of the result on a 30x45 JPEG image." />
    <br />
    <i align="center">Screenshots of the result.</i>
</p>

# Build
This project depends on `libjpeg`, `libpng`, `giflib` and `libwebp`. On Fedora you can install them with:

```bash
sudo dnf install libjpeg-turbo-devel libpng-devel giflib-devel libwebp-devel
```

On Debian/Ubuntu:

```bash
sudo apt install libjpeg-dev libpng-dev libgif-dev libwebp-dev
```

You can create an executable file using the `makefile` in the project folder and executing this command:

```bash
make
```
After that, you can run the script by using the command:
```bash
./bin/tv [-l] <path_to_your_image> [other_images ...]
```

| Option | Description |
|---|---|
| `-l` | Loop animations until `Ctrl-C` (they are played once by default). |
| `-h` | Show the help. |

`Ctrl-C` during an animation stops the program. When the output is redirected to a file, only the first frame of an animation is written.

Errors are written on `stderr` as `tv: <file>: <message>`. The exit code is `0` when every file was displayed, `1` when at least one file could not be displayed and `130` when the program was stopped with `Ctrl-C`.

You can also install `tv` system wide (default prefix is `/usr/local`):
```bash
sudo make install
```

# Project structure
```
srcs/               sources, main.c is at the root
├── cli/            command-line arguments parsing
├── decoders/       one decoder per image format (JPEG, PNG, GIF, WebP) and EXIF parsing
├── display/        terminal size, rendering and animation playback
├── image/          image creation, resizing, rotation and drawing
└── utils/          color conversion helpers
includes/           headers, same layout as srcs/
bin/                compiled executable (tv)
```

# Limitation 
Please note that the current version of this program only supports JPEG, PNG, GIF and WebP file formats (the format is detected from the file content, not its extension). Transparent pixels are displayed over a black background. Images are only shrunk to fit the terminal, small images are never enlarged. Feel free to implement other formats yourself! :)

# Licence
- romainflcht