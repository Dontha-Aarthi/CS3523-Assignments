#include <iostream>
#include <thread>
#include <string.h>
#include <fstream>
#include <ctime>
#include <chrono>
#include <unistd.h>
#include <random>
#include <semaphore.h>
#include <vector>
#include <algorithm>
#include <mutex>

using namespace std;

int nw,nr,kw,kr,mu_cs,mu_rem;
sem_t rw_mutex, sem_mutex;
int read_count = 0;
vector <vector<double>> reader_waiting_time;
vector <vector<double>> writer_waiting_time;

//files which contains log and average
ofstream output("RW-log.txt");
ofstream avg("Average_time-RW.txt");

mutex mtx_lock_reader;
mutex mtx_lock_writer;

void writer(int id)
{
	random_device r;
   seed_seq ssq{r()};
   default_random_engine generator{r()};
   exponential_distribution <double> distribution1((float)1000/mu_cs);
   exponential_distribution <double> distribution2((float)1000/mu_rem);
   chrono::duration <double> randCSTime(distribution2(generator));
   chrono::duration <double> randRemTime(distribution2(generator));
    
   time_t req_time[nw] = {0}, entry_time[nw] = {0}, exit_time; 
   
	for(int i = 1; i <= kw; i++)
	{
		time(&req_time[id]);
      string reqTime(ctime(&req_time[id]));
      mtx_lock_writer.lock();
		// cout << i << "th CS request by Writer Thread " << id << " at " << reqTime.substr(11,8) << endl;
		output << i << "th CS request by Writer Thread " << id << " at " << reqTime.substr(11,8) << endl;
		mtx_lock_writer.unlock();
		sem_wait(&rw_mutex);

		time(&entry_time[id]);
		string enterTime(ctime(&entry_time[id]));
		mtx_lock_writer.lock();
		// cout << i << "th CS entry by Writer Thread " << id << " at " << enterTime.substr(11,8) << endl;
		output << i << "th CS entry by Writer Thread " << id << " at " << enterTime.substr(11,8) << endl;
		mtx_lock_writer.unlock();
		writer_waiting_time[id].push_back(difftime(entry_time[id], req_time[id])*1000);
		this_thread::sleep_for(randCSTime); //simulate a thread writing in CS

		sem_post(&rw_mutex);

		time(&exit_time);
		string exitTime(ctime(&exit_time));
		mtx_lock_writer.lock();
		// cout << i << "th CS exit by Writer Thread " << id << " at " << exitTime.substr(11,8) << endl;
		output << i << "th CS exit by Writer Thread " << id << " at " << exitTime.substr(11,8) << endl;
		mtx_lock_writer.unlock();

		this_thread::sleep_for(randRemTime); // simulate a thread executing in Remainder Section
	}
}

void reader(int id)
{
	random_device r;
   seed_seq ssq{r()};
   default_random_engine generator{r()};
   exponential_distribution <double> distribution1((float)1000/mu_cs);
   exponential_distribution <double> distribution2((float)1000/mu_rem);
   chrono::duration <double> randCSTime(distribution2(generator));
   chrono::duration <double> randRemTime(distribution2(generator));
    
   time_t req_time[nr] = {0}, entry_time[nr] = {0}, exit_time; 
	for(int i = 1; i <= kr; i++)
	{
		time(&req_time[id]);
      string reqTime(ctime(&req_time[id]));
      mtx_lock_reader.lock();
		// cout << i << "th CS request by Reader Thread " << id << " at " << reqTime.substr(11,8)<< endl;
		output << i << "th CS request by Reader Thread " << id << " at " << reqTime.substr(11,8)<< endl;
		mtx_lock_reader.unlock();
		sem_wait(&sem_mutex);
		read_count++;
		if(read_count == 1)
		{
			sem_wait(&rw_mutex);
		}
		sem_post(&sem_mutex);

		time(&entry_time[id]);
		string enterTime(ctime(&entry_time[id]));
		mtx_lock_reader.lock();
		// cout << i << "th CS entry by Reader Thread " << id << " at " << enterTime.substr(11,8) << endl;
		output << i << "th CS entry by Reader Thread " << id << " at " << enterTime.substr(11,8) << endl;
		mtx_lock_reader.unlock();
		reader_waiting_time[id].push_back(difftime(entry_time[id], req_time[id])*1000);
		this_thread::sleep_for(randCSTime); //simulate a thread writing in CS

		sem_wait(&sem_mutex);
		read_count--;
		if(read_count == 0)
		{
			sem_post(&rw_mutex);
		}
		sem_post(&sem_mutex);
		
		time(&exit_time);
		string exitTime(ctime(&exit_time));
		mtx_lock_reader.lock();
		// cout << i << "th CS exit by Reader Thread " << id << " at " << exitTime.substr(11,8) << endl;
		output << i << "th CS exit by Reader Thread " << id << " at " << exitTime.substr(11,8) << endl;
		mtx_lock_reader.unlock();
		
		this_thread::sleep_for(randRemTime); // simulate a thread executing in Remainder Section
	}
}


int main()
{
   //input file
 	ifstream input("inp-params.txt");

   //reading from input file
 	input >> nw >> nr >> kw >> kr >> mu_cs >> mu_rem; 
 	input.close();

 	sem_init(&rw_mutex,0,1);
 	sem_init(&sem_mutex,0,1);


   // initialising threads 
 	vector <std::thread> writer_threads;
 	vector <std::thread> reader_threads;

 	// creating 2d array of waiting times
 	for(int i = 0; i < nr+1; i++){
        reader_waiting_time.push_back(vector<double>());
   }

   for(int i = 0; i < nw+1; i++){
        writer_waiting_time.push_back(vector<double>());
   }
 	
   //calling writer function using writer threads
 	for(int i = 0; i < nw; i++){
 		writer_threads.push_back(std::thread(writer,i+1));
 	}
 	for(int i = 0; i < nr; i++){
 		reader_threads.push_back(std::thread(reader,i+1));
 	}

    //joining threads
 	for(auto& th : reader_threads){
 		th.join();
 	}
 	for(auto& th : writer_threads){
 		th.join();
 	}

 	// printing all the waiting times 
 	double reader_avg_wt = 0, writer_avg_wt = 0;
 	for(int i = 1; i < reader_waiting_time.size(); i++){
 		cout << "Waiting times of Reader Thread " << i << ": ";
        	for(int j = 0; j < reader_waiting_time[i].size(); j++){
            	cout << reader_waiting_time[i][j] << " ";
            	reader_avg_wt += reader_waiting_time[i][j];
        	}
     	cout << endl;
   }

   cout << endl;

   for(int i = 1; i < writer_waiting_time.size(); i++){
 		cout << "Waiting times of Writer Thread " << i << ": ";
        	for(int j = 0; j < writer_waiting_time[i].size(); j++){
          	  cout << writer_waiting_time[i][j] << " ";
          	  writer_avg_wt += writer_waiting_time[i][j];
        	}
     	cout << endl;
   }

   //printing avg times into Average_time-RW.txt file
   avg << "Reader thread avg waiting time is: " << reader_avg_wt/(nr*kr) << "\n";
   avg << "Writer thread avg waiting time is: " << writer_avg_wt/(nw*kw) << "\n";
   output.close();
	avg.close();	
 	return 0;
 }