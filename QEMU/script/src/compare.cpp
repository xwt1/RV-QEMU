#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

// 用于存储每一列的数据
struct ColumnData {
    std::vector<std::string> values;
};

// 用于比对两个列的数据并生成报告
void compareColumns(const ColumnData& column1, const ColumnData& column2, int columnIndex, std::ofstream& reportFile) {
    bool hasDifference = false; // 标志是否存在不同数值

    for (size_t i = 0; i < column1.values.size(); ++i) {
        if (column1.values[i] != column2.values[i]) {
            hasDifference = true;
            break;
        }
    }

    if (hasDifference) {
        reportFile << "------------------------------------------------------------" << std::endl;
        reportFile << "第" << columnIndex <<"条load/store类指令不一样"<< std::endl;
        for (size_t i = 0; i < column1.values.size(); ++i) {
            // if (column1.values[i] != column2.values[i]) {
                if(i == 0){
                    reportFile << "第一个可执行程序指令所处基本块为 " << column1.values[i] << std::endl;
                    reportFile << "第二个可执行程序指令所处基本块为 " << column2.values[i] << std::endl;                  
                }
                if(i == 1){
                    reportFile << "第一个可执行程序指令所处地址为 " << column1.values[i] << std::endl;
                    reportFile << "第二个可执行程序指令所处地址为 " << column2.values[i] << std::endl; 
                }
                if(i == 2){
                    reportFile << "第一个可执行程序指令种类为 " << column1.values[i] << std::endl;
                    reportFile << "第二个可执行程序指令种类为 " << column2.values[i] << std::endl; 
                }
                if(i == 3){
                    reportFile << "第一个可执行程序指令加载/存储的值为 " << column1.values[i] << std::endl;
                    reportFile << "第二个可执行程序指令加载/存储的值为 " << column2.values[i] << std::endl; 
                }

            // }
        }
        reportFile << "------------------------------------------------------------" << std::endl;
        reportFile << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        std::cerr << "Usage: " << argv[0] << " file1.csv file2.csv [output_report_file]" << std::endl;
        return 1;
    }

    std::ifstream file1(argv[1]);
    std::ifstream file2(argv[2]);
    std::ofstream reportFile;

    if (!file1.is_open() || !file2.is_open()) {
        std::cerr << "Failed to open one or both files." << std::endl;
        return 1;
    }

    if (argc == 4) {
        reportFile.open(argv[3]);
    } else {
        reportFile.open("compare.report");
    }

    if (!reportFile.is_open()) {
        std::cerr << "Failed to create/open the report file." << std::endl;
        return 1;
    }

    std::vector<ColumnData> data1, data2;
    std::string line;

    // 读取第一个文件的数据
    while (std::getline(file1, line)) {
        std::stringstream ss(line);
        std::string cell;
        ColumnData column;
        while (std::getline(ss, cell, ',')) {
            column.values.push_back(cell);
        }
        data1.push_back(column);
    }

    // 读取第二个文件的数据
    while (std::getline(file2, line)) {
        std::stringstream ss(line);
        std::string cell;
        ColumnData column;
        while (std::getline(ss, cell, ',')) {
            column.values.push_back(cell);
        }
        data2.push_back(column);
    }

    if (data1.size() > data2.size()) {
        std::cout << "看起来"<<argv[1] <<"生成的Load/store类型的指令要比"<<argv[2]<<"多" << std::endl;
    }else{
        if(data1.size() < data2.size()){
            std::cout << "看起来"<<argv[1] <<"生成的Load/store类型的指令要比"<<argv[2]<<"少" << std::endl;
        }
    }
    // 比对每一列并生成报告
    for (size_t i = 0; i < data1.size(); ++i) {
        if (data1[i].values.size() != data2[i].values.size()) {
            std::cout << "比较结束" << std::endl;
            return 0;
        }
        compareColumns(data1[i], data2[i], i + 1, reportFile);
    }
    std::cout << "Comparison complete." << std::endl;

    file1.close();
    file2.close();
    reportFile.close();

    return 0;
}
