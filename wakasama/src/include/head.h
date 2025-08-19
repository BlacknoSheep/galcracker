#pragma once

#include "assembly.h"
#include "crc64.h"
#include "utils.h"
#include <fstream>
#include <unordered_map>

const DWORD M1 = 0x8B6A4E5F; // 用于计算文件数量的固定值
const DWORD M2 = 0xBABA18A9; // 用于解密encrypt_type==6的文件

// 根据crc64的低高位计算出一个用于索引的key
inline DWORD getListkey(DWORD low, DWORD high) {
  return ((high << 6) + low + (high >> 2)) ^ high;
}

// 根据字符串计算出一个用于索引的key
DWORD getListkey(const std::vector<BYTE> &key) {
  auto [low, high] = crc64(key);
  return getListkey(low, high);
}

class FileInfo {
public:
  std::vector<BYTE> raw; // 从dat文件中读入的原始数据,长度为21字节

  // exe程序利用读入的原始数据初步计算并保存于数据结构的信息
  // 共21B
  DWORD crc_low, crc_high; // 8B，key（文件相对路径）的哈希值
  DWORD encrypt_type; // 1B，加密类型：0未加密；1加密；5加密；6加密&压缩
  DWORD offset; // 4B，文件偏移量，可能被key加密
  DWORD size;   // 4B，文件字节数，可能被key加密
  DWORD unpacked_size; // 4B，解包后的文件字节数，文件未压缩则此字段无用

  DWORD listkey; // 4B，根据密钥字符串的crc64值计算得到，用于索引

  std::string datfilename; // 文件来源于哪个dat文件

  // 密钥，游戏使用的是打包时文件的相对路径进行的加密
  std::vector<BYTE> key;

  FileInfo() : raw(std::vector<BYTE>(21)) {}

  explicit FileInfo(const std::vector<BYTE> &buffer,
                    const std::string &datfilename)
      : FileInfo() {
    if (buffer.size() != 21)
      errExit("初始化FileInfo所需字节数：21，但是传入字节数：" +
              std::to_string(buffer.size()));
    raw = buffer;
    this->datfilename = datfilename;

    crc_low = readDwordFromBuffer(raw, 0);
    crc_high = readDwordFromBuffer(raw, 4);

    listkey = getListkey(crc_low, crc_high);

    encrypt_type = raw[8];
    encrypt_type ^= crc_low & 0xFF;

    offset = readDwordFromBuffer(raw, 9);
    offset ^= crc_low ^ M1;

    size = readDwordFromBuffer(raw, 13);
    size ^= crc_low;

    unpacked_size = readDwordFromBuffer(raw, 17);
    unpacked_size ^= crc_low;
  }

  // 使用密钥更新文件信息
  void updateWithKey(const std::vector<BYTE> &key) {
    this->key = key;
    size_t n = key.size();
    offset = key[n >> 1] ^ offset;
    size = key[n >> 2] ^ size;
  }
};

std::vector<FileInfo> getFileInfosFromDat(const std::string &datfilepath) {
  std::ifstream fs(datfilepath, std::ios::binary);
  if (!fs)
    errExit("文件打开失败！");

  DWORD d;
  std::string datfilename = getFilenameFromPath(datfilepath);

  // 检查文件头
  fs.read(reinterpret_cast<char *>(&d), sizeof(d));
  if (d != 0x31564341)
    errExit("不支持的文件格式！");

  // 获取文件数量
  fs.read(reinterpret_cast<char *>(&d), sizeof(d));
  int cnt_files = d ^ M1;

  // 读入每个文件的信息
  std::vector<FileInfo> fileinfos;
  std::vector<BYTE> buffer(21);
  for (int i = 0; i < cnt_files; ++i) {
    fs.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
    fileinfos.push_back(FileInfo(buffer, datfilename));
  }

  fs.close();
  return fileinfos;
}

std::unordered_map<DWORD, std::vector<BYTE>>
getListkeyToKey(std::vector<std::vector<BYTE>> &keys) {
  std::unordered_map<DWORD, std::vector<BYTE>> mp;
  for (auto &key : keys) {
    mp[getListkey(key)] = key;
  }
  return mp;
}

// 解密函数encrypt_type==1或5的数据
void decrypt1(std::vector<BYTE> &buffer, const std::vector<BYTE> &key) {
  size_t key_size = key.size();
  if (key_size == 0)
    return;
  size_t part_size = buffer.size() / key_size;
  for (size_t i = 0; i < key_size - 1; ++i) {
    for (size_t j = 0; j < part_size; ++j) {
      size_t index = i * part_size + j;
      buffer[index] ^= key[i];
    }
  }
}

// 解密encrypt_type==6的数据，并用zlib解压
void decrypt6(std::vector<BYTE> &buffer, DWORD crc_low) {
  for (int i = 0; i + 4 <= buffer.size(); i += 4) {
    DWORD &d = *reinterpret_cast<DWORD *>(buffer.data() + i);
    d ^= crc_low ^ M2;
  }
}

void decrypt(std::vector<BYTE> &buffer, const FileInfo &fi) {
  switch (fi.encrypt_type) {
  case 1:
  case 5:
    decrypt1(buffer, fi.key);
    break;
  case 6: {
    decrypt6(buffer, fi.crc_low);
    std::vector<BYTE> decompressed_buffer(fi.unpacked_size);
    decompress_zlib(buffer, decompressed_buffer);
    buffer = decompressed_buffer;
    break;
  }
  default:
    break;
  }
}