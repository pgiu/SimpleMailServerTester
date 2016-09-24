/* 
 * File:   BufferedSocketReader.cpp
 * Author: pablo
 * 
 * Created on September 24, 2016, 10:31 AM
 */

#include "BufferedSocketReader.h"
#include "ConnectionInfo.h"
#include <sys/socket.h>
#include <string.h>

#include <iostream>

using namespace std;
char inputBuffer[MAX_BUFFER_SIZE];

BufferedSocketReader::BufferedSocketReader(int fd) {
	this->fd = fd;
	this->readPointer = 0;
	this->writePointer = 0;
	this->connectionClosedByRemoteHost = false;
	this->error = false;
}

BufferedSocketReader::BufferedSocketReader(const BufferedSocketReader& orig) {
}

BufferedSocketReader::~BufferedSocketReader() {
}

int BufferedSocketReader::readOneByte(char* c) {
	// This is the return value. If ret=1 then everything is ok, -1 or 0 symbolizes an error
	char ret = 1;
	// In case readPointer is valid and it's not pointing to the last appended 
	// place, we can just return the value in the buffer.
	if (readPointer != -1 && readPointer != writePointer) {
		*c = circularBuffer[readPointer];
		readPointer = (readPointer + 1) % MAX_BUFFER_SIZE;
	} else {

		int size = recv(fd, inputBuffer, MAX_BUFFER_SIZE, 0);

		if (size == 0) {
			connectionClosedByRemoteHost = true;
			ret = 0;

		} else if (size == -1) {
			error = true;
			ret = -1;

		} else if (size < (MAX_BUFFER_SIZE - writePointer)) {
			strncpy(circularBuffer + writePointer, inputBuffer, size);
			writePointer += size;
			// Enhance this
			*c = circularBuffer[readPointer];
			readPointer = (readPointer + 1) % MAX_BUFFER_SIZE;

		} else if (size < MAX_BUFFER_SIZE) {
			// It doesn't fit
			int firstCopySize = MAX_BUFFER_SIZE - writePointer - 1;
			int secondCopySize = size - firstCopySize;
			strncpy(circularBuffer + writePointer , inputBuffer, firstCopySize);
			strncpy(circularBuffer, inputBuffer + firstCopySize, secondCopySize);
			writePointer = (writePointer + secondCopySize) % MAX_BUFFER_SIZE;
			// Enhance this
			*c = circularBuffer[readPointer];
			readPointer = (readPointer + 1) % MAX_BUFFER_SIZE;

		} else {
			cout << "ERROR! We can't copy so many bytes. Please rewrite this to throw an exception. Danke sehr!";
		}
	}
	return ret;
}