import os

input_folder = r""
output_folder = r""

# 确保输出目录存在
os.makedirs(output_folder, exist_ok=True)

for filename in os.listdir(input_folder):
    if filename.endswith('.txt'):
        input_path = os.path.join(input_folder, filename)
        output_path = os.path.join(output_folder, filename)

        with open(input_path, 'r', encoding='shift_jis', errors='ignore') as f_in:
            content = f_in.read()

        with open(output_path, 'w', encoding='utf-8') as f_out:
            f_out.write(content)

print("转换完成！")
