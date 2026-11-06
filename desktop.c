/* Solar Desktop - ambiente gráfico próprio do Solar OS (framebuffer + evdev, C puro, sem libs) */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <linux/fb.h>
#include <linux/input.h>

/* ---------- fonte 8x8 (ASCII 32..126, bit0 = pixel da esquerda) ---------- */
static const unsigned char font[95][8] = {
{0,0,0,0,0,0,0,0},{0x18,0x3C,0x3C,0x18,0x18,0,0x18,0},{0x36,0x36,0,0,0,0,0,0},
{0x36,0x36,0x7F,0x36,0x7F,0x36,0x36,0},{0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0},
{0,0x63,0x33,0x18,0x0C,0x66,0x63,0},{0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0},
{0x06,0x06,0x03,0,0,0,0,0},{0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0},
{0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0},{0,0x66,0x3C,0xFF,0x3C,0x66,0,0},
{0,0x0C,0x0C,0x3F,0x0C,0x0C,0,0},{0,0,0,0,0,0x0C,0x0C,0x06},{0,0,0,0x3F,0,0,0,0},
{0,0,0,0,0,0x0C,0x0C,0},{0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0},
{0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0},{0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0},
{0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0},{0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0},
{0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0},{0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0},
{0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0},{0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0},
{0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0},{0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0},
{0,0x0C,0x0C,0,0,0x0C,0x0C,0},{0,0x0C,0x0C,0,0,0x0C,0x0C,0x06},
{0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0},{0,0,0x3F,0,0,0x3F,0,0},
{0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0},{0x1E,0x33,0x30,0x18,0x0C,0,0x0C,0},
{0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0},{0x0C,0x1E,0x33,0x33,0x3F,0x33,0x33,0},
{0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0},{0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0},
{0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0},{0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0},
{0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0},{0x3C,0x66,0x03,0x03,0x73,0x66,0x7C,0},
{0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0},{0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0},
{0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0},{0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0},
{0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0},{0x63,0x77,0x7F,0x7F,0x6B,0x63,0x63,0},
{0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0},{0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0},
{0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0},{0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0},
{0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0},{0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0},
{0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0},{0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0},
{0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0},{0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0},
{0x63,0x63,0x36,0x1C,0x1C,0x36,0x63,0},{0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0},
{0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0},{0x1E,0x06,0x06,0x06,0x06,0x06,0x1E,0},
{0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0},{0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0},
{0x08,0x1C,0x36,0x63,0,0,0,0},{0,0,0,0,0,0,0,0xFF},{0x0C,0x0C,0x18,0,0,0,0,0},
{0,0,0x1E,0x30,0x3E,0x33,0x6E,0},{0x07,0x06,0x06,0x3E,0x66,0x66,0x3B,0},
{0,0,0x1E,0x33,0x03,0x33,0x1E,0},{0x38,0x30,0x30,0x3E,0x33,0x33,0x6E,0},
{0,0,0x1E,0x33,0x3F,0x03,0x1E,0},{0x1C,0x36,0x06,0x0F,0x06,0x06,0x0F,0},
{0,0,0x6E,0x33,0x33,0x3E,0x30,0x1F},{0x07,0x06,0x36,0x6E,0x66,0x66,0x67,0},
{0x0C,0,0x0E,0x0C,0x0C,0x0C,0x1E,0},{0x30,0,0x30,0x30,0x30,0x33,0x33,0x1E},
{0x07,0x06,0x66,0x36,0x1E,0x36,0x67,0},{0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0},
{0,0,0x33,0x7F,0x7F,0x6B,0x63,0},{0,0,0x1F,0x33,0x33,0x33,0x33,0},
{0,0,0x1E,0x33,0x33,0x33,0x1E,0},{0,0,0x3B,0x66,0x66,0x3E,0x06,0x0F},
{0,0,0x6E,0x33,0x33,0x3E,0x30,0x78},{0,0,0x3B,0x6E,0x66,0x06,0x0F,0},
{0,0,0x3E,0x03,0x1E,0x30,0x1F,0},{0x08,0x0C,0x3E,0x0C,0x0C,0x2C,0x18,0},
{0,0,0x33,0x33,0x33,0x33,0x6E,0},{0,0,0x33,0x33,0x33,0x1E,0x0C,0},
{0,0,0x63,0x6B,0x7F,0x7F,0x36,0},{0,0,0x63,0x36,0x1C,0x36,0x63,0},
{0,0,0x33,0x33,0x33,0x3E,0x30,0x1F},{0,0,0x3F,0x19,0x0C,0x26,0x3F,0},
{0x38,0x0C,0x0C,0x07,0x0C,0x0C,0x38,0},{0x18,0x18,0x18,0,0x18,0x18,0x18,0},
{0x07,0x0C,0x0C,0x38,0x0C,0x0C,0x07,0},{0x6E,0x3B,0,0,0,0,0,0}};

