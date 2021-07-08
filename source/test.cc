#include "argparse.hpp"
#include "nlohmann/json.hpp"
#include "blastemInstance.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
 printf("Testing...\n");

 blastemInstance blastem(argc, argv, "libblastem.so", false);

 printf("Blastem created.\n");

 size_t step = 0;
 while(1)
 {
  printf("Step %lu\n", step++);
  printf("-------------------------------------\n");
  blastem.playFrame();
  blastem.printState();
  getchar();
 }
}
