#!/bin/bash

# 检查是否提供了至少两个参数
if [ $# -lt 2 ]; then
  echo "用法: $0 参数1 参数2 [参数3]"
  exit 1
fi

# 提取输入参数
param1="$1"
param2="$2"
param3="$3"

# 运行qemu-riscv64，并传递参数1和参数2
qemu-riscv64 "$param1" 
qemu-riscv64 "$param2"

# 生成.csv文件名
output_csv1="${param1}.csv"
output_csv2="${param2}.csv"

# 运行compare程序，并传递生成的.csv文件名作为参数
if [ -n "$param3" ]; then
  ./script/bin/compare "$output_csv1" "$output_csv2" "$param3"
else
  ./script/bin/compare "$output_csv1" "$output_csv2"
fi

# 完成
echo "脚本执行完成"
