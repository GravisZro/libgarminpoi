#include <sys/types.h>
#include <sys/stat.h>

#include <list>
#include <string>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cassert>

#include <simplified/simple_sqlite.h>

int main(int argc, char* argv[])
{

  // enable automatic flushing
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  std::clog << std::unitbuf;

  return EXIT_SUCCESS;
}
