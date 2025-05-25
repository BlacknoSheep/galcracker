#include "./include/head.h"

int main(int argc, char *argv[]) {
  // 读取密钥文件
  std::ifstream fs("./keys.txt");
  std::vector<std::string> keys;
  std::string line;
  while (std::getline(fs, line))
    keys.emplace_back(line);
  std::unordered_map<DWORD, std::string> hashkey_to_key = getHashkeyToKey(keys);
  fs.close();

  std::string datfilepath;
  if (argc < 2) {
    std::cout << "请输入dat文件路径：";
    std::cin >> datfilepath;
  } else
    datfilepath = argv[1];
  for (char &c : datfilepath) {
    if (c == '\\')
      c = '/';
  }
  std::string outdir = getDirectoryFromPath(datfilepath) + "/out";

  // 读入文件信息
  std::vector<FileInfo> fileinfos = getFileInfosFromDat(datfilepath);

  // 读入并解密文件
  fs.open(datfilepath, std::ios::binary);
  if (!fs)
    errExit("文件打开失败！");
  int cnt_done = 0; // 成功解密的文件数
  int idx = 0;      // 文件序号
  std::string filename;
  for (FileInfo &fi : fileinfos) {
    ++idx;
    std::string key;

    if (hashkey_to_key.count(fi.listkey)) {
      key = hashkey_to_key[fi.listkey];
      filename = key;
    } else
      filename = fi.datfilename + "/" + DWORDToHexString(fi.crc_low) + "_" +
                 DWORDToHexString(fi.crc_high) + '_' +
                 DWORDToHexString(fi.encrypt_type);

    // 未加密，根据文件头判断应该是avi
    if (fi.encrypt_type == 0) {
      if (key.empty())
        filename += ".avi";
    }
    // 普通加密
    else if (fi.encrypt_type == 1 || fi.encrypt_type == 5) {
      // 没有key则无法解密，跳过
      if (key.empty())
        continue;
      fi.updateWithKey(key);
    }
    // 6类加密，不需要key
    else if (fi.encrypt_type == 6) {
      if (key.empty())
        filename = filename + ".txt";
    }
    // 跳过其他未知类型
    else
      continue;

    std::vector<BYTE> buffer(fi.size);
    readFile(fs, fi.offset, fi.size, buffer);
    decrypt(buffer, fi);
    saveFile(buffer, buffer.size(), outdir + '/' + filename);
    ++cnt_done;
  }
  std::cout << "共读入了" << fileinfos.size() << "个文件" << std::endl;
  std::cout << "成功提取" << cnt_done << "个文件" << std::endl;

  fs.close();
  system("pause");
  return 0;
}