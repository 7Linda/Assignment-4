/** This program illustrates the server end of the message queue **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <vector>
#include <list>
#include <string>
#include <fstream>
#include <cstring>

#include "msg.h"
using namespace std;

struct record
{
	/* The record id */
	int id;
	
	/* The first name */	
	string firstName;
		
	/* The first name */
	string lastName;	
};



/**
 * The structure of a hashtable cell
 */
class hashTableCell
{
	/* Public members */
	public:
	

	/**
 	 * Initialize the mutex
 	 */
	hashTableCell()
	{
		/* Initialize the mutex using pthread_mutex_init() */
		pthread_mutex_init(&cellMutex, NULL);

	}
	
	/**
 	 * Initialize the mutex
 	 */
	~hashTableCell()
	{
		/* Deallocate the mutex using pthread_mutex_destroy() */
		pthread_mutex_destroy(&cellMutex);
	}
	
	/**
 	 * Locks the cell mutex
 	 */
	void lockCell()
	{
		/*TODO: Add code for locking the cell mutex */
		pthread_mutex_lock(&cellMutex);
	}
	
	/**
 	 * Unlocks the cell mutex
 	 */
	void unlockCell()
	{
		/* TODO: Add code for unlocking the cell mutex */
		pthread_mutex_unlock(&cellMutex);
	}

		
	
	/* The linked list of records */
	list<record> recordList;
	
	
	/**
 	 * TODO: declare a cell mutex
 	 */
	pthread_mutex_t cellMutex;
	
};

/* The number of cells in the hash table */
#define NUMBER_OF_HASH_CELLS 100

/* The number of inserter threads */
#define NUM_INSERTERS 5

/* The hash table */
vector<hashTableCell> hashTable(NUMBER_OF_HASH_CELLS);

/* The number of threads */
int numThreads = 0;

/* The message queue id */
int msqid;

/* The ids that yet to be looked up */
list<int> idsToLookUpList;

pthread_t* lookupThreads;

/**
 * TODO: Declare and initialize a mutex for protecting the idsToLookUpList.
 */
pthread_mutex_t idsToLookUpListMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * TODO: declare and initialize the condition variable, threadPoolCondVar, 
 * for implementing a thread pool.
 */

pthread_cond_t threadPoolCondVar = PTHREAD_COND_INITIALIZER;

/* TODO: Declare the mutex, threadPoolMutex, for protecting the thread pool
 * condition variable. 
 */

pthread_mutex_t threadPoolMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Prototype for createInserterThreads
 */
void createInserterThreads();


/**
 * A prototype for adding new records.
 */
void* addNewRecords(void* arg);

/**
 * Deallocated the message queue
 * @param sig - the signal
 */
void cleanUp(int sig)
{
	fprintf(stderr, "Cleaning up message queue...\n");
	
	/* Remove the message queue */
	if (msgctl(msqid, IPC_RMID, NULL) < 0)
	{
		perror("msgctl");
	}
	else
	{
		fprintf(stderr, "Message queue %d removed successfully.\n", msqid);
	}
	
	/* Exit the program */
	exit(0);
}

/**
 * Sends the message over the message queue
 * @param msqid - the message queue id
 * @param rec - the record to send
 */
void sendRecord(const int& msqid, const record& rec)
{
	/**
 	 * Convert the record to message
 	 */
		
	/* The message to send */
	message msg; 
	
	/* Copy fields from the record into the message queue */	
	msg.messageType = SERVER_TO_CLIENT_MSG;
	msg.id = rec.id;
	strncpy(msg.firstName, rec.firstName.c_str(), MAX_NAME_LEN);	
	msg.firstName[MAX_NAME_LEN - 1] = '\0';
	strncpy(msg.lastName, rec.lastName.c_str(), MAX_NAME_LEN);
	msg.lastName[MAX_NAME_LEN - 1] = '\0';
	
	/* Send the message */
	sendMessage(msqid, msg);		
}

/**
 * Prints the hash table
 */
void printHashTable()
{
	/* Go through the hash table */
	for(vector<hashTableCell>::const_iterator hashIt = hashTable.begin();
		hashIt != hashTable.end(); ++hashIt)
	{
		/* Go through the list at each hash location */
		for(list<record>::const_iterator lIt = hashIt->recordList.begin();
			lIt != hashIt->recordList.end(); ++lIt)
		{
			fprintf(stderr, "%d-%s-%s-%d\n", lIt->id, 
						lIt->firstName.c_str(), 
						lIt->lastName.c_str(),
						lIt->id % NUMBER_OF_HASH_CELLS
						);
		}
	}
}

