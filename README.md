# Solar OS ☀️
Sistema operativo minimalista: **kernel Linux (kernel.org) + um único `/init` em C**.
Sem BusyBox, sem glibc dinâmica, sem distro.

## Usar
    sudo apt install build-essential flex bison bc libelf-dev libssl-dev wget xz-utils qemu-system-x86
    ./build.sh      # descarrega o kernel, compila, gera solar-os.bzImage
    ./run.sh        # arranca no QEMU (Ctrl+A, X para sair)

Para outra versão: `KVER=6.6.70 ./build.sh`.
Para pen USB/disco real: instala o `solar-os.bzImage` com GRUB/syslinux (o initramfs já vai dentro do kernel).

## Comandos do shell Solar
help, ls, cat, echo, cd, pwd, uname, uptime, mem, clear, reboot, poweroff

## Estrutura
- `init.c`  – PID 1: monta /proc /sys /dev, banner, shell
- `build.sh` – kernel + initramfs embutido
- `run.sh`  – teste em QEMU

## ISO
    sudo apt install grub-pc-bin grub-efi-amd64-bin xorriso mtools
    ./make-iso.sh   # corre o build.sh se necessário e gera solar-os.iso
    qemu-system-x86_64 -cdrom solar-os.iso -m 256M

## Sem Linux/WSL (Windows)
Envia esta pasta para um repositório GitHub. Em **Actions → Build Solar OS ISO → Run workflow**,
espera uns minutos e descarrega o artefacto `solar-os-iso`. Corre a ISO no VirtualBox/VMware/QEMU.

## Solar Desktop (ambiente gráfico próprio)
`desktop.c` desenha diretamente em `/dev/fb0` (sem X11/Wayland/GNOME): fundo de pôr do sol, ícones,
janelas arrastáveis, terminal, info do sistema, menu Solar, barra de tarefas e relógio.
O `/init` lança-o automaticamente; se não houver framebuffer cai no shell de texto (`gui` volta a lançar).

Atalhos: `Ctrl+T` novo terminal, `Esc` fecha o menu.

Correr no QEMU (Windows/Scoop), com rato absoluto:
    qemu-system-x86_64 -cdrom solar-os.iso -m 256M -device qemu-xhci -device usb-tablet -accel whpx
