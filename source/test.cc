#include "argparse.hpp"
#include "nlohmann/json.hpp"
#include "blastemInstance.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
 printf("Testing...\n");

 blastemInstance blastem("libblastem.so", false);
 printf("Blastem created.\n");
 blastem.initialize(argv[1], argv[3]);
 printf("Blastem initialized.\n");

 blastem.playFrame("L");
 uint8_t testState[_STATE_DATA_SIZE];
 blastem.saveState(testState);

 size_t step = 0;
 while(1)
 {
  printf("Step %lu\n", step++);
  printf("-------------------------------------\n");
  blastem.playFrame("L");
  blastem.printState();
  getchar();
  if (step % 10 == 5) blastem.loadState(testState);
 }
}
