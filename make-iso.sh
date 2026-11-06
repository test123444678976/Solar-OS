#!/usr/bin/env bash
# Solar OS - gera solar-os.iso (arranca em BIOS e UEFI via GRUB)
# Dependências: grub-pc-bin grub-efi-amd64-bin xorriso mtools  (Debian/Ubuntu)
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE"

[ -f solar-os.bzImage ] || ./build.sh

ISO_DIR="$HERE/build/iso"
rm -rf "$ISO_DIR"; mkdir -p "$ISO_DIR/boot/grub"
cp solar-os.bzImage "$ISO_DIR/boot/solar-os"

cat > "$ISO_DIR/boot/grub/grub.cfg" <<GRUB
set timeout=3
set default=0
insmod all_video
set gfxpayload=1024x768x32
menuentry "Solar OS" {
    linux /boot/solar-os console=ttyS0 quiet
}
menuentry "Solar OS (verbose)" {
    linux /boot/solar-os console=ttyS0
}
GRUB

grub-mkrescue -o solar-os.iso "$ISO_DIR" -V SOLAR_OS
echo "ISO pronta: $HERE/solar-os.iso"
echo "Testa: qemu-system-x86_64 -cdrom solar-os.iso -m 256M"
