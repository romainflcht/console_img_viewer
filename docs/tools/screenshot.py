#!/usr/bin/env python3
"""Generate the README screenshots and demo of tv.

tv is run in a real pseudo terminal, its output is replayed in a small
terminal emulator and the screen is drawn in a terminal window (PNG), or
every frame of an animation is drawn (animated WebP).

Requires Python 3 and Pillow. Run from the root of the repository after
`make`:

    python3 docs/tools/screenshot.py

The images are written to docs/assets/.
"""

import fcntl
import math
import os
import pty
import re
import select
import struct
import subprocess
import sys
import tempfile
import termios

from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
ASSETS = os.path.join(ROOT, "docs", "assets")
TV = os.path.join(ROOT, "bin", "tv")
# tv runs in a temporary directory where the sample images have readable 
# names, so the commands shown match the real output. 
WORKDIR = tempfile.mkdtemp(prefix="tv-screenshots-")
SAMPLES = {
    "photo.jpg": os.path.join(ROOT, "img", "img(800x400).jpeg"),
    "plasma.gif": os.path.join(ROOT, "img", "plasma.gif"),
    "anim.webp": os.path.join(ROOT, "img", "img(40x30).webp"),
}

# Look of the terminal window.
CELL_W, CELL_H = 10, 20
PADDING = 16
TITLE_BAR = 34
BACKGROUND = (30, 30, 46)
FOREGROUND = (205, 214, 244)
PROMPT = (166, 227, 161)
WINDOW_BAR = (24, 24, 37)
DOTS = [(243, 139, 168), (249, 226, 175), (166, 227, 161)]
FONT_PATHS = [
    "/usr/share/fonts/jetbrains-mono-fonts/JetBrainsMono-Regular.otf",
    "/usr/share/fonts/google-noto-vf/NotoSansMono[wght].ttf",
    "/usr/share/fonts/dejavu-sans-mono-fonts/DejaVuSansMono.ttf",
]

# Colors assumed by tv for the 16 system colors (xterm defaults).
SYSTEM = [
    (0, 0, 0), (205, 0, 0), (0, 205, 0), (205, 205, 0),
    (0, 0, 238), (205, 0, 205), (0, 205, 205), (229, 229, 229),
    (127, 127, 127), (255, 0, 0), (0, 255, 0), (255, 255, 0),
    (92, 92, 255), (255, 0, 255), (0, 255, 255), (255, 255, 255),
]
LEVELS = [0, 95, 135, 175, 215, 255]


