/*
 * The MIT License
 *
 * Copyright 2019 Thinh Pham.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "SimpleSerial.h"

SimpleSerial::~SimpleSerial() {
	close();
}

void SimpleSerial::open(char* comPort, DWORD baudrate) {
	close();
	comHandler = CreateFileA(static_cast<LPCSTR>(comPort),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (comHandler == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
			printf("Warning: Handle was not attached. Reason: %s not available\n", comPort);
	} else {
		DCB dcbSerialParams/* = { 0 }*/;
		if (!GetCommState(comHandler, &dcbSerialParams)) {
			printf("Warning: Failed to get current serial params");
		} else {
			dcbSerialParams.BaudRate = baudrate;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.StopBits = ONESTOPBIT;
			dcbSerialParams.Parity = NOPARITY;
			dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;
			if (!SetCommState(comHandler, &dcbSerialParams)) {
				printf("Warning: could not set serial port params\n");
			} else {
				comConnected = true;
				PurgeComm(comHandler, PURGE_RXCLEAR | PURGE_TXCLEAR);
			}
		}
	}
}

//void SimpleSerial::CustomSyntax(string syntax_type) {
//
//	ifstream syntaxfile_exist("syntax_config.txt");
//
//	if (!syntaxfile_exist) {
//		ofstream syntaxfile;
//		syntaxfile.open("syntax_config.txt");
//
//		if (syntaxfile) {
//			syntaxfile << "json { }\n";
//			syntaxfile << "greater_less_than < >\n";
//			syntaxfile.close();
//		}
//	}
//
//	syntaxfile_exist.close();
//
//	ifstream syntaxfile_in;
//	syntaxfile_in.open("syntax_config.txt");
//
//	string line;
//	bool found = false;
//
//	if (syntaxfile_in.is_open()) {
//
//		while (syntaxfile_in) {
//			syntaxfile_in >> syntax_name_ >> front_delimiter_ >> end_delimiter_;
//			getline(syntaxfile_in, line);
//
//			if (syntax_name_ == syntax_type) {
//				found = true;
//				break;
//			}
//		}
//
//		syntaxfile_in.close();
//
//		if (!found) {
//			syntax_name_ = "";
//			front_delimiter_ = ' ';
//			end_delimiter_ = ' ';
//			printf("Warning: Could not find delimiters, may cause problems!\n");
//		}
//	}
//	else
//		printf("Warning: No file open");
//}

//string SimpleSerial::ReadSerialPort(int reply_wait_time, string syntax_type) {
//
//	DWORD bytes_read;
//	char inc_msg[1];
//	string complete_inc_msg;
//	bool began = false;
//
//	CustomSyntax(syntax_type);
//
//	unsigned long start_time = time(nullptr);
//
//	ClearCommError(comHandler, &comErrors, &comStatus);
//
//	while ((time(nullptr) - start_time) < reply_wait_time) {
//
//		if (comStatus.cbInQue > 0) {
//
//			if (ReadFile(comHandler, inc_msg, 1, &bytes_read, NULL)) {
//
//				if (inc_msg[0] == front_delimiter_ || began) {
//					began = true;
//
//					if (inc_msg[0] == end_delimiter_)
//						return complete_inc_msg;
//
//					if (inc_msg[0] != front_delimiter_)
//						complete_inc_msg.append(inc_msg, 1);
//				}
//			}
//			else
//				return "Warning: Failed to receive data.\n";
//		}
//	}
//	return complete_inc_msg;
//}

int SimpleSerial::write(uint8_t *data, uint32_t length) {
	if (comConnected) {
		DWORD written;
		if (!WriteFile(comHandler, (void*)data, length, &written, NULL)) {
			ClearCommError(comHandler, &comErrors, &comStatus);
		}
		return written;
	}
	return -1;
}

void SimpleSerial::close() {
	if (comConnected) {
		comConnected = false;
		CloseHandle(comHandler);
	}
}

bool SimpleSerial::isConnected() {
	return comConnected;
}