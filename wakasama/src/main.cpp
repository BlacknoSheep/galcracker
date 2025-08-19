#include "./include/head.h"

int main(int argc, char *argv[]) {
  // 读取密钥文件
  std::vector<std::vector<BYTE>> keys = readFileLinesAsBytes("./keys.txt");
  std::unordered_map<DWORD, std::vector<BYTE>> listkey_to_key =
      getListkeyToKey(keys);

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

  std::string outdir = "./out";

  // 读入文件信息
  std::vector<FileInfo> fileinfos = getFileInfosFromDat(datfilepath);

  // 统计0~6类和其他类加密文件的数量和成功解密的数量
  std::vector<std::vector<int>> stats(8, std::vector<int>(2));

  // 读入并解密文件
  std::ifstream fs;
  fs.open(datfilepath, std::ios::binary);
  if (!fs)
    errExit("文件打开失败！");
  int cnt_done = 0; // 成功解密的文件数
  std::string filepath;
  for (FileInfo &fi : fileinfos) {
    std::vector<BYTE> key;

    if (listkey_to_key.count(fi.listkey)) {
      key = listkey_to_key[fi.listkey];
      filepath = std::string(key.begin(), key.end());
    } else
      filepath = fi.datfilename + "/" + DWORDToHexString(fi.crc_low) + "_" +
                 DWORDToHexString(fi.crc_high) + '_' +
                 DWORDToHexString(fi.encrypt_type);

    // 未加密，根据文件头判断应该是avi
    if (fi.encrypt_type == 0) {
      ++stats[fi.encrypt_type][0];
      ++stats[fi.encrypt_type][1];

      if (key.empty())
        filepath += ".avi";
    }
    // 普通加密
    else if (fi.encrypt_type == 1 || fi.encrypt_type == 5) {
      ++stats[fi.encrypt_type][0];

      // 没有key则无法解密，跳过
      if (key.empty())
        continue;
      fi.updateWithKey(key);

      ++stats[fi.encrypt_type][1];
    }
    // 6类加密，不需要key
    else if (fi.encrypt_type == 6) {
      ++stats[fi.encrypt_type][0];
      ++stats[fi.encrypt_type][1];

      if (key.empty())
        filepath = filepath + ".txt";
    }
    // 跳过其他未知类型
    else {
      ++stats[7][0];
      continue;
    }

    std::vector<BYTE> buffer(fi.size);
    readFile(fs, fi.offset, fi.size, buffer);
    decrypt(buffer, fi);
    saveFile(buffer, buffer.size(), outdir + '/' + filepath);
    ++cnt_done;
  }

  std::cout << "共读入了" << fileinfos.size() << "个文件" << std::endl;
  std::cout << "成功提取" << cnt_done << "个文件" << std::endl;

  std::cout << "统计信息：" << std::endl;
  std::cout << "加密类型\t总数\t成功解密" << std ::endl;
  for (int i = 0; i < stats.size(); ++i) {
    std::cout << i << "\t\t" << stats[i][0] << "\t" << stats[i][1] << std::endl;
  }

  fs.close();
  system("pause");
  return 0;
}