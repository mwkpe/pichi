#ifndef LOGFILE_H_
#define LOGFILE_H_


#include <string>
#include <fstream>


namespace logging {


class LogFile
{
protected:
  LogFile() = default;
  explicit LogFile(const std::string& filename);

public:
  virtual ~LogFile();
  LogFile(const LogFile&) = delete;
  LogFile& operator=(const LogFile&) = delete;

  bool is_open() const { return fs_.is_open(); }

protected:
  bool open_(const std::string& filename);
  std::ofstream fs_;
};


}  // namespace logging


#endif  // LOGFILE_H_
