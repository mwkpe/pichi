#include "logfile.h"


#include <string>
#include <iostream>


logging::LogFile::LogFile(const std::string& filename)
{
  open_(filename);
}


logging::LogFile::~LogFile()
{
  // Stream will close itself
  if (fs_.is_open())
    std::cout << "File closed" << std::endl;
}


bool logging::LogFile::open_(const std::string& filename)
{
  fs_.open(filename);
  if (!is_open())
    std::cerr << "Could not open file!" << std::endl;
  else
    std::cout << "File opened" << std::endl;

  return is_open();
}
