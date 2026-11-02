/* Solar OS - /init (PID 1) minimalista, o único programa de userspace */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>

static void banner_real(void) {
    struct utsname u; uname(&u);
    printf("\033[33m\n   \\   |   /\n    .-\"\"\"-.      \033[1mSolar OS\033[0m\033[33m\n"
           "-- /  SOL  \\ --  Linux %s (%s)\n    '-...-'      Escreve 'help' para comecar.\n"
           "   /   |   \\\033[0m\n\n", u.release, u.machine);
}

static void cat(const char *p) {
    int fd = open(p, O_RDONLY); char b[512]; ssize_t n;
    if (fd < 0) { printf("cat: %s: erro\n", p); return; }
    while ((n = read(fd, b, sizeof b)) > 0) write(1, b, n);
    close(fd);
}

static void ls(const char *p) {
    DIR *d = opendir(p ? p : "."); struct dirent *e;
    if (!d) { printf("ls: erro\n"); return; }
    while ((e = readdir(d))) printf("%s  ", e->d_name);
    puts(""); closedir(d);
}

int main(void) {
    char line[256];
    mount("proc", "/proc", "proc", 0, 0);
    mount("sysfs", "/sys", "sysfs", 0, 0);
    mount("devtmpfs", "/dev", "devtmpfs", 0, 0);
    sethostname("solar", 5);
    banner_real();

    for (;;) {
        printf("\033[33msolar\033[0m:# "); fflush(stdout);
        if (!fgets(line, sizeof line, stdin)) { sleep(1); clearerr(stdin); continue; }
        line[strcspn(line, "\r\n")] = 0;
        char *cmd = strtok(line, " "), *arg = strtok(NULL, "");
        if (!cmd) continue;
        if (!strcmp(cmd, "help"))
            puts("help  ls [dir]  cat <file>  echo <txt>  cd <dir>  pwd\n"
                 "uname  uptime  mem  clear  reboot  poweroff");
        else if (!strcmp(cmd, "ls")) ls(arg);
        else if (!strcmp(cmd, "cat") && arg) cat(arg);
        else if (!strcmp(cmd, "echo")) puts(arg ? arg : "");
        else if (!strcmp(cmd, "cd")) { if (chdir(arg ? arg : "/")) puts("cd: erro"); }
        else if (!strcmp(cmd, "pwd")) { char c[256]; puts(getcwd(c, sizeof c) ? c : "?"); }
        else if (!strcmp(cmd, "uname")) { struct utsname u; uname(&u); printf("%s %s %s %s\n", u.sysname, u.nodename, u.release, u.machine); }
        else if (!strcmp(cmd, "uptime")) { struct sysinfo s; sysinfo(&s); printf("up %lds\n", s.uptime); }
        else if (!strcmp(cmd, "mem")) { struct sysinfo s; sysinfo(&s); printf("RAM: %lu MB livres / %lu MB\n", s.freeram*s.mem_unit>>20, s.totalram*s.mem_unit>>20); }
        else if (!strcmp(cmd, "clear")) printf("\033[2J\033[H");
        else if (!strcmp(cmd, "reboot")) { sync(); reboot(RB_AUTOBOOT); }
        else if (!strcmp(cmd, "poweroff")) { sync(); reboot(RB_POWER_OFF); }
        else printf("%s: comando desconhecido\n", cmd);
    }
}
