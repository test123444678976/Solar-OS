#!/usr/bin/env bash
# Solar OS - build: kernel.org Linux + /init próprio (sem BusyBox, sem libc dinâmica)
set -euo pipefail

KVER="${KVER:-6.12.10}"                      # muda para outra versão de kernel.org se quiseres
MAJOR="v${KVER%%.*}.x"
HERE="$(cd "$(dirname "$0")" && pwd)"
BUILD="$HERE/build"
mkdir -p "$BUILD"; cd "$BUILD"

# Dependências (Debian/Ubuntu): build-essential flex bison bc libelf-dev libssl-dev wget xz-utils qemu-system-x86

# 1) Kernel oficial de kernel.org
if [ ! -d "linux-$KVER" ]; then
  [ -f "linux-$KVER.tar.xz" ] || wget "https://cdn.kernel.org/pub/linux/kernel/$MAJOR/linux-$KVER.tar.xz"
  tar xf "linux-$KVER.tar.xz"
fi

# 2) /init (único programa de userspace)
gcc -static -O2 -o init "$HERE/init.c"
gcc -static -O2 -o solar-desktop "$HERE/desktop.c"

# 3) initramfs embutido no kernel
cat > initramfs.list <<LIST
dir /dev 755 0 0
dir /proc 755 0 0
dir /sys 755 0 0
nod /dev/console 600 0 0 c 5 1
nod /dev/null 666 0 0 c 1 3
file /init $BUILD/init 755 0 0
file /solar-desktop $BUILD/solar-desktop 755 0 0
LIST

# 4) Configuração mínima
cd "linux-$KVER"
make tinyconfig
./scripts/config \
  -e 64BIT -e PRINTK -e BLK_DEV_INITRD -e RD_GZIP \
  --set-str INITRAMFS_SOURCE "$BUILD/initramfs.list" \
  -e TTY -e SERIAL_8250 -e SERIAL_8250_CONSOLE -e VT -e VT_CONSOLE -e VGA_CONSOLE \
  -e BINFMT_ELF -e PROC_FS -e SYSFS -e DEVTMPFS -e DEVTMPFS_MOUNT \
  -e PCI -e ACPI -e MULTIUSER \
  -e FB -e FB_DEVICE -e FB_VESA -e FB_EFI -e FB_SIMPLE -e SYSFB_SIMPLEFB \
  -e INPUT -e INPUT_EVDEV -e INPUT_KEYBOARD -e KEYBOARD_ATKBD -e INPUT_MOUSE -e MOUSE_PS2 \
  -e SERIO -e SERIO_I8042 -e SERIO_LIBPS2 -e HID -e HID_GENERIC -e INPUT_MOUSEDEV \
  -e USB_SUPPORT -e USB -e USB_HID -e USB_XHCI_HCD -e USB_XHCI_PCI -e USB_EHCI_HCD -e USB_EHCI_PCI \
  -e USB_OHCI_HCD -e USB_OHCI_PCI -e USB_UHCI_HCD \
  --set-str DEFAULT_HOSTNAME "solar"
make olddefconfig
make -j"$(nproc)" bzImage

cp arch/x86/boot/bzImage "$HERE/solar-os.bzImage"
echo "Solar OS pronto: $HERE/solar-os.bzImage"
echo "Corre: ./run.sh"
