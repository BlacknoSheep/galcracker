# 解包游戏「和香様の座する世界」

## 使用方法

```shell
$ g++ -g ./src/main.cpp -o main.exe -lz -fexec-charset=GBK
```

然后将每个【script.dat】或【arc.dat】文件拖入main.exe即可，提取出的文件保存在out文件夹中。

语音，BGM，CG等文件需要key才能解密，这里的key是文件封包时的相对路径。

keys.txt只包含部分key。完整key可以从【script.dat】解包出的txt文件中提取出，或是hook游戏提取。