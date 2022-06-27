#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <string.h>
#include <fstream>
#include <ctime>
#include <chrono>
#include <unistd.h>
#include <random>
#include <algorithm>

using namespace std;

//global waiting array which is initialised to false(CAS bounded waiting)
vector <vector<double>> waiting_time;

//total waiting time of all threads
double total_waiting_time = 0;

//initialising input varaibles
int n,k,lambda1,lambda2;

//file which contains log
ofstream output("tas.txt");

atomic_flag lock_stream = ATOMIC_FLAG_INIT; //lock is set to false initially

//entry section code
void entry_sec()
{
	while(lock_stream.test_and_set() == 1){}
}

//exit section code
void exit_sec()
{
	lock_stream.clear();
}

void testCS(int index)
{
    int id = index;

    //generating t1 and t2 using exponential distribution
    random_device r;
    seed_seq ssq{r()};
    default_random_engine generator{r()};
    exponential_distribution <double> distribution1((float)1/lambda1);
    exponential_distribution <double> distribution2((float)1/lambda2);
    chrono::duration <double> t1(distribution2(generator));
    chrono::duration <double> t2(distribution2(generator));
    
    time_t req_time[n] = {0}, entry_time[n] = {0}, exit_time; 
    for(int i = 1; i <= k ; i ++)
    {       
        time(&req_time[id]);
        string reqEnterTime(ctime(&req_time[id]));
        output << i << "th CS Request at " << reqEnterTime.substr(11,8) << " by thread " << id << endl;
        cout << i << "th CS Request at " << reqEnterTime.substr(11,8) << " by thread " << id << endl;

        entry_sec(); // Entry Section
        
        time(&entry_time[id]);
        string actEnterTime(ctime(&entry_time[id]));
        output << i << "th CS Entry at " << actEnterTime.substr(11,8) << " by thread " << id << endl;
        cout << i << "th CS Entry at " << actEnterTime.substr(11,8) << " by thread " << id << endl;
        
        double wait = difftime(entry_time[id],req_time[id]);        
        waiting_time[id].push_back(wait);
        total_waiting_time += wait;
        
        this_thread::sleep_for(t1); // Simulation of critical-section

        exit_sec(); // Exit Section
        
        time(&exit_time);
        string exitTime(ctime(&exit_time));
        output << i << "th CS Exit at " << exitTime.substr(11,8) << " by thread " << id << endl;
        cout << i << "th CS Exit at " << exitTime.substr(11,8) << " by thread " << id << endl;

        this_thread::sleep_for(t2); // Simulation of Reminder Section
    }     
}

 int main()
 {
    //input file
 	ifstream input("inp-params.txt");

    //reading from input file
 	input >> n >> k >> lambda1 >> lambda2; 

    // initialising threads 
 	vector <std::thread> threads;

    // creating 2d array of waiting times
 	for(int i = 0; i < n+1; i++){
        waiting_time.push_back(vector<double>());
    }
 	
    //calling testCS function using threads
 	for(int i = 0; i < n; i++){
 		threads.push_back(std::thread(testCS,i+1));
 	}

    //joining threads
 	for(auto& th : threads){
 		th.join();
 	}

    // printing all the waiting times 
 	for(int i = 1; i < waiting_time.size(); i++){
 		cout << "Waiting times of Thread " << i << ": ";
        for(int j = 0; j < waiting_time[i].size(); j++){
            cout << waiting_time[i][j] << " ";
        }
        cout << endl;
    }
    // average waiting time for a process to enter CS
    cout << "Average waiting time is " << (float)total_waiting_time/(k*n) << " sec"<< endl;
	
 	return 0;
 }