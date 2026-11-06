#!/usr/bin/env bash
cd "$(dirname "$0")"
# Janela gráfica com o desktop. O rato é "absoluto" (usb-tablet): sem precisar de capturar.
qemu-system-x86_64 -kernel solar-os.bzImage -m 256M \
  -append "console=ttyS0 vga=0x318" \
  -device qemu-xhci -device usb-tablet -serial stdio
