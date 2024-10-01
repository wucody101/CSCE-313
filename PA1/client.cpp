/*
	Author of the starter code
    Yifan Ren
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 9/15/2024
	
	Please include your Name, UIN, and the date below
	Name: Cody Wu
	UIN: 133001517
	Date: 9/29/24
*/
#include "common.h"
#include "stdlib.h"
#include "FIFORequestChannel.h"
#include <sys/wait.h>

using namespace std;

// Task 2.1: Single Data Point transfer
// Base code already provides an implementation for this task
void requestDataPoint(int p, double t, int e, FIFORequestChannel &chan) {
    // char buf[MAX_MESSAGE];
	datamsg x(p, t, e);		// Request patient data point
	
    // memcpy(buf, &x, sizeof(datamsg));   // Can either copy datamsg into separate buffer then write buffer into pipe,
	chan.cwrite(&x, sizeof(datamsg));   // or just directly write datamsg into pipe
	double response;
	chan.cread(&response, sizeof(double));

	cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << response << endl;
}

// Task 2.2: 1000 Data Point transfer into received/x1.csv
// Request 1000 data points from server and write results into received/x1.csv with same file format as the patient {1-15}.csv files
void requestData(int p, FIFORequestChannel &chan) {
    // Open x1.csv file under ./received/
	// Can use any method for opening a file, ofstream is one method
	ofstream ofs("./received/x1.csv");

	// Iterate 1000 times
	double t = 0.0;
	for (int i = 0; i < 1000; i++) {
		// Write time into x1.csv (Time is 0.004 second deviations)
		ofs << t << ",";

		datamsg msg1 = datamsg(p, t, 1); // Request ecg1
		// Write ecg1 datamsg into pipe
		chan.cwrite(&msg1, sizeof(datamsg));
		// Read response for ecg1 from pipe
		double ecg1;
		chan.cread(&ecg1, sizeof(double));
        // Write ecg1 value into x1.csv
		ofs << ecg1 << ",";
		
		datamsg msg2 = datamsg(p, t, 2); // Request ecg2
		// Write ecg2 datamsg into pipe
		chan.cwrite(&msg2, sizeof(datamsg));
        // Read response for ecg2 from pipe
		double ecg2;
		chan.cread(&ecg2, sizeof(double));
        // Write ecg2 value into x1.csv
		ofs << ecg2 << endl;

        // Increment time
		t += 0.004;
	}

	// CLOSE YOUR FILE
	ofs.close();
}

// Task 3: Request File from server
// Request a file under BIMDC/ given the file name, by sequentially requesting data chunks from the file and copying into a new file of the same name into received/
void requestFile(string filename, FIFORequestChannel &chan) {
	filemsg fm(0, 0);	// Request file length message
	string fname = filename; // Make sure to read filename from command line arguments (given to -f)

	// Calculate file length request message and set up buffer
	int len = sizeof(filemsg) + (fname.size() + 1);
	char* buf2 = new char[len];

	// Copy filemsg fm into msgBuffer, attach filename to the end of filemsg fm in msgBuffer, then write msgBuffer into pipe
	memcpy(buf2, &fm, sizeof(filemsg));
	strcpy(buf2 + sizeof(filemsg), fname.c_str());
	chan.cwrite(buf2, len);
    delete[] buf2;

	// Read file length response from server for specified file
	__int64_t file_length;
	chan.cread(&file_length, sizeof(__int64_t));
	cout << "The length of " << fname << " is " << file_length << endl;

	// Set up output file under received folder
	// Can use any file opening method
	ofstream ofs("./received/" + fname);

	// // Request data chunks from server and output into file
	// // Loop from start of file to file_length
	__int64_t pos = 0;
	__int64_t chunk_range = MAX_MESSAGE;
	
	while (pos < file_length) {
		// Create filemsg for data chunk range
    	// Assign data chunk range properly so that the data chunk to fetch from the file does NOT exceed the file length (i.e. take minimum between the two)
		filemsg fm(pos, min(file_length - pos, chunk_range));
		buf2 = new char[len];

		// Copy filemsg into buf2 buffer and write into pipe
		// File name need not be re-copied into buf2, as filemsg struct object is staticly sized and therefore the file name is unchanged when filemsg is re-copied into buf2
		memcpy(buf2, &fm, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), fname.c_str());
		chan.cwrite(buf2, len);
		delete[] buf2;

		char* buf = new char[min(file_length - pos, chunk_range)];

		// Read data chunk response from server into separate data buffer
		chan.cread(buf, min(file_length - pos, chunk_range));
		// Write data chunk into new file
		ofs.write(buf, min(file_length - pos, chunk_range));

		delete[] buf;

		pos += min(file_length - pos, chunk_range);
	}
	
	// CLOSE YOUR FILE
	ofs.close();
}

