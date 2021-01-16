//
//	Created by diand1an on 2021/01/13.
//
#include <iostream>
#include <fstream>
#include <iomanip>
#include <bitset>
#include <sstream>
#include <deque>
#include <regex>
#include <map>
#include "coe.cpp"
using namespace std;

unsigned int biosStaAddress = 0x700;	//十六进制,0x700为bios文件的首地址
unsigned int startAddress = 0x1100;		//十六进制,0x1100为其他区域的首地址
unsigned int curAddress = startAddress;	//打开当前coe文件时的起始地址，处理不是bios对应文件时才改变

deque<COE> lineOfCOE;//存储coe文件,每行对应一个COE对象
map<int, string >mapFileStaAdr;//存每个coe文件名和对应起始地址

int RowCounter(string inputFileName);//统计文件行数
void adjustCOEFile(string File);//读入coe文件
void depositCOEFile(string File);//写入coe文件
int binToi(const string& num);//将二进制字符串转换成十进制数字
string add(const string& base, int addNum, int opLength, int type);//传入任意进制的字符串，进行加减，返回十六进制的字符串
int hexToInt(string hex);//十六进制字符串转int

void generateMap(void);//第一遍遍历coe文件，生成mapFileStaAdr
void link(void);//第二遍遍历coe文件，链接

int main() {
	//生成起始地址和文件名的映射关系
	generateMap();

	//链接
	link();
	return 0;
}

//统计文件行数
int RowCounter(string FileName) {
	fstream ReadFile;
	int n = 0;
	string tmp;
	ReadFile.open(FileName + "_rom.coe", ios::in);//以只读方式读取文件
	if (ReadFile.fail()) {//文件打开失败
		cerr << "文件打开失败！\n";
	}
	else {
		while (getline(ReadFile, tmp, '\n')) {
			n++;
		}
		ReadFile.close();
		return n;
	}
}

//读入coe文件，并且存入deque
void adjustCOEFile(string File) {
	ifstream inputFile(File + "_rom.coe");

	if (inputFile.is_open()) {
		string lineOfCode;	//coe文件的一行
		int lineTag = 0;	//coe文件中的行号,与currentOffset一一对应

		while (getline(inputFile, lineOfCode)) {
			lineTag++;
			if (lineTag > 2) {
				if (File == "bios"&&lineTag >= 258) {
					cerr << "bios_rom.coe文件过大！";
				}
				if (lineOfCode.size() == 0) { continue; }//读入空行，跳过

				//如果行中有#，则与地址有关
				else if (regex_search(lineOfCode, regex("#"))) {

					/*寻找label对应的地址
					  将操作码与地址拼起来
					  转十六进制
					  存入deque
					*/

					string labelTag = "";//label标号名
					int labelLine = -1;//_label.txt文件中找到的的行号
					labelTag = lineOfCode.substr(lineOfCode.find("#") + 1);
					int sizeOfLabel = 0;
					sizeOfLabel = labelTag.size() - 1;
					labelTag = labelTag.substr(0, sizeOfLabel);

					//以各文件名为模式，匹配labelTag对应的txt文件
					for (auto &fileName : mapFileStaAdr) {
						if (regex_search(labelTag, regex(fileName.second))) {

							//在_label.txt中寻找label对应的行号
							ifstream inputLableFile(fileName.first + "_label.txt");
							if (inputLableFile.is_open()) {
								while (!inputLableFile.eof()) {
									string tempLabel = "";
									inputLableFile >> tempLabel;
									if (tempLabel == labelTag) { inputLableFile >> labelLine; break; }
								}
								if (labelLine == -1)
									cerr << "找不到label对应行号！\n";
							}
							else {
								cerr << "打不开 " << File << "_label.txt!\n";
							}

							inputLableFile.close();

							int opLength = 0;//操作码长度
							opLength = lineOfCode.size() - labelTag.size();

							unsigned int labelPhyAdr = 0;//label的绝对地址
							labelPhyAdr = fileName.first + (labelLine * 4);//相对地址=（行号-2）*4
																			//绝对地址=起始地址+相对地址

							//将操作码与绝对地址拼起来，并转十六进制
							string op = lineOfCode.substr(0, opLength - 2);
							string hex = add(op, labelPhyAdr, opLength, 1);

							//存入deque
							COE CoeFile;
							if (File == "bios") {
								CoeFile.setStartAddress(biosStaAddress);
							}
							else {
								CoeFile.setStartAddress(curAddress);
							}
							CoeFile.setCurrentOffset((lineTag - 2) * 4);
							CoeFile.setCoeString(hex);
							lineOfCOE.push_back(CoeFile);//存入deque
						}
					}
				}
				//读入的是指令，与地址无关
				else {
					//每条指令占4个字节
					COE CoeFile;
					if (File == "bios") {
						CoeFile.setStartAddress(biosStaAddress);
					}
					else {
						CoeFile.setStartAddress(curAddress);
					}
					CoeFile.setCurrentOffset((lineTag - 2) * 4);
					CoeFile.setCoeString(lineOfCode.substr(0, 8));
					lineOfCOE.push_back(CoeFile);//存入deque
				}
			}
		}

		if (File != "bios") {
			curAddress = curAddress + (lineTag - 2) * 4;//读入该coe文件后的地址
		}

	}
	else {
		cerr << "打不开 " << File << "_rom.coe!\n";
	}
	inputFile.close();

}
//写入coe文件
void depositCOEFile(string Filepath) {
	ofstream outputFile(Filepath + "_link.coe");
	outputFile << "MEMORY_INITIALIZATION_RADIX=16;\nMEMORY_INITIALIZATION_VECTOR=\n";

	//读入的为bios_rom.coe，则用00000000填补空隙
	if (Filepath == "bios")
	{
		int i = 0;
		for (; i < lineOfCOE.size(); i++) {
			outputFile << setw(8) << setfill('0') << lineOfCOE[i].getCoeString() << ",\n";
		}
		if (lineOfCOE.size() < 256) {
			for (; i < 256; i++)outputFile << "00000000,\n";
		}
	}
	else {
		for (int i = 0; i < lineOfCOE.size(); i++) {
			outputFile << setw(8) << setfill('0') << lineOfCOE[i].getCoeString() << ",\n";
		}
	}
	lineOfCOE.clear();//清空deque，以便下次存入
}