/* ---------- ecrã ---------- */
static int W, H, bpp;
static uint32_t *bb, *bgimg;
static uint8_t *fbmem;
static struct fb_var_screeninfo vi;
static struct fb_fix_screeninfo fi;

static void rect(int x, int y, int w, int h, uint32_t c) {
    if (x < 0) { w += x; x = 0; } if (y < 0) { h += y; y = 0; }
    if (x + w > W) w = W - x; if (y + h > H) h = H - y;
    for (int j = 0; j < h; j++) { uint32_t *p = bb + (y + j) * W + x; for (int i = 0; i < w; i++) p[i] = c; }
}
static void glyph(int x, int y, int ch, uint32_t c, int sc) {
    if (ch < 32 || ch > 126) return;
    const unsigned char *g = font[ch - 32];
    for (int r = 0; r < 8; r++) for (int b = 0; b < 8; b++)
        if (g[r] >> b & 1) rect(x + b * sc, y + r * sc, sc, sc, c);
}
static void text(int x, int y, const char *s, uint32_t c, int sc) {
    for (; *s; s++, x += 8 * sc) glyph(x, y, *s, c, sc);
}

static void make_background(void) {
    bgimg = malloc((size_t)W * H * 4);
    int cx = W / 2, cy = H * 62 / 100, R = H / 7;
    for (int y = 0; y < H; y++) {
        int t = y * 255 / (H - 1);                       /* azul noite -> laranja do pôr do sol */
        int r = 11 + (255 - 11) * t / 255 * t / 255, g = 16 + (140 - 16) * t / 255, b = 48 + (66 - 48) * t / 255;
        for (int x = 0; x < W; x++) {
            long dx = x - cx, dy = y - cy, d2 = dx * dx + dy * dy;
            int rr = r, gg = g, bb2 = b;
            if (d2 < (long)R * R) { rr = 255; gg = 214; bb2 = 102; }
            else if (d2 < (long)(R * 14 / 10) * (R * 14 / 10)) { rr = (rr + 255) / 2; gg = (gg + 190) / 2; bb2 = (bb2 + 90) / 2; }
            bgimg[y * W + x] = rr << 16 | gg << 8 | bb2;
        }
    }
}

static void present(void) {
    int ok = bpp == 32 && vi.red.offset == 16 && vi.green.offset == 8 && vi.blue.offset == 0;
    for (int y = 0; y < H; y++) {
        uint8_t *dst = fbmem + (size_t)y * fi.line_length;
        if (ok) { memcpy(dst, bb + y * W, W * 4); continue; }
        for (int x = 0; x < W; x++) {
            uint32_t c = bb[y * W + x], r = c >> 16 & 255, g = c >> 8 & 255, b = c & 255;
            uint32_t p = (r >> (8 - vi.red.length)) << vi.red.offset | (g >> (8 - vi.green.length)) << vi.green.offset |
                         (b >> (8 - vi.blue.length)) << vi.blue.offset;
            if (bpp == 32) ((uint32_t *)dst)[x] = p;
            else if (bpp == 16) ((uint16_t *)dst)[x] = p;
            else { dst[x * 3] = p; dst[x * 3 + 1] = p >> 8; dst[x * 3 + 2] = p >> 16; }
        }
    }
}

/* ---------- janelas ---------- */
#define TITLE 24
#define TB 32
#define TR 22
#define TC 38
enum { T_TERM, T_INFO, T_ABOUT };
typedef struct {
    int open, type, x, y, w, h; char title[24];
    char buf[TR][TC + 1]; int cx, cy, inlen; char in[128];
} Win;
static Win pool[8], *z[8]; static int nz;
static int mx, my, drag, dx0, dy0, menu, shift, ctrl;
static Win *dragw;