// Task 4: Request a new FIFORequestChannel
// Send a request to the server to establish a new FIFORequestChannel, and use the servers response to create the client's FIFORequestChannel
// Client must now communicate over this new RequestChannel for any data point or file transfers
void openChannel(FIFORequestChannel &chan, FIFORequestChannel *&chan2) {
	MESSAGE_TYPE m = NEWCHANNEL_MSG;
	// Write new channel message into pipe
	chan.cwrite(&m, sizeof(MESSAGE_TYPE));
    // Read response from pipe (Can create any static sized char array that fits server response, e.g. MAX_MESSAGE)
	char newPipeName[MAX_MESSAGE];
	chan.cread(newPipeName, MAX_MESSAGE);
    // Create a new FIFORequestChannel object using the name sent by server
	chan2 = new FIFORequestChannel(newPipeName, FIFORequestChannel::CLIENT_SIDE);
}

int main (int argc, char *argv[]) {
	int opt;

    // Can add boolean flag variables if desired
	int p = 1;
	double t = 0.0;
	int e = 1;
	string filename = "";
	int m = MAX_MESSAGE;
	bool c_Flag = false;
	bool p_Flag = false;
	bool f_Flag = false;

	// Add other arguments here   |   Need to add -c, -m flags. BE CAREFUL OF getopt() NOTATION
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				p_Flag = true;
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				f_Flag = true;
				break;
			case 'm':
				m = atoi(optarg);
				break;
			case 'c':
				c_Flag = true;
				break;
            /*
                Add -c and -m flag cases
            */
		}
	}

    // NOT NECESSARY: Check for p flag XOR f flag; That is, either p flag or f flag should be given not both
    if ( p_Flag == f_Flag && p_Flag &&  f_Flag) {
		EXITONERROR("p flag equal to f flag");
	}
    
	// Task 1:
	// Run the server process as a child of the client process
	
	// string mS = to_string(m); 
	// char* cmd[] = {"./server", "-m", (char*)mS.c_str(), nullptr };
    
	// Fork child process
	int pid = fork();
	if ( pid < 0 ) {
		EXITONERROR("forked failed");
	}
	else if (pid == 0) {
		string mS = to_string(m); 
		char* cmd[] = {(char*)"./server", (char*)"-m", (char*)mS.c_str(), (char*)NULL };
		execvp(cmd[0], cmd);
		exit(0);
	}

    // Run server

    // // SECOND CONDITIONAL NOT EVALUATED BY PARENT
	// if ( pid == 0 && ( execvp((char*)"./server") < 0 ) ) {
	// 	EXITONERROR("exit");
	// }

	// Set up FIFORequestChannel
    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
	FIFORequestChannel* chan2 = nullptr;
	FIFORequestChannel* data_channel = &chan;

	// Task 4:
	// Request a new channel (e.g. -c)
	// Should use this new channel to communicate with server (still use control channel for sending QUIT_MSG)
	if (c_Flag) {
		openChannel(chan, chan2);
	}
	if (chan2) {
		data_channel = chan2;
	}

	// Task 2.1 + 2.2:
	// Request data points
	if (p != 1) {
		if (t != 0.0) {
			requestDataPoint(p, t, e, *data_channel); // (e.g. './client -p 1 -t 0.000 -e 1')
		}
		else {
			requestData(p, *data_channel);	// (e.g. './client -p 1')
		} 
	}
	//Task 3:
	//Request files (e.g. './client -f 1.csv')
	else if (!filename.empty()) {
		requestFile(filename, *data_channel);
	}
	
	//Task 5:
	// Closing all the channels
    MESSAGE_TYPE Qm = QUIT_MSG;
    chan.cwrite(&Qm, sizeof(MESSAGE_TYPE));
	
	if (c_Flag) {
		chan2->cwrite(&Qm, sizeof(MESSAGE_TYPE));
		delete chan2;
	}
	
	
	// Wait on children
	wait(nullptr);

	return 0;
}
