#ifndef LOG_FILE_H_
#define LOG_FILE_H_


#include <string>
#include <fstream>


namespace base {


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


}  // namespace base


#endif  // LOG_FILE_H_