static void tnl(Win *t) {
    if (t->cy < TR - 1) t->cy++;
    else { memmove(t->buf[0], t->buf[1], (TR - 1) * (TC + 1)); memset(t->buf[TR - 1], 0, TC + 1); }
}
static void tputc(Win *t, char c) {
    if (c == '\t') c = ' ';
    if (c == '\n') { t->cx = 0; tnl(t); return; }
    if (c < 32 || c > 126) return;
    if (t->cx >= TC) { t->cx = 0; tnl(t); }
    t->buf[t->cy][t->cx++] = c;
}
static void tputs(Win *t, const char *s) { while (*s) tputc(t, *s++); }
static void prompt(Win *t) { tputs(t, "solar:# "); t->inlen = 0; }

static void run(Win *t, char *line) {
    char *cmd = strtok(line, " "), *arg = strtok(NULL, ""), tmp[256];
    if (!cmd) return;
    if (!strcmp(cmd, "help")) tputs(t, "help ls cat echo cd pwd uname\nuptime mem clear reboot poweroff\n");
    else if (!strcmp(cmd, "echo")) { tputs(t, arg ? arg : ""); tputc(t, '\n'); }
    else if (!strcmp(cmd, "pwd")) { tputs(t, getcwd(tmp, sizeof tmp) ? tmp : "?"); tputc(t, '\n'); }
    else if (!strcmp(cmd, "cd")) { if (chdir(arg ? arg : "/")) tputs(t, "cd: erro\n"); }
    else if (!strcmp(cmd, "uname")) { struct utsname u; uname(&u); snprintf(tmp, sizeof tmp, "%s %s %s\n", u.sysname, u.release, u.machine); tputs(t, tmp); }
    else if (!strcmp(cmd, "uptime")) { struct sysinfo s; sysinfo(&s); snprintf(tmp, sizeof tmp, "up %lds\n", s.uptime); tputs(t, tmp); }
    else if (!strcmp(cmd, "mem")) { struct sysinfo s; sysinfo(&s); snprintf(tmp, sizeof tmp, "RAM %lu/%lu MB livres\n", (unsigned long)(s.freeram * s.mem_unit >> 20), (unsigned long)(s.totalram * s.mem_unit >> 20)); tputs(t, tmp); }
    else if (!strcmp(cmd, "clear")) { memset(t->buf, 0, sizeof t->buf); t->cx = t->cy = 0; }
    else if (!strcmp(cmd, "ls")) {
        DIR *d = opendir(arg ? arg : "."); struct dirent *e;
        if (!d) { tputs(t, "ls: erro\n"); return; }
        while ((e = readdir(d))) { tputs(t, e->d_name); tputs(t, "  "); }
        closedir(d); tputc(t, '\n');
    } else if (!strcmp(cmd, "cat") && arg) {
        int fd = open(arg, O_RDONLY); char b[1024]; ssize_t n;
        if (fd < 0) { tputs(t, "cat: erro\n"); return; }
        if ((n = read(fd, b, sizeof b)) > 0) for (ssize_t i = 0; i < n; i++) tputc(t, b[i]);
        close(fd);
    } else if (!strcmp(cmd, "reboot")) { sync(); reboot(RB_AUTOBOOT); }
    else if (!strcmp(cmd, "poweroff")) { sync(); reboot(RB_POWER_OFF); }
    else { tputs(t, cmd); tputs(t, ": comando desconhecido\n"); }
}

