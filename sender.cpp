#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include "msg.h"    /* For the message struct */
using namespace std;

/* The size of the shared memory chunk*/
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void* sharedMemPtr;

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory
 * @param msqid - the id of the shared memory
 */

void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
	/* TODO: */

  /*
			1. Create a file called keyfile.txt containing string "Hello world" (you may do
 		    so manually or from the code).
	*/

			//creating the file and adding the string "Hello world" to the file
		 	 
			ofstream outfile;
			outfile.open("keyfile.txt");
			outfile << "Hello world";
			outfile.close();
			 /*
				Alternative method:
		 		printf("Hello world");
		   	gets(message.mesg_text);
			*/


	/*
			2. Use ftok("keyfile.txt", 'a') in order to generate the key.
	*/
			 key_t key = ftok("keyfile.txt", 'a');


	/*
				3. Use the key in the TODO's below. Use the same key for the queue
		    and the shared memory segment. This also serves to illustrate the difference
		    between the key and the id used in message queues and shared memory. The id
		    for any System V objest (i.e. message queues, shared memory, and sempahores)
		    is unique system-wide among all SYstem V objects. Two objects, on the other hand,
		    may have the same key.
	*/

	/*
			TODO: Get the id of the shared memory segment. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE
	*/

	//creating the id and getting the key to the memory segment and giving it the permission to read and write on the file
	shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, S_IRUSR | S_IWUSR | IPC_CREAT);

	/*
			TODO: Attach to the shared memory
	*/

	//pointing the sharedMemPtr to the memory segment which we just assigned to the ID
  sharedMemPtr = shmat(shmid, NULL, 0);
	//Altrnative method: char *star = (char*) shmat(shmid,(void*)0,0);


	/*
			TODO: Attach to the message queue
	*/
	msqid = msgget(key, S_IRUSR | S_IWUSR | IPC_CREAT);
  //message.mesg_type = 1;

	/* Store the IDs and the pointer to the shared memory region in the corresponding parameters */

}

/**
 * Performs the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */

void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
	/* TODO: Detach from shared memory */
	shmdt(sharedMemPtr); //the memory is detached from the sender
	cout << "Complete! Memory successfully was detatched.\n";

}

/**
 * The main send function
 * @param fileName - the name of the file
 */
void send(const char* fileName)
{
	/* Open the file for reading */
	FILE* fp = fopen(fileName, "r");


	/* A buffer to store message we will send to the receiver. */
	message sndMsg;

	/* A buffer to store message received from the receiver. */
	message rcvMsg;

	/* Was the file open? */
	if(!fp)
	{
		perror("fopen");  
		exit(-1);
	}

	/* Read the whole file */
	while(!feof(fp))
	{
		/* Read at most SHARED_MEMORY_CHUNK_SIZE from the file and store them in shared memory.
 		 * fread will return how many bytes it has actually read (since the last chunk may be less
 		 * than SHARED_MEMORY_CHUNK_SIZE).
 		 */
		if((sndMsg.size = fread(sharedMemPtr, sizeof(char), SHARED_MEMORY_CHUNK_SIZE, fp)) < 0)
		{
			perror("fread");
			exit(-1);
		}

		/* TODO: Send a message to the receiver telling him that the data is ready
 		 * (message of type SENDER_DATA_TYPE)*/
		 sndMsg.mtype = SENDER_DATA_TYPE;
		 msgsnd(msqid, &sndMsg, sizeof(message)- sizeof(long), 0);
		 //Alterate way: msgsnd(msgid, &sndMsg, sizeof(sndMsg)- sizeof(long), 0);


		/* TODO: Wait until the receiver sends us a message of type RECV_DONE_TYPE telling us
 		 * that he finished saving the memory chunk.*/
		 msgrcv(msqid, &rcvMsg, sizeof(rcvMsg) - sizeof(long), RECV_DONE_TYPE, 0);

	}


	/** TODO: once we are out of the above loop, we have finished sending the file.
 	  * Lets tell the receiver that we have nothing more to send. We will do this by
 	  * sending a message of type SENDER_DATA_TYPE with size field set to 0.
	  */
		sndMsg.size = 0;
		msgsnd(msqid, &sndMsg, sizeof(message)- sizeof(long), 0);
		//Alterate way: msgsnd(msgid, &sndMsg, sizeof(sndMsg)- sizeof(long), 0);

	/* Close the file */
	fclose(fp);

}


int main(int argc, char** argv)
{

	/* Check the command line arguments */
	if(argc < 2)
	{
		fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);
		exit(-1);
	}

	/* Connect to shared memory and the message queue */
	init(shmid, msqid, sharedMemPtr);


	/* Send the file */
	send(argv[1]);

	/* Cleanup */
	cleanUp(shmid, msqid, sharedMemPtr);

	return 0;
}
