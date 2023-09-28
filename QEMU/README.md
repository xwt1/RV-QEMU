# QEMU文档

## 1. 环境说明

- 主要使用的编译环境：
    - 构建描述工具：meson v0.53.2
    - 构建任务执行工具：GNU Make 4.3
    - 语言环境：C
    - 操作系统：ubuntu:22.04
    - QEMU版本：v8.0.0.4

## 2. 使用方法

1. 首先配置好环境：
    
    ```bash
    apt install \
    						python3 \
    						pip \
    						ninja-build \
    						libglib2.0-dev \
    						libpixman-1-dev \
    						git \
    						flex \
    						bison  -y
    apt install ninja-build						
    pip install -i https://pypi.tuna.tsinghua.edu.cn/simple pip -U
    pip config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple
    pip3 install Sphinx
    pip3 install sphinx_rtd_theme
    ```
    
2. 其次构建工具，假设目前已经进入到qemu文件夹中：
    
    ```bash
    ./configure
    make -j{nproc}
    ```
    
3. 运行脚本：
    
    ```bash
    cd ..   #退回到QEMU文件夹下
    ./script/compare.sh filepath1 filepath2 [filepath3] 
    #filepath1和filepath2是两个要比较的文件,filepath3可选，代表打印报告文件的路径
    #例子:
    ./script/compare.sh ./test/topology_direct.o ./test/topology_indirect.o ./test/compare.logs
    #执行后会生成./test/compare.logs,其中就可以看到运行过程中指令的不同
    ```