static void raise_win(Win *w) {
    int i = 0; while (i < nz && z[i] != w) i++;
    if (i == nz) return;
    for (; i < nz - 1; i++) z[i] = z[i + 1];
    z[nz - 1] = w;
}
static void open_win(int type) {
    Win *w = NULL;
    if (type != T_TERM) for (int i = 0; i < 8; i++) if (pool[i].open && pool[i].type == type) { raise_win(&pool[i]); return; }
    for (int i = 0; i < 8; i++) if (!pool[i].open) { w = &pool[i]; break; }
    if (!w) return;
    memset(w, 0, sizeof *w); w->open = 1; w->type = type;
    int n = nz;
    w->x = 140 + n * 28; w->y = 40 + n * 28;
    if (type == T_TERM) { w->w = 8 + TC * 16; w->h = TITLE + 8 + TR * 16; strcpy(w->title, "Terminal"); tputs(w, "Solar OS terminal\n"); prompt(w); }
    else if (type == T_INFO) { w->w = 420; w->h = 200; strcpy(w->title, "Sistema"); }
    else { w->w = 440; w->h = 190; strcpy(w->title, "Sobre o Solar OS"); }
    if (w->x + w->w > W) w->x = W - w->w; if (w->y + w->h > H - TB) w->y = H - TB - w->h;
    if (w->x < 0) w->x = 0; if (w->y < 0) w->y = 0;
    z[nz++] = w;
}
static void close_win(Win *w) {
    int i = 0; while (i < nz && z[i] != w) i++;
    for (; i < nz - 1; i++) z[i] = z[i + 1];
    nz--; w->open = 0;
}

static const char *menu_items[] = { "Terminal", "Info do sistema", "Sobre o Solar OS", "Reiniciar", "Desligar" };
#define NMENU 5
#define MI_H 28
static const char *icon_names[] = { "Terminal", "Sistema", "Sobre" };
static const char icon_letters[] = { '>', 'i', 'S' };

static void draw_window(Win *w, int focused) {
    char b[160]; struct utsname u; struct sysinfo s;
    rect(w->x - 1, w->y - 1, w->w + 2, w->h + 2, 0x000000);
    rect(w->x, w->y, w->w, TITLE, focused ? 0xF2A93B : 0x8A6A3A);
    text(w->x + 8, w->y + 4, w->title, 0x2A1800, 2);
    rect(w->x + w->w - 22, w->y + 3, 18, 18, 0xC0392B);
    glyph(w->x + w->w - 21, w->y + 4, 'x', 0xFFFFFF, 2);
    rect(w->x, w->y + TITLE, w->w, w->h - TITLE, 0x14161C);
    int bx = w->x + 8, by = w->y + TITLE + 8;
    if (w->type == T_TERM) {
        for (int r = 0; r < TR; r++) text(w->x + 4, w->y + TITLE + 4 + r * 16, w->buf[r], 0x7CFC9A, 2);
        if (focused) rect(w->x + 4 + w->cx * 16, w->y + TITLE + 4 + w->cy * 16 + 12, 14, 3, 0x7CFC9A);
    } else if (w->type == T_INFO) {
        uname(&u); sysinfo(&s);
        snprintf(b, sizeof b, "Kernel : Linux %s", u.release); text(bx, by, b, 0xE8E8E8, 2);
        snprintf(b, sizeof b, "Arq.   : %s", u.machine); text(bx, by + 24, b, 0xE8E8E8, 2);
        snprintf(b, sizeof b, "Ecra   : %dx%d %dbpp", W, H, bpp); text(bx, by + 48, b, 0xE8E8E8, 2);
        snprintf(b, sizeof b, "RAM    : %lu/%lu MB livres", (unsigned long)(s.freeram * s.mem_unit >> 20), (unsigned long)(s.totalram * s.mem_unit >> 20));
        text(bx, by + 72, b, 0xE8E8E8, 2);
        snprintf(b, sizeof b, "Uptime : %ld s", s.uptime); text(bx, by + 96, b, 0xE8E8E8, 2);
    } else {
        text(bx, by, "Solar OS", 0xFFD166, 3);
        text(bx, by + 36, "Kernel Linux + userspace propria.", 0xE8E8E8, 2);
        text(bx, by + 60, "Sem BusyBox, sem glibc, sem distro.", 0xE8E8E8, 2);
        text(bx, by + 84, "Desktop em C, desenhado em /dev/fb0.", 0xE8E8E8, 2);
        text(bx, by + 116, "Ctrl+T abre um terminal.", 0x9AA0B0, 2);
    }
}

static void draw_cursor(void) {
    static const char *cur[] = {
        "X           ", "XX          ", "X.X         ", "X..X        ", "X...X       ", "X....X      ", "X.....X     ",
        "X......X    ", "X.......X   ", "X........X  ", "X.....XXXXX ", "X..X..X     ", "X.X X..X    ", "XX  X..X    ",
        "X    X..X   ", "     X..X   ", "      XX    " };
    for (int r = 0; r < 17; r++) for (int c = 0; c < 12; c++)
        if (cur[r][c] != ' ') rect(mx + c, my + r, 1, 1, cur[r][c] == 'X' ? 0x000000 : 0xFFFFFF);
}

