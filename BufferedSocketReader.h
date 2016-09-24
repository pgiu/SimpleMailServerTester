/* 
 * File:   BufferedSocketReader.h
 * Author: pablo
 *
 * Created on September 24, 2016, 10:31 AM
 */

#ifndef BUFFEREDSOCKETREADER_H
#define	BUFFEREDSOCKETREADER_H
#define MAX_BUFFER_SIZE 1000

class BufferedSocketReader {
public:
	BufferedSocketReader(int fd);
	BufferedSocketReader(const BufferedSocketReader& orig);

	int readOneByte(char *);

	bool isConnectionClosedByRemoteHost() const {
		return connectionClosedByRemoteHost;
	}

	bool isError() const {
		return error;
	}


	virtual ~BufferedSocketReader();
private:
	// File descriptor from where to read
	int fd;
	// Here we hold our bytes
	char circularBuffer[MAX_BUFFER_SIZE];
	int readPointer;
	int writePointer;
	// This is set to true when the recv function returns 0 in size
	bool connectionClosedByRemoteHost;
	// This is set to true when the recv function returns a -1
	bool error;
};

#endif	/* BUFFEREDSOCKETREADER_H */

