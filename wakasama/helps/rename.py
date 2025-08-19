"""
为提取出的脚本文件重命名
"""

import os
import re

# 指定你要操作的文件夹路径
folder_path = r''  # 改成你的实际目录

# 定义匹配模式
pattern = re.compile(r'^\*[A-Za-z0-9_]+')

for filename in os.listdir(folder_path):
    if filename.lower().endswith('.txt'):
        file_path = os.path.join(folder_path, filename)

        tag = None
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                for line in f:
                    line = line.strip()
                    if line:
                        match = pattern.match(line)
                        if match:
                            tag = match.group().lstrip('*')  # 去掉*
                        break
        except Exception as e:
            print(f"读取文件 {filename} 出错: {e}")
            continue

        # 文件关闭后再重命名
        if tag and not filename.startswith(tag + '_'):
            new_filename = f"{tag}_{filename}"
            new_file_path = os.path.join(folder_path, new_filename)
            try:
                os.rename(file_path, new_file_path)
                print(f"已重命名：{filename} -> {new_filename}")
            except Exception as e:
                print(f"重命名文件 {filename} 出错: {e}")