static void draw(void) {
    struct sysinfo s; char b[32];
    memcpy(bb, bgimg, (size_t)W * H * 4);
    for (int i = 0; i < 3; i++) {                                   /* ícones */
        int x = 40, y = 24 + i * 100;
        rect(x, y, 56, 56, 0x20243A); rect(x + 2, y + 2, 52, 52, 0x2E3452);
        glyph(x + 12, y + 12, icon_letters[i], 0xFFD166, 4);
        text(x + 28 - (int)strlen(icon_names[i]) * 8, y + 62, icon_names[i], 0xFFFFFF, 2);
    }
    for (int i = 0; i < nz; i++) draw_window(z[i], i == nz - 1);
    rect(0, H - TB, W, TB, 0x14161C); rect(0, H - TB, W, 1, 0xF2A93B);  /* barra de tarefas */
    rect(0, H - TB, 130, TB, menu ? 0xFFC15E : 0xF2A93B);
    text(10, H - TB + 8, "* Solar", 0x2A1800, 2);
    for (int i = 0; i < nz; i++) {
        int x = 140 + i * 130;
        rect(x, H - TB + 4, 124, TB - 8, i == nz - 1 ? 0x3A4068 : 0x23263A);
        char t[12]; snprintf(t, sizeof t, "%.9s", z[i]->title); text(x + 6, H - TB + 8, t, 0xFFFFFF, 2);
    }
    sysinfo(&s);
    snprintf(b, sizeof b, "%02ld:%02ld:%02ld", s.uptime / 3600, s.uptime / 60 % 60, s.uptime % 60);
    text(W - 8 * 2 * 8 - 12, H - TB + 8, b, 0xFFD166, 2);
    if (menu) {
        int mh = NMENU * MI_H, my0 = H - TB - mh;
        rect(0, my0 - 1, 284, mh + 1, 0xF2A93B); rect(1, my0, 282, mh, 0x1C1F2E);
        for (int i = 0; i < NMENU; i++) {
            int hov = mx < 284 && my >= my0 + i * MI_H && my < my0 + (i + 1) * MI_H;
            if (hov) rect(1, my0 + i * MI_H, 282, MI_H, 0x3A4068);
            text(12, my0 + i * MI_H + 6, menu_items[i], 0xFFFFFF, 2);
        }
    }
    draw_cursor();
}

static void click(void) {
    if (menu) {
        int mh = NMENU * MI_H, my0 = H - TB - mh;
        menu = 0;
        if (mx < 284 && my >= my0 && my < H - TB) {
            switch ((my - my0) / MI_H) {
                case 0: open_win(T_TERM); break; case 1: open_win(T_INFO); break; case 2: open_win(T_ABOUT); break;
                case 3: sync(); reboot(RB_AUTOBOOT); break; case 4: sync(); reboot(RB_POWER_OFF); break;
            }
            return;
        }
        if (my >= H - TB && mx < 130) return;
    }
    if (my >= H - TB) {
        if (mx < 130) { menu = 1; return; }
        int i = (mx - 140) / 130; if (mx >= 140 && i < nz) raise_win(z[i]);
        return;
    }
    for (int i = nz - 1; i >= 0; i--) {
        Win *w = z[i];
        if (mx >= w->x && mx < w->x + w->w && my >= w->y && my < w->y + w->h) {
            raise_win(w);
            if (my < w->y + TITLE) {
                if (mx >= w->x + w->w - 22 && mx < w->x + w->w - 4) close_win(w);
                else { drag = 1; dragw = w; dx0 = mx - w->x; dy0 = my - w->y; }
            }
            return;
        }
    }
    for (int i = 0; i < 3; i++) if (mx >= 40 && mx < 96 && my >= 24 + i * 100 && my < 24 + i * 100 + 80) { open_win(i); return; }
}

