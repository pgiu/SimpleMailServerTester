/* 
 * File:   ConnectionInfo.h
 * Author: pablo
 *
 * Created on September 23, 2016, 10:29 PM
 */

#ifndef CONNECTIONINFO_H
#define	CONNECTIONINFO_H
#define MAX_BUFFER_SIZE 1000

class ConnectionInfo {
public:
	ConnectionInfo();
	ConnectionInfo(const ConnectionInfo& orig);
	virtual ~ConnectionInfo();

	char* getBuffer() {
		return buffer;
	}

	int getBufferSize() {
		return MAX_BUFFER_SIZE;
	}

	int getIndex() {
		return index;
	}

	void setIndex(int in) {
		index = in;
	}
private:
	char buffer[MAX_BUFFER_SIZE];
	// This is the index to the next valid character to read. 
	// It starts from -1 and goes up to MAX_BUFFER_SIZE
	char index;
};

#endif	/* CONNECTIONINFO_H */

