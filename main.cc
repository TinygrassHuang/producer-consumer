/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"
#include <vector>
using namespace std;

void *producer (void *id);
void *consumer (void *id);

// job parameters
struct job{
	int id = -1;
	int duration = -1;
};

int queue_size;
int num_job;
int num_producer;
int num_consumer;
int sem_array_id;
key_t key = SEM_KEY;

// generate circular queue
vector<job> queue; // consume first in the queue and push_back new job in

int job_id_counter = 0;

int main (int argc, char **argv)
{
	// check number of input arguments
	if(argc != 5){
		cerr << "Wrong number of input arguments" << endl;
		return -1;
	}
	
	
	// initialise all the variables from input
	queue_size = check_arg(argv[1]);
	num_job = check_arg(argv[2]);
	num_producer = check_arg(argv[3]);
	num_consumer = check_arg(argv[4]);
	// check input argument validity
	if(queue_size==-1||num_job==-1||num_producer==-1||num_consumer==-1){
		cerr << "Invalid input argument" << endl;
		return -1;
	}
	
	// initialise 3 semaphores (mutual exclusive, space, item), and check errors
	sem_array_id = sem_create(key,NUM_SEM); // create semaphore array first
	if(sem_array_id == -1){
		cerr << "Error in creating semaphore array" << endl;
		return -1;
	}
	
	if(sem_init(sem_array_id, MUTEX, 1) == -1){
		cerr << "Error in initialising semaphore" << endl;
		return -1;
	}
	
	if(sem_init(sem_array_id, SPACE, queue_size) == -1){
		cerr << "Error in initialising semaphore" << endl;
		return -1;
	}
	
	if(sem_init(sem_array_id, ITEM, 0) == -1){
		cerr << "Error in initialising semaphore" << endl;
		return -1;
	}
	
	
	// create producer threads and consumer threads
	pthread_t producer_id[num_producer];
	pthread_t consumer_id[num_consumer];
	
	int pID[num_producer];
	int cID[num_consumer];
	
	for(int i=0; i < num_producer; i++){
		pID[i] = i+1;
		if(pthread_create(&producer_id[i], NULL, producer, (void *) &pID[i]) != 0){
			cerr << "Error in creating producer thread" << endl;
			return -1;
		}
	}
	
	for(int j=0; j < num_consumer; j++){
		cID[j] = j+1;
		if(pthread_create(&consumer_id[j], NULL, consumer, (void *) &cID[j]) != 0){
			cerr << "Error in creating consumer thread" << endl;
			return -1;
		}
	}

	
	
	// finish up and quit programme
	for(int i=0; i < num_producer; i++){
		pthread_join(producer_id[i],NULL);
	}
	for(int i=0; i < num_consumer; i++){
		pthread_join(consumer_id[i],NULL);
	}
	
	if(sem_close(sem_array_id) == -1){
		cerr << "Error in closing semaphore array" << endl;
		return -1;
	}
	
	return 0;
}

void *producer (void *parameter) 
{
	int* p_id = (int *) parameter;

	for(int i=0; i < num_job; i++){
		
		
		// down(space) - if wait over 20s, quit
		if(sem_wait_20s(sem_array_id,SPACE) == -1){
			cout << "Producing job timeout (20s)" << endl;
			pthread_exit(0);
		}
		sem_wait(sem_array_id,MUTEX); // down(MUTEX)
		sleep(rand()%5 + 1); //sleep
		
		// produce new job
		job new_job;
		new_job.id = job_id_counter%queue_size;
		new_job.duration = rand()%10 + 1;
		
		queue.push_back(new_job); // deposit item at the back of queue
		job_id_counter++; // iterate job id counter
		cout << "Producer(" << *p_id << "): Job id " << new_job.id << " duration " << new_job.duration << endl;
		sem_signal(sem_array_id,MUTEX); // up(MUTEX)
		sem_signal(sem_array_id,ITEM); // up(item)
		
	}
	
	// quit when no more jobs left
	cout << "Producer(" << *p_id << "): No more jobs to generate" << endl;
	pthread_exit(0);
}

void *consumer (void *parameter) 
{
	int* c_id = (int *) parameter;

	job c_job;
	while(1){
		
		// down(item) - if wait over 20s, quit
		if(sem_wait_20s(sem_array_id,ITEM) == -1){
			cout << "Consumer(" << *c_id << "): No more jobs left" << endl;
			pthread_exit(0);
		}
		
		sem_wait(sem_array_id,MUTEX); // down(MUTEX)
		c_job = queue.front();// fetch first job
		queue.erase(queue.begin());// delete first job in queue
		cout << "Consumer(" << *c_id << "): Job id " << c_job.id << " executing sleep duartion " << c_job.duration << endl;
		sem_signal(sem_array_id,MUTEX); // up(MUTEX)
		sem_signal(sem_array_id,SPACE); // up(space)
		sleep(c_job.duration);// consume item
		cout << "Consumer(" << *c_id << "): Job id " << c_job.id << " completed" << endl;
	}

  pthread_exit (0);

}
