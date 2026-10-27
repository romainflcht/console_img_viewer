# console_img_viewer
Command line tool utility that allow you to display JPEG and PNG images directly into the console. 


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
./build/main <path_to_your_image> [other_images ...]
```

# Limitation 
Please note that the current version of this program only supports JPEG and PNG file formats (the format is detected from the file content, not its extension). Transparent PNG pixels are displayed over a black background. Additionally, if the image file is too large, it may not display correctly in the terminal, as it could exceed the available number of rows and columns. Feel free to implement other formats yourself! :)

# Licence
- romainflcht