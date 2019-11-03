#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include "msg.h"    /* For the message struct */

using namespace std;
/* The size of the shared memory chunk */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void *sharedMemPtr;

/* The name of the received file */
const char recvFileName[] = "keyfile.txt";

/* Initializing buffers */
message sndMsg;
message rcvMsg;

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory
 * @param msqid - the id of the shared memory
 * @param sharedMemPtr - the pointer to the shared memory
 */

void init(int& shmid, int& msqid, void*& sharedMemPtr)
{

	/* TODO: 1. Create a file called keyfile.txt containing string "Hello world" (you may do
 		    so manually or from the code).
	         2. Use ftok("keyfile.txt", 'a') in order to generate the key.
		 3. Use the key in the TODO's below. Use the same key for the queue
		    and the shared memory segment. This also serves to illustrate the difference
		    between the key and the id used in message queues and shared memory. The id
		    for any System V object (i.e. message queues, shared memory, and sempahores)
		    is unique system-wide among all System V objects. Two objects, on the other hand,
		    may have the same key.
	 */

	cout << "Generating key..." << endl;
	key_t key = ftok("keyfile.txt", 'a');
	if (key == -1)
	{
		perror("Key was not generated.");
		exit(1);
	}
	else
	{
		cout << "Key generated successfully!" << endl;
	}



	/* TODO: Allocate a piece of shared memory. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE. */
	cout << "Allocating shared memory..." << endl;
	//shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0644 | IPC_CREAT);
	shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, S_IRUSR | S_IWUSR | IPC_CREAT);
  	if (shmid == -1)
  	{
		perror("Did not allocate memory successfully.");
    exit(1);
	}
	else
	{
		cout << "Allocated memory successfully!" << endl;
	}

	/* TODO: Attach to the shared memory */
	cout << "Attaching to shared memory..." << endl;

	//sharedMemPtr = shmat(shmid, (void *)0, 0);
	sharedMemPtr = shmat(shmid, NULL, 0);
  	if (sharedMemPtr == (void *)(-1))
	{
    perror("Could not attach to shared memory.");
    exit(1);
	}
	else
	{
		cout << "Attached to shared memory successsfully!" << endl;
	}

	/* TODO: Create a message queue */
	/* Store the IDs and the pointer to the shared memory region in the corresponding parameters */

	cout << "Creating a message queue..." << endl;

	//msqid = msgget(key, 0644 | IPC_CREAT);
	msqid = msgget(key, S_IRUSR | S_IWUSR | IPC_CREAT);
	if (msqid == -1)
	{
    perror("Unable to create a message queue.");
    exit(1);
	}
	else
	{
		cout << "Created a message queue successfully!" << endl;
	}

}


/**
 * The main loop
 */
void mainLoop()
{
	/* The size of the mesage */
	int msgSize = 0;

	/* Open the file for writing */
	FILE* fp = fopen(recvFileName, "w");

	/* Error checks */
	cout << "Opening file..." << endl;
	if(!fp)
	{
		perror("File does not exist.");
		exit(-1);
	}
	else
	{
		cout << "File opened successfully!" << endl;
	}

    /* TODO: Receive the message and get the message size. The message will
     * contain regular information. The message will be of SENDER_DATA_TYPE
     * (the macro SENDER_DATA_TYPE is defined in msg.h).  If the size field
     * of the message is not 0, then we copy that many bytes from the shared
     * memory region to the file. Otherwise, if 0, then we close the file and
     * exit.
     */

	/* Keep receiving until the sender set the size to 0, indicating that
 	 * there is no more data to send
 	 */
	cout << "Receiving message..." << endl;
	if(msgrcv(msqid, &rcvMsg, sizeof(rcvMsg), SENDER_DATA_TYPE, 0) == -1)
	{
		perror("Message not received.");
		exit(1);
	}
	else
	{
		cout << "Received successfully!" << endl;
	}

	msgSize = rcvMsg.size;
	if(msgSize != 0)
	{
		while(msgSize != 0)
		{
			/* Save the shared memory to file */
			if(fwrite(sharedMemPtr, sizeof(char), msgSize, fp) < 0)
			{
				perror("fwrite");
			}
			else
			{
				cout << "Ready to receive next message." << endl;
			}

			sndMsg.mtype = RECV_DONE_TYPE;
			sndMsg.size = 0;

			/* TODO: Tell the sender that we are ready for the next file chunk.
 			 * I.e. send a message of type RECV_DONE_TYPE (the value of size field
 			 * does not matter in this case).
 			 */
			cout << "Sending RECV_DONE_TYPE message..." << endl;
			if(msgsnd(msqid, &sndMsg, 0, 0) == -1)
			{
				perror("Unable to send RECV_DONE_TYPE message.");
			}
			else
			{
				cout << "RECV_DONE_TYPE message sent successfully!" << endl;
			}

			cout << "Receiving message..." << endl;
			if(msgrcv(msqid, &rcvMsg, sizeof(rcvMsg), SENDER_DATA_TYPE, 0) == -1)
			{
				perror("Message not received.");
				exit(1);
			}
			else
			{
				cout << "Received successfully!" << endl;
			}

			msgSize = rcvMsg.size;
		}
	}
	else
	{
		/* Close the file */
		fclose(fp);
		cout << "Memory size is 0. File closed." << endl;
	}
	fclose(fp);
}



/**
 * Perfoms the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */

void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
	/* TODO: Detach from shared memory */
	cout << "Detaching from shared memory..." << endl;
	if(shmdt(sharedMemPtr) == -1)
	{
	   perror("Unable to detach from shared memory.");
	   exit(1);
  }
  else
	{
		cout << "Detached from shared memory!" << endl;
	}

	/* TODO: Deallocate the shared memory chunk */
	cout << "Deallocating shared memory chunk..." << endl;
	if(shmctl(shmid, IPC_RMID, NULL) == -1)
	{
		perror("Unable to deallocate from shared memory chunk.");
	  	exit(1);
	}
	else
	{
		cout << "Deallocated the shared memory chunk!" << endl;
	}

	/* TODO: Deallocate the message queue */
	cout << "Deallocating message queue..." << endl;
	if(msgctl( msqid, IPC_RMID, NULL) == -1)
	{
    perror("Unable to deallocate message queue.");
    exit(1);
  	}
	else
	{
    cout << "Deallocated message queue!" << endl;
	}
}

/**
 * Handles the exit signal
 * @param signal - the signal type
 */

void ctrlCSignal(int signal)
{
	/* Free system V resources */
	cleanUp(shmid, msqid, sharedMemPtr);
}

int main(int argc, char** argv)
{

	/* TODO: Install a singnal handler (see signaldemo.cpp sample file).
 	 * In a case user presses Ctrl-c your program should delete message
 	 * queues and shared memory before exiting. You may add the cleaning functionality
 	 * in ctrlCSignal().
 	 */

	signal(SIGINT,ctrlCSignal);

	/* Initialize */
	init(shmid, msqid, sharedMemPtr);

	/* Go to the main loop */
	mainLoop();

	/** TODO: Detach from shared memory segment, and deallocate shared memory and message queue (i.e. call cleanup) **/
	cleanUp(shmid, msqid, sharedMemPtr);
	cout << "Program completed!" << endl;

	return 0;
}