/**
 * Adds a record to hashtable
 * @param rec - the record to add
 */
void addToHashTable(const record& rec)
{
	/**
 	 * TODO: grab mutex of the hash table cell
 	 */
	hashTableCell* hashTableCellPtr = &hashTable.at(rec.id % NUMBER_OF_HASH_CELLS);
	hashTableCellPtr->lockCell();
	
	/* Hash, and save the record */
	hashTableCellPtr->recordList.push_back(rec);
	
	/**
 	 * TODO: release mutex of the hashtable cell
 	 */

	hashTableCellPtr->unlockCell();
	
}


/**
 * Adds a record to hashtable
 * @param id the id of the record to retrieve
 * @return - the record from hashtable if exists;
 * otherwise returns a record with id field set to -1
 */
record getHashTableRecord(const int& id)
{
	/* Get pointer to the hash table record */
	hashTableCell* hashTableCellPtr = &hashTable.at(id % NUMBER_OF_HASH_CELLS); 
	
	/**
 	 * The record to return
 	 */
	 record rec = { -1, "", ""};
	
	/**
 	 * TODO: grab mutex of the cell
 	 */
	hashTableCellPtr->lockCell();
	
	/* Get the iterator to the list of records hashing to this location */
	list<record>::iterator recIt = hashTableCellPtr->recordList.begin();
	
	/* Go through all the records */
	while(recIt != hashTableCellPtr->recordList.end())
	{
		/* Save the record */
		if(recIt->id == id) 
		{
			rec = *recIt;
			break;
		}
		
		/* Advance the record it */
		++recIt;
	}
	
	
	
	/**
 	 * TODO: release mutex of the cell. Hint: call unlockCell() to release
     *       mutex protecting the cell.
 	 */

	hashTableCellPtr->unlockCell();
	
	return rec;
}


/**
 * Loads the database into the hashtable
 * @param fileName - the file name
 * @return - the number of records left.
 */
int populateHashTable(const string& fileName)
{	
	/* The record */
	record rec;
	int numRecords = 0;
	
	/* Open the file */
	ifstream dbFile(fileName.c_str());
	
	/* Is the file open */
	if(!dbFile.is_open())
	{
		fprintf(stderr, "Could not open file %s\n", fileName.c_str());
		exit(-1);
	}
	
	
	/* Read the entire file */
	while(!dbFile.eof())
	{
		/* Read the id */
		dbFile >> rec.id;
		
		/* Make sure we did not hit the EOF */
		if(!dbFile.eof())
		{
			/* Read the first name and last name */
			dbFile >> rec.firstName >> rec.lastName;
						
			/* Add to hash table */
			addToHashTable(rec);
			numRecords++;	
		}
	}
	
	/* Close the file */
	dbFile.close();

	return numRecords;
}

/**
 * Gets ids to process from work list
 * @return - the id of record to look up, or
 * -1 if there is no work
 */
int getIdsToLookUp()
{
	/* The id */
	int id = -1;
	
	/* TODO: Aquire the idsToLookUpListMutex mutex */
	pthread_mutex_lock(&idsToLookUpListMutex);
	
	/* Remove id from the list if exists */
	if(!idsToLookUpList.empty()) 
    { 
        id = idsToLookUpList.front(); 
        idsToLookUpList.pop_front(); 
    }
	
	/* TODO: Release idsToLookUpListMutex  */
	pthread_mutex_unlock(&idsToLookUpListMutex);

	return id;
}

/**
 * Add id of record to look up
 * @param id - the id to process
 */
void addIdsToLookUp(const int& id)
{
	/* TODO: Aquire idsToLookUpListMutex the list mutex */
	pthread_mutex_lock(&idsToLookUpListMutex);
	/* Add the element to look up */
	idsToLookUpList.push_back(id);
		
	/* TODO: Release the idsToLookUpList  */
	pthread_mutex_unlock(&idsToLookUpListMutex);
}

/**
 * The thread pool function
 * @param thread argument
 */
void* threadPoolFunc(void* arg)
{
	/* The id to process */
	int id = -1; 
	
	/* Sleep until work arrives */
	while(true)
	{

		/* TODO: Lock the mutex protecting threadPoolCondVar from race conditions */
		pthread_mutex_lock(&threadPoolMutex);

		/* Get the id to look up */
		id = getIdsToLookUp();	
			
		/* No work to do */
		while(id == -1)
		{
			
			/* TODO: Sleep on the condition variable threadPoolCondVar */
			pthread_cond_wait(&threadPoolCondVar, &threadPoolMutex);
			
			/* Get the id to look up */
			id = getIdsToLookUp();	
			
		}
		
		
		/* TODO: Release the mutex protecting threadPoolCondVar from race conditions */
		pthread_mutex_unlock(&threadPoolMutex);
			
		/* Look up id */
		record rec = getHashTableRecord(id);
		
		
		/* Send the record to the client */
		sendRecord(msqid, rec);
		
	}
	
}

