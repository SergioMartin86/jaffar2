#include "argparse.hpp"
#include "nlohmann/json.hpp"
#include "blastemInstance.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
 printf("Testing...\n");

 blastemInstance blastem("libblastem.so", false);

 printf("Blastem created.\n");

 blastem.initialize();
 blastem.main(argc, argv);

 printf("Blastem initialized.\n");
}
