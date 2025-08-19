"""
# 以二进制形式逐行读取，然后去重
"""

inp = "keys_utf-8.log"
outp = "keys.txt"

seen = set()
with open(inp, "rb") as fin, open(outp, "wb") as fout:
    for raw in fin:                      # raw 是 bytes，包含原始行尾（可能是 \r\n 或 \n）
        key = raw.rstrip(b"\r\n")        # 仅用于判重的 key，去掉 CR/LF，避免同一内容不同换行的不一致
        if key not in seen:
            seen.add(key)
            fout.write(raw)              # 原样写回，字节不变（含最初遇到的行尾风格）
