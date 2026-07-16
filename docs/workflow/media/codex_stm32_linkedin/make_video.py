from __future__ import annotations

import subprocess
import textwrap
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parent
SLIDES = ROOT / "slides"
OUT = ROOT / "codex_stm32_vscode_linkedin.mp4"
CONCAT = ROOT / "concat.txt"

W, H = 1080, 1920
BG = "#08111f"
PANEL = "#101c2f"
TEXT = "#edf5ff"
MUTED = "#9fb3c8"
CYAN = "#50d5ff"
GREEN = "#72f7a1"
YELLOW = "#ffd166"
RED = "#ff6b6b"


def font(name: str, size: int):
    path = Path("C:/Windows/Fonts") / name
    return ImageFont.truetype(str(path), size=size) if path.exists() else ImageFont.load_default()


FONT_TITLE = font("segoeuib.ttf", 60)
FONT_H2 = font("segoeuib.ttf", 42)
FONT_BODY = font("segoeui.ttf", 36)
FONT_SMALL = font("segoeui.ttf", 28)
FONT_MONO = font("consola.ttf", 28)
FONT_MONO_SMALL = font("consola.ttf", 24)


def wrap(text: str, chars: int) -> list[str]:
    lines: list[str] = []
    for paragraph in text.split("\n"):
        lines.extend(textwrap.wrap(paragraph, width=chars) if paragraph else [""])
    return lines


def draw_wrapped(draw, text: str, x: int, y: int, chars: int, fnt, fill=TEXT, gap: int = 8) -> int:
    for line in wrap(text, chars):
        draw.text((x, y), line, font=fnt, fill=fill)
        y += int(fnt.size * 1.25) + gap
    return y


def panel(draw, xy, title: str, lines: list[str], small: bool = False) -> None:
    x1, y1, x2, y2 = xy
    draw.rounded_rectangle(xy, radius=28, fill=PANEL, outline="#223b5c", width=2)
    draw.ellipse((x1 + 26, y1 + 26, x1 + 42, y1 + 42), fill=RED)
    draw.ellipse((x1 + 54, y1 + 26, x1 + 70, y1 + 42), fill=YELLOW)
    draw.ellipse((x1 + 82, y1 + 26, x1 + 98, y1 + 42), fill=GREEN)
    draw.text((x1 + 124, y1 + 20), title, font=FONT_SMALL, fill=MUTED)
    fnt = FONT_MONO_SMALL if small else FONT_MONO
    y = y1 + 78
    for line in lines:
        color = GREEN if "PASS" in line or "OK" in line else CYAN if line.startswith(">") else TEXT
        draw.text((x1 + 34, y), line, font=fnt, fill=color)
        y += int(fnt.size * 1.28)


def header(draw, eyebrow: str, title: str) -> None:
    draw.text((72, 78), eyebrow.upper(), font=FONT_SMALL, fill=CYAN)
    draw_wrapped(draw, title, 72, 128, 31, FONT_TITLE, TEXT, gap=4)


def footer(draw, n: int) -> None:
    draw.line((72, H - 116, W - 72, H - 116), fill="#21344d", width=2)
    draw.text((72, H - 86), "STM32 + VS Code + Codex", font=FONT_SMALL, fill=MUTED)
    draw.text((W - 160, H - 86), f"{n:02d}/08", font=FONT_SMALL, fill=MUTED)


def bullet(draw, text: str, y: int) -> int:
    draw.ellipse((96, y + 10, 110, y + 24), fill=GREEN)
    return draw_wrapped(draw, text, 130, y, 34, FONT_BODY, TEXT, gap=6) + 10


def make_slide(n: int, eyebrow: str, title: str, body) -> Path:
    img = Image.new("RGB", (W, H), BG)
    draw = ImageDraw.Draw(img)
    draw.ellipse((-180, -220, 420, 380), fill="#102a44")
    draw.ellipse((780, 1120, 1280, 1650), fill="#0b3145")
    draw.rectangle((0, 0, W, H), outline="#182c44", width=6)
    header(draw, eyebrow, title)
    body(draw)
    footer(draw, n)
    path = SLIDES / f"slide_{n:02d}.png"
    img.save(path)
    return path


