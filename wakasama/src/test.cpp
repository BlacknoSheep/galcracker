#include "./include/head.h"

using namespace std;

std::vector<BYTE> RAW = {0x98, 0x42, 0x9E, 0x6B, 0x78, 0x55, 0xD2,
                         0xA0, 0x9E, 0x32, 0x52, 0xE7, 0xE0, 0xBC,
                         0x42, 0x9E, 0x6B, 0x0E, 0x68, 0x9E, 0x6B};

int main() {
  FileInfo fi(RAW, "script.dat");
  cout << std::uppercase << std::hex << "crc_low=" << fi.crc_low
       << "\nlistkey=" << fi.listkey << "\noffset=" << fi.offset
       << "\nsize=" << fi.size << "\nunpacked_size=" << fi.unpacked_size
       << endl;
  return 0;
}