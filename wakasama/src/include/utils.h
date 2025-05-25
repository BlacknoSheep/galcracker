#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>
#include <zlib.h>

// 从缓冲区指定偏移量处读入一个DWORD
inline DWORD readDwordFromBuffer(const std::vector<BYTE> &buffer,
                                 size_t offset) {
  return *reinterpret_cast<const DWORD *>(buffer.data() + offset);
}

inline void errExit(const std::string &msg) {
  std::cerr << msg << std::endl;
  system("pause");
  exit(1);
}

// DWORD转16进制字符串
std::string DWORDToHexString(DWORD value, bool uppercase = true,
                             bool with0x = false) {
  std::stringstream ss;
  if (uppercase)
    ss << std::uppercase;
  if (with0x)
    ss << "0x";
  ss << std::hex << std::setw(8) << std::setfill('0') << value;
  return ss.str();
}

// 递归创建目录
void createDirectories(const std::string &path) {
  std::string cmd = "mkdir \"" + path + "\" >nul 2>nul";
  system(cmd.c_str());
}

inline std::string getFilenameFromPath(const std::string &file_path) {
  size_t pos = file_path.find_last_of("/\\");
  if (pos != std::string::npos) {
    return file_path.substr(pos + 1);
  }
  return file_path; // 如果没有找到路径分隔符，返回原字符串
}

inline std::string getDirectoryFromPath(const std::string &file_path) {
  size_t pos = file_path.find_last_of("/\\");
  if (pos != std::string::npos) {
    return file_path.substr(0, pos);
  }
  return ""; // 如果没有找到路径分隔符，返回空字符串
}

// 将数据读入缓冲区
void readFile(std::ifstream &fs, size_t offset, size_t size,
              std::vector<BYTE> &buffer) {
  if (size > buffer.size())
    errExit("读入缓冲区太小！");
  fs.seekg(offset);
  fs.read(reinterpret_cast<char *>(buffer.data()), size);
  // std::cerr << std::hex << "offset=" << offset << ", size=" << size <<
  // std::dec
  //           << std::endl;
  if (fs.gcount() != size)
    errExit("文件读入失败！");
}

// 将buffer的前size个字节写入到文件
void saveFile(const std::vector<BYTE> &buffer, size_t size,
              const std::string &filepath) {
  // 创建输出目录
  createDirectories(getDirectoryFromPath(filepath));
  std::ofstream fs(filepath, std::ios::binary);
  if (!fs)
    errExit("创建输出文件失败！");
  fs.write(reinterpret_cast<const char *>(buffer.data()), size);
  fs.close();
}

bool decompress_zlib(const std::vector<BYTE> &input,
                     std::vector<BYTE> &output) {
  uLongf uncompressedSize =
      output.size() > 0 ? output.size() : input.size() * 5;
  output.resize(uncompressedSize);

  while (true) {
    int ret = uncompress(output.data(), &uncompressedSize, input.data(),
                         input.size());
    if (ret == Z_BUF_ERROR) {
      // 输出缓冲区过小，翻倍
      uncompressedSize *= 2;
      output.resize(uncompressedSize);
    } else {
      if (ret != Z_OK) {
        std::cerr << "zlib解压文件失败！" << std::endl;
        return false;
      }
      break;
    }
  }

  // 修正最终解压后的数据大小
  output.resize(uncompressedSize);
  return true;
}