//将二进制字符串转换成十进制数字
int binToi(const string& num) {
	int result = 0;
	for (char i : num) {
		result = result * 2 + i - '0';
	}
	return result;
}

/*传入二进制的字符串，进行拼接（type=1）or相加（type=0），返回十六进制的字符串
base:字符串类型的操作码
addNum:int类型的地址
opLength:操作码的长度
type:拼接（type==1）or相加（type==0）
*/
string add(const string& base, int addNum, int opLength, int type) {
	long nValue = 0;
	stringstream ioss;//定义字符串流
	string s_temp;//存放转化后的字符

	nValue = binToi(base.substr(0, base.size()));
	//type==1,执行拼接功能；type==0，执行加法功能
	if (type == 1) {
		nValue = nValue << (32 - opLength);//二进制左移32-opLength位
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

//第一遍遍历coe文件，生成mapFileStaAdr
void generateMap(void) {
	unsigned int staAdr = startAddress;
	unsigned int curAdr = curAddress;

	cout << "输入待链接的coe文件名 ( 大小写敏感,输入“0”结束 ) ：\n";
	string coeFileName;
	cin >> coeFileName;
	while (coeFileName != "0") {
		if (coeFileName == "bios") {

			mapFileStaAdr.insert(pair<int, string >(biosStaAddress, coeFileName));
		}
		else {
			mapFileStaAdr.insert(pair<int, string >(curAdr, coeFileName));
			int Size = 0;//该文件所占的地址空间大小
			Size = (RowCounter(coeFileName) - 2) * 4;
			curAdr = curAdr + Size;
		}
		cin >> coeFileName;
	}
}

//链接
void link() {
	//生成每个coe文件对应的_link.coe文件
	for (auto& it : mapFileStaAdr) {
		string FileName = "";
		FileName = it.second;
		adjustCOEFile(FileName);
		depositCOEFile(FileName);
	}
	//把多个_link.coe文件合并到一个linked.coe文件
	ofstream oFile("linked.coe");
	oFile << "MEMORY_INITIALIZATION_RADIX=16;\nMEMORY_INITIALIZATION_VECTOR=\n";
	for (auto& it : mapFileStaAdr) {
		string FileName = "";
		FileName = it.second;
		ifstream iFile(FileName + "_link.coe");

		int i = 0;
		string line = "";
		while (!iFile.eof()&&iFile.peek()!=EOF) {
			i++;
			getline(iFile, line);
			//从coe文件的第三行开始
			if (i > 2) {
				oFile << line << "\n";
			}
			//iFile.get();
		}
		iFile.close();
	}
}