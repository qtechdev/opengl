#ifndef __FILE_IO_HPP__
#define __FILE_IO_HPP__
// https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html

#include <filesystem>
#include <string>
#include <vector>
#include <optional>

namespace xdg {
  namespace fs = std::filesystem;
  using path = std::filesystem::path;

  struct base {
    path home;
    path xdg_data_home; // $HOME/.local/share
    path xdg_config_home; // $HOME/.config
    std::vector<path> xdg_data_dirs; // /usr/local/share/:/usr/share/
    std::vector<path> xdg_config_dirs; // /etc/xdg
    path xdg_cache_home; // $HOME/.cache
    path xdg_runtime_dir;
  };

  base get_base_directories();

  std::optional<path> get_data_path(
    const base &b, const std::string &name, const path &p,
    const bool create=false
  );
};

namespace fio {
  std::optional<std::string> read(const xdg::path &p);
  bool write(const xdg::path &p, const std::string &data, const bool trunc=false);

  class log_stream_f {
  public:
    log_stream_f(const std::string &s, const bool no_buf=false);
    template <typename T>
    std::ofstream &operator<<(const T t);
  private:
    std::ofstream ofs;
  };
};

template<typename T>
std::ofstream &fio::log_stream_f::operator<<(const T t) {
  ofs << t;

  return ofs;
}

#endif // __FILE_IO_HPP__
