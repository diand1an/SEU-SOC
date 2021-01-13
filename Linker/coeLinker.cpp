//
//	Created by diand1an 2021.01.13
//
#include <iostream>
#include <fstream>
#include <iomanip>
#include <bitset>
#include <sstream>
#include <vector>
#include <regex>
#include "coe.cpp"
using namespace std;

unsigned int startAddress = 100;//十进制
unsigned int curAddress = startAddress;//打开当前coe文件时的起始地址

vector<COE> lineOfCOE;//存储coe文件,每行对应一个COE对象


void inputCOEFile(string File);//读入coe文件
void outputCOEFile(string File);//写入coe文件
int binToi(const string& num);//将二进制字符串转换成十进制数字
string add(const string& base, int addNum, int opLength,int type);//传入任意进制的字符串，进行加减，返回十六进制的字符串
int hexToInt(string hex);//十六进制字符串转int

int main() {
	string coeFileName = "test";
	inputCOEFile(coeFileName);
	outputCOEFile(coeFileName);
	return 0;
}
//读入coe文件，并且存入vector
void inputCOEFile(string File) {
	ifstream inputFile(File + "_rom.coe");

	if (inputFile.is_open()) {
		string lineOfCode;//coe文件的一行
		int lineTag = 0;//coe文件中的行号,与currentOffset一一对应

		while (getline(inputFile, lineOfCode)) {
			lineTag++;
			if (lineTag > 2) {
				if (lineOfCode.size() == 0) { continue; }//读入空行，跳过

				//如果行中有#，则与地址有关
				else if (regex_search(lineOfCode, regex("#"))) {

					/*寻找label对应的地址
					  将操作码与地址拼起来
					  转十六进制
					  存入vector
					*/

					string labelTag = "";//标号名
					int labelLine = -1;//.label.txt文件中找到的的行号
					labelTag = lineOfCode.substr(lineOfCode.find("#") + 1);
					int sizeOfLabel = 0;
					sizeOfLabel = labelTag.size() - 1;
					labelTag = labelTag.substr(0, sizeOfLabel);

					//在_label.txt中寻找label对应的行号
					ifstream inputLableFile(File + "_label.txt");
					if (inputLableFile.is_open()) {
						while (!inputLableFile.eof()) {
							string tempLabel = "";
							inputLableFile >> tempLabel;
							if (tempLabel == labelTag) { inputLableFile >> labelLine; break; }
						}
						if (labelLine == -1)
							cerr << "找不到对应行号！\n";
					}
					else {
						cerr << "cannot open " << File << "_label.txt!\n";
					}
					inputLableFile.close();

					int opLength = 0;
					opLength = lineOfCode.size() - labelTag.size();

					//将操作码与地址拼起来，并转十六进制
					string op = lineOfCode.substr(0,opLength - 2);
					string hex = add(op, (labelLine) * 4,opLength, 1);//相对地址=（行号-2）*4

					//存入vector
					COE CoeFile;
					CoeFile.setStartAddress(curAddress);
					CoeFile.setCurrentOffset((lineTag - 2) * 4);
					CoeFile.setCoeString(hex);
					lineOfCOE.push_back(CoeFile);//存入vector
				}
				//读入的是指令，与地址无关
				else {
					//每条指令占4个字节
					COE CoeFile;
					CoeFile.setStartAddress(curAddress);
					CoeFile.setCurrentOffset((lineTag - 2) * 4);
					CoeFile.setCoeString(lineOfCode.substr(0, 8));
					lineOfCOE.push_back(CoeFile);//存入vector
				}
			}
		}
		
		curAddress = curAddress + (lineTag - 2) * 4;//读入该coe文件后的地址
	}
	else {
		cerr << "cannot open " << File << "_rom.ceo!\n";
	}
	inputFile.close();
}
//写入coe文件
void outputCOEFile(string Filepath) {
	ofstream outputFile(Filepath + "_link.coe");
	outputFile << "MEMORY_INITIALIZATION_RADIX=16;\nMEMORY_INITIALIZATION_VECTOR=\n";
	for (int i = 0; i < lineOfCOE.size(); i++) {
		outputFile << setw(8) << setfill('0') << lineOfCOE[i].getCoeString() << ",\n";
	}
}

//将二进制字符串转换成十进制数字
int binToi(const string& num) {
	int result = 0;
	for (char i : num) {
		result = result * 2 + i - '0';
	}
	return result;
}

/*传入2进制的字符串，进行拼接（type=1）or相加（type=0），返回十六进制的字符串
base:字符串类型的操作码
addNum:int类型的地址
opLength:操作码的长度
type:拼接（type==1）or相加（type==0）
*/
string add(const string& base, int addNum,int opLength, int type) {
	long nValue = 0;
	stringstream ioss;//定义字符串流
	string s_temp;//存放转化后的字符

	nValue = binToi(base.substr(0, base.size()));
	//type==1,执行拼接功能；type==0，执行加法功能
	if (type == 1){
		nValue = nValue << (32- opLength);//二进制左移32-opLength位
	}
	else if (type != 0) {
		cerr << "wrong type!\n";
		return 0;
	}
	nValue += addNum;
	char res[32];
	//转十六进制字符串
	sprintf_s(res, "%X", nValue);//以十六进制形式输出
	return res;
}

//十六进制字符串转int
int hexToInt(string hex) {
	if (!regex_match(hex, regex("[0-9A-F]{8}"))) {
		return atoi(hex.c_str());//atoi将字符串转化为int
	}
	int nValue;
	stringstream ss;
	ss << std::hex << hex;
	ss >> nValue;
	return nValue;
}
