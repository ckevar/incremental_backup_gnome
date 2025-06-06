#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// compilation:
// gcc monodirectional_backup.c -o monodirectional_backup

#define NUM_INPUTS_REQ      3
#define SRC_DIR             1
#define DST_DIR             2
#define MAX_FILENAME_SZ     256
#define CMD_GIO_CP_MAX_SZ   1024

int cmpdir(char *src_name, char *dst_name);

void copyfile(char *src_name, char *dst_name) {
  char cmd[CMD_GIO_CP_MAX_SZ] = {0};
  int ret;

  printf("copying %s > %s ... ", src_name, dst_name);

  ret = snprintf(cmd, 
      CMD_GIO_CP_MAX_SZ, 
      "gio copy %s \"%s\"", 
      src_name, 
      dst_name);

  if (ret < 0 || ret >= CMD_GIO_CP_MAX_SZ) {
    printf("FAIL, filename too long.\n");
    return;
  }
  
  system(cmd);

  printf("DONE.\n");
}

void cmpfile(char *src_name, char *dst_name) {
  struct stat src_sb, dst_sb;
  int ret;

  ret = stat(src_name, &src_sb);
  if(-1 == ret) {
    fprintf(stderr, "\n  [Err:stat] skipping %s \n", src_name);
    return;
  }

  if (S_IFREG != (src_sb.st_mode & S_IFMT))
    return;

  ret = stat(dst_name, &dst_sb);
  if(-1 == ret) {
    copyfile(src_name, dst_name);
    return;
  }

  if (difftime(src_sb.st_mtime, dst_sb.st_mtime) <= 0)
    return;

  copyfile(src_name, dst_name);

}

int join_dir(char *full_name, char *parent_dir_name, char *child_dir_name) {
  int ret;

  ret = snprintf(
      full_name, 
      MAX_FILENAME_SZ, 
      "%s/%s", 
      parent_dir_name, 
      child_dir_name
      );

  if(ret < 0 || ret >= MAX_FILENAME_SZ) {
    fprintf(stderr, "\n  [Err:overflow] name %s is too long\n", full_name);
    return 1;
  } 
  return 0;
}

int check_mkdir(char *src_name, char *dst_name) {
  struct stat dir_sb;

  if(0 == stat(dst_name, &dir_sb))
    return 0;

  return mkdir(dst_name, 0700);
}

int check_in(char *src_name, char *dst_name, struct dirent *diroi) {
  char full_src_name[MAX_FILENAME_SZ], full_dst_name[MAX_FILENAME_SZ];
  int ret;

  if('.' == *diroi->d_name) // Ignoring hidden files and upwards directory
    return 0;

  ret = join_dir(full_src_name, src_name, diroi->d_name);
  if (1 == ret) return 1;

  ret = join_dir(full_dst_name, dst_name, diroi->d_name);
  if (1 == ret) return 1;

  switch(diroi->d_type) {
  case DT_DIR:

    ret = check_mkdir(full_src_name, full_dst_name);
    if(ret < 0) {
      printf("FAIL: skipping directory %s\n", full_src_name);
      return 0;
    }

    cmpdir(full_src_name, full_dst_name);
    break;

  case DT_REG:
    cmpfile(full_src_name, full_dst_name);
    break;
  }
  return 0;
}

int cmpdir(char *src_name, char *dst_name) {
  DIR *root_dir;
  struct dirent *dir;

  root_dir = opendir(src_name);
  if (!root_dir) 
    return 0;

  while(NULL != (dir = readdir(root_dir))) {
    if(1 == check_in(src_name, dst_name, dir)) {
      return 1;
    }
  }

  closedir(root_dir);

  return 1;
}

int main(int argc, char *argv[]) {
  int ret;

  if (argc < NUM_INPUTS_REQ) {
    fprintf(stderr, "\n  [Err:required] source dir and destination dir\n");
    return 1;
  }

  ret = cmpdir(argv[SRC_DIR], argv[DST_DIR]);
  if (!ret) {
    fprintf(stderr, "\n  [Err:not found] %s\n", argv[SRC_DIR]);
  }

}
