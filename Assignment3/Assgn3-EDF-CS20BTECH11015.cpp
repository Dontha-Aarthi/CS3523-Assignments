#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <numeric>
#include <vector>

using namespace std;

int main()
{
	string processes;
	int num_processes = 0;

	ifstream inputFile("inp-params.txt");
	ofstream log("EDF-Log.txt");
	ofstream stats("EDF-Stats.txt");

	//Reading number of processes from inp-params.txt
	getline(inputFile, processes);
	stringstream temp(processes);
	temp >> num_processes;
	
	//intialising arrays for processes to store ID, execution time, period, deadline and k
	int processID[num_processes+1];
	int executionTime[num_processes+1];
	int period[num_processes+1];
	int deadline[num_processes+1];
	int k[num_processes+1];
	int times[num_processes+1];
	int missed[num_processes+1] = {0};
	int waiting_time[num_processes+1] = {0};
	int num_of_each_process[num_processes+1];
	string processInfo;

	//Reading process id, execution time, period and k values from inp-params.txt
	int i = 1; 
	while(getline(inputFile, processInfo))
	{		
		stringstream linestream(processInfo);
		linestream >> processID[i] >> executionTime[i] >> period[i] >> k[i];
		i++;
	}
	inputFile.close();	

	//assigning the value to process deadline same as its period and copying k value to num_of_each_process array, so that k can be modified.
	for(int j = 1; j <= num_processes; j++)
	{
		deadline[j] = period[j];
		num_of_each_process[j] = k[j];
	}

	//writing process information into log file
	for(int j = 1; j <= num_processes; j++)
	{
		log << "process" << processID[j] << ": processing time=" << executionTime[j] << 
				"; deadline:" << deadline[j] << "; period:" << period[j] << " joined the system at time 0.\n";
	}

	//Calculating the total period for each process. 
	for(int j = 1; j <= num_processes; j++)
	{
		times[j] = k[j]*period[j];
	}
	
	// finding the nax total period 
	int max_periodtime = times[1]; 
	for(int j = 2; j <= num_processes; j++)
	{
		if(times[j] > max_periodtime)
		{
			max_periodtime = times[j];
		}
	}
	
	//some new variables used for scheduling the processes
	int earliestDeadline, next_process;
	vector <int> execQueue;
	int remainingExecTime[num_processes+1];
	int nextDeadline[num_processes+1];
	int realTimePeriod[num_processes+1] = {0};

	//creating arrays with deadline, execution time which can be modified at the time of scheduling.
	for(i = 1 ; i <= num_processes ; i++)
	{
		nextDeadline[i] = deadline[i];
		remainingExecTime[i] = executionTime[i];
	}
	
	//scheduling the processes for each unit time till max total period time.
	for(i = 0; i < max_periodtime; i++)
	{				
		//getting the process with earliest deadline,
		earliestDeadline = max_periodtime;
		next_process = -1;
		for(int j = 1; j <= num_processes; j++)
		{
			if(remainingExecTime[j] > 0 && k[j] > 0)
			{
			 	if(earliestDeadline > nextDeadline[j])
			 	{					 		
					earliestDeadline = nextDeadline[j];
					next_process = j;
				}
			}
		}
		//pushing back the process ID with earliest deadline in the queue.
		execQueue.push_back(next_process);

		//decreasing the execution time of the above pushed process
		remainingExecTime[next_process]--;

		//calculating waiting time 
		for(int p = 1; p <= num_processes; p++)
		{
			if(p != next_process && remainingExecTime[p] != 0 && (realTimePeriod[p]-1) != period[p] && k[p] > 0) //  nextDeadline[p] != 0
				waiting_time[p]++;
		}

		//if the execution time of that process becomes 0, we decrease the k value by 1.
		if(remainingExecTime[next_process] == 0)
		{
			k[next_process]--;
		}

		//Calculating misses of process
		if(remainingExecTime[next_process] != 0 && (nextDeadline[next_process] == 1) )
		{
			k[next_process]--;
			missed[next_process]++;
		}
		for(int p = 1; p <= num_processes; p++)
		{
			if( p != next_process && remainingExecTime[p] != 0 && nextDeadline[p] <= 1 && k[p] > 0)
			{
				missed[p]++;
				k[p]--;
			}
		}
		
		//finding next earliest deadline process
		for(int j=1 ; j<=num_processes ; j++)
		{
			if(realTimePeriod[j] == (period[j] - 1))
			{
			 	nextDeadline[j] =  deadline[j];
			 	remainingExecTime[j] = executionTime[j];
			 	realTimePeriod[j] = 0;
			}
			else
			{
			 	if(nextDeadline[j] > 0)
			 	{
			 		nextDeadline[j]--;
			    }
				realTimePeriod[j]++; 
			} 
		}
	}
	//Writing into stats file
	int total_misses = 0, total_waiting = 0, total_processes = 0;
	for(int j = 1; j <= num_processes; j++)
	{
		stats << "Process " << processID[j] << ":\n";
		stats << "number of processes that came into system: " << num_of_each_process[j] << endl;
		stats << "number of misses: " << missed[j] << endl;
		stats << "number of processes that completed successfully: " << num_of_each_process[j] - missed[j] << endl;
		stats << "average waiting time: " << (float)waiting_time[j]/num_of_each_process[j] << endl << endl;
		total_misses += missed[j];
		total_processes += num_of_each_process[j];
		total_waiting += waiting_time[j];
	}
	
	stats << "total number of processes that came into system is " << total_processes << endl;
	stats << "total number of processes that completed successfully is " << total_processes-total_misses << endl;
	stats << "total number of processes that missed their deadline are " << total_misses << endl;
	stats << "total average waiting time is " << (float)total_waiting/total_processes << endl;
	
	//Writing into log file by iterating over the execQueue vector
	int count[num_processes+1];
	for(int k = 1; k <= num_processes; k++)
	{
		count[k] = 0;
	}

	int process_id = execQueue.front();
	log << "\nprocess p" << process_id << ": starts execution at time 0\n";

	int time = 0;
	for (auto it = execQueue.begin(); it != execQueue.end(); ++it)
    {
		cout << *it << " ";
    	if(*it == process_id && executionTime[process_id] != count[process_id])
    	{
    		count[process_id]++;
    		time++;
    	}
    	else 
       	{
       		if(process_id != -1)
			{        		
    			log << "process p" << process_id;
    		}
    		if(count[process_id] == executionTime[process_id] && process_id != -1)
    		{
    			log << ": finishes execution at time " << time << endl;
    			count[process_id] = 0;
    		}
    		else if(count[process_id] < executionTime[process_id] && process_id != -1)
    		{
    			log << ": is preempted by process p" << *(it+1) << " at time " << time << ". Remaining processing time: " << executionTime[process_id] - count[process_id] << endl;
			}
    		else if(process_id == -1)
    		{
    			log << "CPU remains idle till " << time << endl ;
    		}

    		process_id = *(it+1);
    		count[process_id]++;
    		time++;

    		if(process_id != -1)
			{
    			log << "process p" << process_id << ": starts execution at time " << time << endl;
    		}	
    	}
    }
	
    if(process_id == -1)
    {
    	log << "CPU remains idle till " << time << endl ;
    }	
    log << "process p" << execQueue.front() << " starts execution at " << time << endl;

	
	//Closing files
    log.close();
	stats.close();	
	return 0;
}