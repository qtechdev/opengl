
#include <iostream>
#include <fstream>
#include <string>
#include <optional>

#include <cstdlib>

#include "file_io.hpp"

std::vector<xdg::path> split_dirs(std::string s) {
  std::vector<xdg::path> dirs;

  std::size_t pos = 0;
  std::string dir;

  while ((pos = s.find(":")) != std::string::npos) {
    dir = s.substr(0, pos);
    dirs.push_back(dir);
    s.erase(0, pos + 1);
  }

  dirs.push_back(s);

  return dirs;
}

xdg::base xdg::get_base_directories() {
  xdg::base base_dirs;
  base_dirs.home = std::getenv("HOME");

  char *xdg_data_home = std::getenv("XDG_DATA_HOME");
  if (xdg_data_home == nullptr) {
    base_dirs.xdg_data_home = base_dirs.home / ".local" / "share";
  } else {
    base_dirs.xdg_data_home = xdg_data_home;
  }

  char *xdg_config_home = std::getenv("XDG_CONFIG_HOME");
  if (xdg_config_home == nullptr) {
    base_dirs.xdg_config_home = base_dirs.home / ".config";
  } else {
    base_dirs.xdg_config_home = xdg_config_home;
  }

  char *xdg_data_dirs = std::getenv("XDG_DATA_DIRS");
  if (xdg_data_dirs == nullptr) {
    base_dirs.xdg_data_dirs = {"/usr/local/share", "/usr/share"};
  } else {
    base_dirs.xdg_data_dirs = split_dirs(xdg_data_dirs);
  }

  char *xdg_config_dirs = std::getenv("XDG_CONFIG_DIRS");
  if (xdg_config_dirs == nullptr) {
    base_dirs.xdg_config_dirs = {"/etc/xdg"};
  } else {
    base_dirs.xdg_config_dirs = split_dirs(xdg_config_dirs);
  }

  char *xdg_cache_home = std::getenv("XDG_CACHE_HOME");
  if (xdg_cache_home == nullptr) {
    base_dirs.xdg_cache_home = base_dirs.home / ".cache";
  } else {
    base_dirs.xdg_cache_home = xdg_cache_home;
  }

  char *xdg_runtime_dir = std::getenv("XDG_RUNTIME_DIR");
  if (xdg_runtime_dir == nullptr) {
    // create runtime dir or throw
  } else {
    // make sure directory is valid
    base_dirs.xdg_runtime_dir = xdg_runtime_dir;
  }

  return base_dirs;
}

std::optional<xdg::path> xdg::get_data_path(
  const base &b, const std::string &name, const path &p, const bool create
) {
  path home_path = b.xdg_data_home / name / p;
  if (fs::is_regular_file(home_path)) {
    return home_path;
  }

  if (create) {
    if (!fs::exists(home_path)) {
      fs::create_directories(home_path.parent_path());
      std::ofstream(home_path);
    }

    return home_path;
  }

  for (const auto &dir : b.xdg_data_dirs) {
    path system_path = dir / name / p;
    if (fs::is_regular_file(system_path)) {
      return system_path;
    }
  }

  path cwd_path = path("./data") / p;
  if (fs::is_regular_file(cwd_path)) {
    return cwd_path;
  }

  return {};
}

std::optional<std::string> fio::read(const xdg::path &p) {
  std::ifstream ifs(p);

  if (ifs) {
    ifs.seekg(0, std::ios::end);
    std::size_t filesize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::string data;
    data.resize(filesize);
    ifs.read(&data[0], filesize);

    return data;
  }

  return {};
}

bool fio::write(const xdg::path &p, const std::string &data, const bool trunc) {
  std::ofstream ofs;

  if (trunc) {
    ofs.open(p, std::ios::out | std::ios::trunc);
  } else {
    ofs.open(p, std::ios::out | std::ios::app);
  }

  if (ofs) {
    ofs.write(data.c_str(), data.size());

    return true;
  }

  return false;
}

fio::log_stream_f::log_stream_f(const std::string &s, const bool no_buf) {
  if (no_buf) { ofs.rdbuf()->pubsetbuf(0, 0); }
  ofs.open(s, std::ios::out | std::ios::app);

  auto now = std::chrono::system_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  ofs << std::string(79, '=') << "\n";
  ofs << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "\n";
}
