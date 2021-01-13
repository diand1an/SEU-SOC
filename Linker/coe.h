#pragma once
//
//	Created by diand1an on 2021/01/10.
//
#ifndef LINKER_COE_H
#define LINKER_COE_H

#include <string>

using namespace std;

class COE {
public:
	COE(const string &coeString, int currentOffset);
	COE();

	const string &getCoeString() const;

	void setCoeString(const string &coe_string);

	int getCurrentOffset()const;

	void setCurrentOffset(int current_offset);

	unsigned int getStartAddress() const;

	void setStartAddress(unsigned int startAddress);

private:
	string coeString;
	int currentOffset;
	unsigned int startAddress;
};

#endif