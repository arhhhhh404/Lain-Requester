#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "darknet_launcher.h"

void tor_launcher(const char *tor_path, const char *torrc_path) {
    pid_t pid = fork();

    if (pid < 0) {
      perror("[!]> fork");
      return;
    }

    if (pid == 0) {
      setsid();
      execl(tor_path, "tor", "-f", torrc_path, NULL);
      perror("[-]> execl tor");
      exit(1);
    }

    printf("[+]> Tor start (PID = %d)\n", pid);
    sleep(5);
}

void i2p_launcher(const char *i2p_path, const char *i2pconfig_path) {
    pid_t pid = fork();

    if (pid < 0) {
      perror("[!]> fork");
      return;
    }

    if (pid == 0) {
      setsid();
      execl(i2p_path, "i2pd", "--conf", i2pconfig_path, NULL);
      perror("[-]> execl i2pd");
      exit(1);
    }

    printf("[+]> i2p start (PID = %d)\n", pid);
    sleep(5);
}