def main() -> None:
    SLIDES.mkdir(parents=True, exist_ok=True)
    slides = [
        make_slide(1, "real embedded workflow", "Codex inside an STM32 firmware project", lambda d: (
            draw_wrapped(d, "Not a toy demo. A real repo, STM32 tooling, CMake, VS Code, flashing, and UART validation.", 72, 420, 30, FONT_H2),
            panel(d, (72, 760, 1008, 1120), "the question", [
                "> Can an AI coding agent work inside",
                "> the constraints firmware engineers",
                "> actually care about?",
            ]),
        )),
        make_slide(2, "not just code generation", "The agent had to work like part of the engineering loop", lambda d: [
            bullet(d, text, y) for y, text in zip([470, 585, 700, 815, 930], [
                "Respect generated and protected STM32 files",
                "Keep the refactor small and reviewable",
                "Update CMake and documentation",
                "Use the canonical build flow",
                "Flash the board and read UART output",
            ])
        ]),
        make_slide(3, "the concrete task", "Clean up src/apps without changing firmware behavior", lambda d: (
            panel(d, (72, 430, 1008, 860), "original problem", [
                "src/apps mixed together:",
                "  product app",
                "  LCD bringups",
                "  panic/debug experiment",
                "  display-format validation",
                "  regression-like tests",
            ], small=True),
            draw_wrapped(d, "Reorganize it without adding unnecessary bureaucracy.", 92, 980, 33, FONT_H2),
        )),
        make_slide(4, "result", "A simple app taxonomy by role", lambda d: panel(d, (72, 430, 1008, 1050), "refactored app layout", [
            "src/apps/",
            "  product/",
            "    main/",
            "  bringups/",
            "    lcd/",
            "    lcd_blink/",
            "    display_format_lcd/",
            "    debug_panic/",
            "  tests/",
            "    regression/",
            "  template/",
        ])),
        make_slide(5, "engineering judgment", "The useful part was deciding what not to over-design", lambda d: panel(d, (72, 420, 1008, 920), "naming decisions", [
            "panic_demo -> bringups/debug_panic",
            "  validates panic/error/fault paths",
            "",
            "fmc_model_units_test -> tests/regression",
            "  now covers several firmware modules",
            "",
            "no demos/ folder yet",
            "  avoid premature taxonomy",
        ], small=True)),
        make_slide(6, "verification", "Codex compiled the selectable firmware profiles", lambda d: panel(d, (72, 410, 1008, 1080), "canonical STM32 builds", [
            "> FM_ACTIVE_APP=tests/regression        OK",
            "> FM_ACTIVE_APP=bringups/display_format_lcd OK",
            "> FM_ACTIVE_APP=bringups/lcd            OK",
            "> FM_ACTIVE_APP=bringups/lcd_blink      OK",
            "> FM_ACTIVE_APP=bringups/debug_panic    OK",
            "> FM_ACTIVE_APP=product/main            OK",
            "> FM_ACTIVE_APP=template                OK",
        ], small=True)),
        make_slide(7, "hardware loop", "The board confirmed the regression firmware over UART", lambda d: panel(d, (72, 360, 1008, 1270), "UART after flashing the STM32 target", [
            "DEBUG_INIT:MSG=ENABLED LED=ENABLED",
            "REGRESSION_TEST:START",
            "REGRESSION_TEST:INIT_DEFAULTS:PASS",
            "REGRESSION_TEST:TOTALS:PASS",
            "REGRESSION_TEST:RATE_WINDOWS:PASS",
            "REGRESSION_TEST:VOLUME_VALUES:PASS",
            "REGRESSION_TEST:DISPLAY_FORMAT_VALUES:PASS",
            "REGRESSION_TEST:DISPLAY_FORMAT_ERROR_PATHS:PASS",
            "REGRESSION_TEST:PASS",
        ], small=True)),
        make_slide(8, "takeaway", "AI coding agents become interesting when they close the loop", lambda d: (
            panel(d, (72, 390, 1008, 860), "the loop that matters", [
                "engineering intent",
                "  -> repository analysis",
                "  -> constrained refactor",
                "  -> documentation update",
                "  -> canonical STM32 build",
                "  -> board flash",
                "  -> UART regression PASS",
            ], small=True),
            draw_wrapped(d, "The question is not only: can the agent write C?", 86, 990, 30, FONT_H2),
            draw_wrapped(d, "Better: can it participate in the real firmware workflow?", 86, 1115, 29, FONT_H2, GREEN),
        )),
    ]

    with CONCAT.open("w", encoding="utf-8") as f:
        for slide in slides:
            f.write(f"file '{slide.as_posix()}'\n")
            f.write("duration 4.2\n")
        f.write(f"file '{slides[-1].as_posix()}'\n")

    subprocess.run([
        "ffmpeg", "-y", "-f", "concat", "-safe", "0", "-i", str(CONCAT),
        "-vf", "fps=30,format=yuv420p", "-movflags", "+faststart", str(OUT),
    ], check=True)
    print(OUT)


if __name__ == "__main__":
    main()