static const char kn[] = "\0\0" "1234567890-=" "\b\t" "qwertyuiop[]" "\n\0" "asdfghjkl;'`" "\0\\" "zxcvbnm,./" "\0*\0 ";
static const char ks[] = "\0\0" "!@#$%^&*()_+" "\b\t" "QWERTYUIOP{}" "\n\0" "ASDFGHJKL:\"~" "\0|" "ZXCVBNM<>?" "\0*\0 ";

static void key(int code) {
    if (code == KEY_ESC) { menu = 0; return; }
    if (ctrl && code == KEY_T) { open_win(T_TERM); return; }
    if (!nz || z[nz - 1]->type != T_TERM || code >= 58) return;
    Win *t = z[nz - 1]; char c = shift ? ks[code] : kn[code];
    if (!c) return;
    if (c == '\n') { tputc(t, '\n'); t->in[t->inlen] = 0; run(t, t->in); prompt(t); }
    else if (c == '\b') { if (t->inlen > 0) { t->inlen--; if (t->cx > 0) { t->cx--; t->buf[t->cy][t->cx] = 0; } } }
    else if (t->inlen < 100) { t->in[t->inlen++] = c; tputc(t, c); }
}

int main(void) {
    int fb = open("/dev/fb0", O_RDWR);
    if (fb < 0) { fputs("solar-desktop: sem /dev/fb0\n", stderr); return 2; }
    ioctl(fb, FBIOGET_VSCREENINFO, &vi); ioctl(fb, FBIOGET_FSCREENINFO, &fi);
    W = vi.xres; H = vi.yres; bpp = vi.bits_per_pixel;
    fbmem = mmap(0, (size_t)fi.line_length * vi.yres, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if (fbmem == MAP_FAILED) return 2;
    bb = malloc((size_t)W * H * 4); make_background();
    mx = W / 2; my = H / 2;
    open_win(T_ABOUT);

    int fds[16], amx[16], amy[16], nfd = 0; for (int i = 0; i < 16; i++) fds[i] = -1;
    for (;;) {
        for (int i = 0; i < 16; i++) if (fds[i] < 0) {                /* (re)descobrir teclados/ratos */
            char p[32]; snprintf(p, sizeof p, "/dev/input/event%d", i);
            fds[i] = open(p, O_RDONLY | O_NONBLOCK);
            if (fds[i] >= 0) {
                struct input_absinfo a;
                amx[i] = ioctl(fds[i], EVIOCGABS(ABS_X), &a) == 0 ? a.maximum : 0;
                amy[i] = ioctl(fds[i], EVIOCGABS(ABS_Y), &a) == 0 ? a.maximum : 0;
            }
        }
        draw(); present();
        struct pollfd pf[16]; nfd = 0; int map[16];
        for (int i = 0; i < 16; i++) if (fds[i] >= 0) { pf[nfd].fd = fds[i]; pf[nfd].events = POLLIN; map[nfd++] = i; }
        poll(pf, nfd, 1000);
        for (int k = 0; k < nfd; k++) {
            struct input_event e; int i = map[k];
            if (pf[k].revents & (POLLERR | POLLHUP)) { close(fds[i]); fds[i] = -1; continue; }
            while (read(fds[i], &e, sizeof e) == sizeof e) {
                if (e.type == EV_REL) {
                    if (e.code == REL_X) mx += e.value; else if (e.code == REL_Y) my += e.value;
                } else if (e.type == EV_ABS) {
                    if (e.code == ABS_X && amx[i]) mx = (long)e.value * (W - 1) / amx[i];
                    else if (e.code == ABS_Y && amy[i]) my = (long)e.value * (H - 1) / amy[i];
                } else if (e.type == EV_KEY) {
                    if (e.code == BTN_LEFT) { if (e.value == 1) click(); else if (e.value == 0) drag = 0; }
                    else if (e.code == KEY_LEFTSHIFT || e.code == KEY_RIGHTSHIFT) shift = e.value;
                    else if (e.code == KEY_LEFTCTRL || e.code == KEY_RIGHTCTRL) ctrl = e.value;
                    else if (e.value >= 1) key(e.code);
                }
                if (mx < 0) mx = 0; if (my < 0) my = 0; if (mx >= W) mx = W - 1; if (my >= H) my = H - 1;
                if (drag && dragw) { dragw->x = mx - dx0; dragw->y = my - dy0; if (dragw->y < 0) dragw->y = 0; }
            }
        }
    }
}