/**
 * Wakes up a thread from the thread pool
 */
void wakeUpThread()
{
	

	/* TODO: Lock the mutex protecting threadPoolCondVar from race conditions */
	pthread_mutex_lock(&threadPoolMutex);

	/* TODO: Wake up a thread sleeping on threadPoolCondVar */
	pthread_cond_signal(&threadPoolCondVar);
	
	/* TODO: Release the mutex protecting threadPoolCondVar from race conditions */
	pthread_mutex_unlock(&threadPoolMutex);
}

/**
 * Creates the threads for looking up ids
 * @param numThreads - the number of threads to create
 */
void createThreads(const int& numThreads)
{
	lookupThreads = new pthread_t[numThreads];
	
	/* Create the lookup threads */
	for(int i = 0; i < numThreads; i++)
	{
		if(pthread_create(&lookupThreads[i], NULL, threadPoolFunc, NULL) != 0)
		{
			perror("pthread_create for lookup thread");
			exit(-1);
		}
		fprintf(stderr, "Created lookup thread %d\n", i);
	}
}

/**
 * Creates threads that update the database
 * with randomly generated records
 */
void createInserterThreads()
{

	/* TODO: create NUM_INSERTERS threads that add new elements to the hashtable
 	 * by calling addNewRecords(). 
 	 */
		static pthread_t inserterThreads[NUM_INSERTERS];

		for (int i = 0; i < NUM_INSERTERS; i++)
	{
		if (pthread_create(&inserterThreads[i], NULL, addNewRecords, NULL) != 0)
		{
			perror("pthread_create for inserter thread");
			exit(-1);
		}
		fprintf(stderr, "Created inserter thread %d\n", i);
	}
}



/**
 * Called by parent thread to process incoming messages
 */
void processIncomingMessages()
{
	/* The arriving message */
	message msg;
	
	/* The id of the record */
	int id = -1;
	
	/* Wait for messages forever */
	while(true)
	{

		/* Receive the message */
		recvMessage(msqid, msg, CLIENT_TO_SERVER_MSG);
		
		/* TODO: Add id to the list of ids that should be processed */
		addIdsToLookUp(msg.id);
			
		/* TODO: Wake up a thread to process the newly received id */
		wakeUpThread();
	}
}

/**
 * Generates a random record
 * @return - a random record
 */
record generateRandomRecord()
{
	/* A record */
	record rec;
		
	/* Generate a random id */
	rec.id = rand() % NUMBER_OF_HASH_CELLS;	
	
	/* Add the fake first name and last name */
	rec.firstName = "Random";
	rec.lastName = "Record";
	
	return rec;
}

/**
 * Threads inserting new records to the database
 * @param arg - some argument (unused)
 */
void* addNewRecords(void* arg)
{	
	/* A randomly generated record */
	record rec;
		
	/* Keep generating random records */	
	while(true)
	{
		/* Generate a record */
		rec = generateRandomRecord();
		
		/* Add the record to hashtable */
		addToHashTable(rec);
	}
	
}

int main(int argc, char** argv)
{
	/**
 	 * Check the command line
 	 */
	if(argc < 3)
	{
		fprintf(stderr, "USAGE: %s <DATABASE FILE NAME> <NUMBER OF THREADS>\n", argv[0]);
		exit(-1);
	}
	
	/* TODO: install a signal handler for deallocating the message queue */	
	signal(SIGINT, cleanUp);

	/* Populate the hash table */
	populateHashTable(argv[1]);
	
	
	printHashTable();
	
	/* Get the number of lookup ID threads */
	numThreads = atoi(argv[2]);
	
	/* Use a random file and a random character to generate
	 * a unique key. Same parameters to this function will 
	 * always generate the same value. This is how multiple
	 * processes can connect to the same queue.
	 */
	key_t key = ftok("/bin/ls", 'O');
	
	/* Error checks */
	if(key < 0)
	{
		perror("ftok");
		exit(-1);
	}
		
	/* Connect to the message queue */
	msqid = createMessageQueue(key);	
	
	/* Instantiate a message buffer for sending/receiving msgs 
       from msg queue */
	message msg;
	
	/* Create the lookup ID threads */
	createThreads(numThreads);				
	
	/* Create the inserter threads */
	createInserterThreads();
		
	/* Process incoming requests */
	processIncomingMessages();
	
	
	return 0;
}
