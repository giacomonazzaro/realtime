#include "image_viewer.h"

int main(int num_args, const char *args[]) {
  // Welcome
  printf("Hello, %s!\n", __FILE__);
  run_image_viewer(args[1]);
}