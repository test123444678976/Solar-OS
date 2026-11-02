#!/usr/bin/env bash
cd "$(dirname "$0")"
# Sai do QEMU com Ctrl+A depois X
qemu-system-x86_64 -kernel solar-os.bzImage -m 256M -nographic \
  -append "console=ttyS0 quiet"
