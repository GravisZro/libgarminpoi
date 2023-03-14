#include <sys/types.h>
#include <sys/stat.h>

#include <list>
#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cassert>
#include <filesystem>

#include <simplified/simple_sqlite.h>

#include <record_types.h>
#include <parsers.h>


int main(int argc, char* argv[])
{
  if(argc < 2)
    return EXIT_FAILURE;

  // enable automatic flushing
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  std::clog << std::unitbuf;


  std::filesystem::path testdata(argv[1]);
  if(std::filesystem::exists(testdata) && std::filesystem::is_directory(testdata))
  {
    std::filesystem::directory_iterator dir(testdata);
    for(const auto& entry : dir)
    {
      std::ifstream file(entry.path(), std::ios::binary | std::ios::in);
      assert(file.is_open());

      std::cout << std::setfill('0') << std::hex;

      std::vector<garmin::any_record_t> records;
      while(file.good())
        file >> records.emplace_back();
      // ignore index if there is one
      /*
      if(file.fail())
      {
        file.clear();
        uint32le_t bytes = 0;
      }
      */

      file.close();
    }
  }


  return EXIT_SUCCESS;
}
