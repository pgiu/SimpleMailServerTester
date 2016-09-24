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

	
	

	int getBufferSize() {
		return MAX_BUFFER_SIZE;
	}

	int getLastRead() {
		return lastRead;
	}

	void setLastRead(int in) {
		this->lastRead = in;
	}
	
	int getSize() const {
		return size;
	}

	void setSize(int size) {
		this->size = size;
	}

private:
	char buffer[MAX_BUFFER_SIZE];
	// This is the index to the next valid character to read. 
	// It starts from -1 and goes up to MAX_BUFFER_SIZE
	int lastRead;
	int size;
};

#endif	/* CONNECTIONINFO_H */

