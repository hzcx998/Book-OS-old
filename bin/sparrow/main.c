#include "cli.h"

int main(int argc, const char** argv) {
   if (argc == 1) {
      runCli();
   } else {
      runFile(argv[1]);
   }
   return 0;
}