def color_256(n):
    if n < 16:
        return SYSTEM[n]
    if n >= 232:
        gray = 8 + (n - 232) * 10
        return (gray, gray, gray)
    n -= 16
    return (LEVELS[n // 36], LEVELS[n // 6 % 6], LEVELS[n % 6])


def font():
    for path in FONT_PATHS:
        if os.path.exists(path):
            return ImageFont.truetype(path, 16)
    return ImageFont.load_default()


FONT = font()


def run(args, cols, rows, env=None):
    """Run tv in a pseudo terminal of cols x rows, return its output."""
    master, slave = pty.openpty()
    fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", rows, cols, cols * CELL_W, rows * CELL_H))
    full_env = {"PATH": os.environ["PATH"], "LANG": "C.UTF-8", "TERM": "xterm-256color", "COLORTERM": "truecolor"}
    full_env.update(env or {})
    process = subprocess.Popen([TV] + args, stdin=slave, stdout=slave, stderr=slave, env=full_env, close_fds=True, cwd=WORKDIR)
    os.close(slave)
    output = b""
    while True:
        ready, _, _ = select.select([master], [], [], 0.1)
        if ready:
            try:
                data = os.read(master, 65536)
            except OSError:
                break
            if not data:
                break
            output += data
        elif process.poll() is not None:
            break
    process.wait()
    return output.decode("utf-8", "replace")


class Screen:
    """Minimal terminal: cursor moves, SGR colors and synchronized output."""

    TOKEN = re.compile(r"\x1b\[\?(\d+)([hl])|\x1b\[([0-9;]*)([mABCDHJK])|\x1b[78]|(\r)|(\n)|([^\x1b\r\n])", re.S)

    def __init__(self, cols, rows):
        self.cols, self.rows = cols, rows
        self.cells = {}
        self.row = self.col = 0
        self.fg = self.bg = None
        self.bold = False
        self.frames = []

    def snapshot(self):
        self.frames.append(dict(self.cells))

    def sgr(self, params):
        values = [int(v) for v in params.split(";") if v != ""] or [0]
        i = 0
        while i < len(values):
            v = values[i]
            if v in (38, 48):
                if values[i + 1] == 2:
                    color, i = tuple(values[i + 2:i + 5]), i + 5
                else:
                    color, i = color_256(values[i + 2]), i + 3
                if v == 38:
                    self.fg = color
                else:
                    self.bg = color
                continue
            if v == 0:
                self.fg = self.bg = None
                self.bold = False
            elif v == 1:
                self.bold = True
            elif 30 <= v <= 37:
                self.fg = SYSTEM[v - 30]
            elif 90 <= v <= 97:
                self.fg = SYSTEM[v - 90 + 8]
            elif 40 <= v <= 47:
                self.bg = SYSTEM[v - 40]
            elif 100 <= v <= 107:
                self.bg = SYSTEM[v - 100 + 8]
            i += 1

    def feed(self, text):
        synced = False
        for m in self.TOKEN.finditer(text):
            if m.group(1):
                if m.group(1) == "2026" and m.group(2) == "h" and not synced:
                    self.snapshot()
                    synced = True
                elif m.group(1) == "2026" and m.group(2) == "l":
                    self.snapshot()
            elif m.group(4):
                n = int(m.group(3) or 1) if m.group(4) in "ABCD" else 0
                if m.group(4) == "m":
                    self.sgr(m.group(3))
                elif m.group(4) == "A":
                    self.row = max(0, self.row - n)
                elif m.group(4) == "B":
                    self.row += n
                elif m.group(4) == "C":
                    self.col += n
                elif m.group(4) == "D":
                    self.col = max(0, self.col - n)
            elif m.group(5):
                self.col = 0
            elif m.group(6):
                self.row += 1
                self.col = 0
            elif m.group(7):
                self.cells[(self.row, self.col)] = (m.group(7), self.fg, self.bg, self.bold)
                self.col += 1
        self.snapshot()


def draw_window(cells, cols, rows, title, prompt_lines):
    """Draw a terminal window: title bar, prompt lines, then the cells."""
    width = cols * CELL_W + 2 * PADDING
    height = TITLE_BAR + (rows + len(prompt_lines)) * CELL_H + 2 * PADDING
    image = Image.new("RGB", (width, height), BACKGROUND)
    draw = ImageDraw.Draw(image)
    draw.rectangle([0, 0, width, TITLE_BAR], fill=WINDOW_BAR)
    for i, dot in enumerate(DOTS):
        draw.ellipse([14 + i * 22, 11, 26 + i * 22, 23], fill=dot)
    draw.text((width // 2, TITLE_BAR // 2), title, font=FONT, fill=(147, 153, 178), anchor="mm")

    top = TITLE_BAR + PADDING
    for i, line in enumerate(prompt_lines):
        y = top + i * CELL_H
        draw.text((PADDING, y + CELL_H // 2), "$", font=FONT, fill=PROMPT, anchor="lm")
        draw.text((PADDING + 2 * CELL_W, y + CELL_H // 2), line, font=FONT, fill=FOREGROUND, anchor="lm")
    top += len(prompt_lines) * CELL_H

    for (row, col), (char, fg, bg, bold) in cells.items():
        if row >= rows or col >= cols:
            continue
        x, y = PADDING + col * CELL_W, top + row * CELL_H
        fg = fg or FOREGROUND
        if bg:
            draw.rectangle([x, y, x + CELL_W - 1, y + CELL_H - 1], fill=bg)
        # Blocks are drawn as rectangles so they are exact and seamless.
        if char == "▄":
            draw.rectangle([x, y + CELL_H // 2, x + CELL_W - 1, y + CELL_H - 1], fill=fg)
        elif char == "▀":
            draw.rectangle([x, y, x + CELL_W - 1, y + CELL_H // 2 - 1], fill=fg)
        elif char == "█":
            draw.rectangle([x, y, x + CELL_W - 1, y + CELL_H - 1], fill=fg)
        elif char != " ":
            draw.text((x + CELL_W // 2, y + CELL_H // 2), char, font=FONT, fill=fg, anchor="mm",
                      stroke_width=1 if bold else 0, stroke_fill=fg)
    return image


def used_rows(cells):
    return max(row for row, _ in cells) + 1 if cells else 1


def screenshot(name, args, cols, rows, prompt, env=None, title="tv"):
    """Run tv and save the final screen as docs/assets/<name>.png."""
    output = run(args, cols, rows, env)
    screen = Screen(cols, rows)
    screen.feed(output)
    cells = screen.frames[-1]
    image = draw_window(cells, cols, used_rows(cells), title, [prompt])
    image.save(os.path.join(ASSETS, name + ".png"), optimize=True)
    return output


def demo_animation(name, scenes, cols, rows):
    """Save an animated WebP made of scenes: (args, prompt, frame_ms, hold_ms). 
    Lossless WebP is about 10 times smaller than GIF here, and tv can play it."""
    frames, durations = [], []
    for args, prompt, frame_ms, hold_ms in scenes:
        screen = Screen(cols, rows)
        screen.feed(run(args, cols, rows))
        # Typing the command, then the frames of tv.
        for i in range(0, len(prompt) + 1, 4):
            frames.append(draw_window({}, cols, rows, "tv", [prompt[:i]]))
            durations.append(60)
        for i, cells in enumerate(screen.frames):
            frames.append(draw_window(cells, cols, rows, "tv", [prompt]))
            durations.append(hold_ms if i == len(screen.frames) - 1 else frame_ms)
    frames[0].save(os.path.join(ASSETS, name + ".webp"), save_all=True, append_images=frames[1:],
                   duration=durations, loop=0, lossless=True, method=6)


def make_logo_source(path):
    """Pixel art of an old TV with color bars, drawn by tv for the logo."""
    image = Image.new("RGBA", (48, 40), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    draw.line([(18, 2), (23, 9)], fill=(180, 180, 190), width=1)
    draw.line([(31, 1), (25, 9)], fill=(180, 180, 190), width=1)
    draw.rounded_rectangle([2, 9, 45, 37], radius=4, fill=(120, 84, 60))
    draw.rounded_rectangle([5, 12, 35, 34], radius=3, fill=(20, 20, 30))
    bars = [(235, 235, 235), (235, 225, 40), (40, 220, 230), (40, 210, 60), (220, 40, 210), (230, 40, 40), (40, 60, 230)]
    for i, color in enumerate(bars):
        draw.rectangle([7 + i * 4, 14, 10 + i * 4, 32], fill=color)
    draw.ellipse([38, 14, 42, 18], fill=(210, 180, 120))
    draw.ellipse([38, 21, 42, 25], fill=(210, 180, 120))
    for y in range(28, 34, 2):
        draw.line([(37, y), (43, y)], fill=(70, 50, 36))
    draw.rectangle([8, 37, 11, 39], fill=(80, 56, 40))
    draw.rectangle([36, 37, 39, 39], fill=(80, 56, 40))
    image.save(path)


def make_plasma(path, size=(96, 54), count=24):
    """Colorful looping animation for the demo (generated, no copyright)."""
    frames = []
    w, h = size
    for t in range(count):
        phase = 2 * math.pi * t / count
        image = Image.new("RGB", size)
        pixels = image.load()
        for y in range(h):
            for x in range(w):
                v = (math.sin(x / 9 + phase) + math.sin(y / 7 - phase)
                     + math.sin((x + y) / 13 + 2 * phase) + math.sin(math.hypot(x - w / 2, y - h / 2) / 6 - phase))
                pixels[x, y] = (int(128 + 127 * math.sin(v * math.pi / 2)),
                                int(128 + 127 * math.sin(v * math.pi / 2 + 2)),
                                int(128 + 127 * math.sin(v * math.pi / 2 + 4)))
        frames.append(image)
    frames[0].save(path, save_all=True, append_images=frames[1:], duration=80, loop=0)


def main():
    if not os.path.exists(TV):
        sys.exit("build tv first: make")
    os.makedirs(ASSETS, exist_ok=True)
    logo_source = os.path.join(ASSETS, "logo-source.png")
    plasma = os.path.join(ROOT, "img", "plasma.gif")
    make_logo_source(logo_source)
    make_plasma(plasma)
    for link, target in SAMPLES.items():
        os.symlink(target, os.path.join(WORKDIR, link))

    # Logo: the pixel art drawn by tv, without the window. tv draws the 
    # transparent pixels in black, they are transparent again in the PNG. 
    screen = Screen(48, 20)
    screen.feed(run(["-q", "-c", "truecolor", logo_source], 48, 24))
    cells = screen.frames[-1]
    rows = used_rows(cells)
    logo = Image.new("RGBA", (48 * CELL_W, rows * CELL_H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(logo)
    for (row, col), (char, fg, bg, _) in cells.items():
        x, y = col * CELL_W, row * CELL_H
        for color, top in ((bg, y), (fg, y + CELL_H // 2)):
            if color and color != (0, 0, 0) and char == "▄":
                draw.rectangle([x, top, x + CELL_W - 1, top + CELL_H // 2 - 1], fill=color)
    logo.save(os.path.join(ASSETS, "logo.png"), optimize=True)
    os.remove(logo_source)

    # Demo: a photo, then an animation.
    demo_animation("demo", [
        (["photo.jpg"], "tv photo.jpg", 0, 2500),
        (["-n", "2", "plasma.gif"], "tv -n 2 plasma.gif", 80, 1500),
    ], 80, 24)

    # The same photo in every mode, with the size of the output.
    modes = [
        ("mode-truecolor", ["-c", "truecolor"], "truecolor"),
        ("mode-256", ["-c", "256"], "256 colors"),
        ("mode-16", ["-c", "16"], "16 colors"),
        ("mode-mono", ["-c", "mono"], "mono"),
        ("mode-ascii-16", ["-a", "-c", "16"], "ASCII, 16 colors"),
        ("mode-ascii-mono", ["-a", "-c", "mono"], "ASCII, mono"),
    ]
    print("| Mode | Output | Time at 115200 bauds |")
    print("|---|---|---|")
    for name, args, label in modes:
        output = screenshot(name, ["-q"] + args + ["photo.jpg"], 60, 18,
                            "tv -q " + " ".join(args) + " photo.jpg", title=label)
        size = len(output.encode())
        print(f"| {label} | {size / 1024:.1f} KB | {size * 10 / 115200:.1f} s |")

    # Serial console: 0x0 size, vt220, no locale, everything detected.
    screenshot("serial", ["photo.jpg"], 80, 24, "tv photo.jpg",
               env={"TERM": "vt220", "LANG": "", "COLORTERM": ""}, title="picocom /dev/ttyUSB0")

    # Information mode.
    screenshot("info", ["-i", "photo.jpg", "plasma.gif", "anim.webp"], 80, 4,
               "tv -i photo.jpg plasma.gif anim.webp")

    for link in SAMPLES:
        os.remove(os.path.join(WORKDIR, link))
    os.rmdir(WORKDIR)


if __name__ == "__main__":
    main()
