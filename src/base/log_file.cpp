#include "log_file.h"


#include <string>
#include <iostream>


base::LogFile::LogFile(const std::string& filename)
{
  open_(filename);
}


base::LogFile::~LogFile()
{
  // Stream will close itself
  if (fs_.is_open())
    std::cout << "File closed" << std::endl;
}


bool base::LogFile::open_(const std::string& filename)
{
  fs_.open(filename);
  if (!is_open())
    std::cerr << "Could not open file!" << std::endl;
  else
    std::cout << "File opened: " << filename << std::endl;

  return is_open();
}
