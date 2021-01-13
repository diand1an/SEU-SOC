//
//	Created by diand1an on 2021/01/10.
//

#include <iostream>
#include <fstream>
#include "coe.h"

COE::COE(const string &coe_String, int current_Offset)
	:coeString(coe_String), currentOffset(current_Offset) {}

COE::COE() {}

const string &COE::getCoeString() const {
	return coeString;
}


void COE::setCoeString(const string &coe_string) {
	COE::coeString = coe_string;
}

int COE::getCurrentOffset()const {
	return currentOffset;
}

void COE::setCurrentOffset(int current_offset) {
	COE::currentOffset = current_offset;
}

unsigned int COE::getStartAddress() const {
	return startAddress;
}

void COE::setStartAddress(unsigned int start_address) {
	COE::startAddress = start_address;
}
