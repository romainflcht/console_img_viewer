# console_img_viewer
Command line tool utility that allow you to display JPEG and PNG images directly into the console. Images that are bigger than the terminal are automatically resized to fit inside it. 


<p align="center">
    <img src="img/README/result.png" width="100%" alt="Screenshot of the result on a 80x40 JPEG image." />
    <img src="img/README/result2.png" width="100%" alt="Screenshot of the result on a 30x45 JPEG image." />
    <br />
    <i align="center">Screenshots of the result.</i>
</p>

# Build
This project depends on `libjpeg` and `libpng`. On Fedora you can install them with:

```bash
sudo dnf install libjpeg-turbo-devel libpng-devel
```

On Debian/Ubuntu:

```bash
sudo apt install libjpeg-dev libpng-dev
```

You can create an executable file using the `makefile` in the project folder and executing this command:

```bash
make
```
After that, you can run the script by using the command:
```bash
./bin/tv <path_to_your_image> [other_images ...]
```

You can also install `tv` system wide (default prefix is `/usr/local`):
```bash
sudo make install
```

# Project structure
```
srcs/               sources, main.c is at the root
├── cli/            command-line arguments parsing
├── decoders/       one decoder per image format (JPEG, PNG...)
├── display/        terminal size and pixel drawing
├── image/          image creation, resizing and drawing
└── utils/          color conversion helpers
includes/           headers, same layout as srcs/
bin/                compiled executable (tv)
```

# Limitation 
Please note that the current version of this program only supports JPEG and PNG file formats (the format is detected from the file content, not its extension). Transparent PNG pixels are displayed over a black background. Images are only shrunk to fit the terminal, small images are never enlarged. Feel free to implement other formats yourself! :)

# Licence
- romainflcht