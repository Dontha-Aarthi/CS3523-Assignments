#include <iostream>
#include <fstream>
#include <cstdio>

using namespace std;

int main()
{
    ifstream input("inp-params.txt");
    ofstream log("RM-Log.txt");
    ofstream stats("RM-Stats.txt");

    //Reading number of processes
    int num_process;
    input >> num_process;

    int processID[num_process+1];
    int execTime[num_process+1];
    int period[num_process+1];
    int k[num_process+1];
    int waitingTime[num_process+1];
    int remainingExecTime[num_process+1];
    int real_time_exec[num_process+1];
    int remainingPeriod[num_process+1];
    int no_of_misses[num_process+1];
    int no_of_each_process[num_process+1];

    for(int h = 0; h <= num_process; h++)
    {
        no_of_misses[h] = 0;
        real_time_exec[h] = 0;
        waitingTime[h] = 0;
    }

    // Taking input and writing it into log file
    for(int i = 1; i <= num_process; i++)
    {
        input >> processID[i] >> execTime[i] >> period[i] >> k[i];
        remainingExecTime[i] = execTime[i];
        remainingPeriod[i] = period[i];
        no_of_each_process[i] = k[i];
        log << "Process" << processID[i] << ": processing time=" << execTime[i] << "; deadine:" <<
                     period[i] << "; period:" << period[i] << " joined the system at time 0.\n"; 
    }

    //Closing input file
    input.close();
    
    //Calculating total time 
    int time,total_time = (k[1]*period[1]);
    for(int i = 2; i <= num_process; i++)
    {
        if(total_time < k[i]*period[i])
            total_time = k[i]*period[i];
    }

    //scheduling the processes
    int execQueue[total_time];
    int next_process = 0 ,min_period;
    for(time = 0; time < total_time; time++)
    {
        min_period = total_time;
        next_process = 0;

        //Finding the process with least process and assigning its ID to next_process
        for(int i = 1; i <= num_process; i++)
        {
            if(remainingExecTime[i] > 0 && k[i] > 0)
            {
                if(min_period > period[i])
                {
                    next_process = i;
                    min_period = period[i];
                }
            } 
        }

        //Checking if next_process's execution time is completed and number of times executed is not less than or equal to 0
        if(remainingExecTime[next_process] > 0 && k[next_process] > 0)
        {
            execQueue[time] = next_process;
            cout << next_process <<" ";
            remainingExecTime[next_process] = remainingExecTime[next_process] - 1;
            real_time_exec[next_process]++;
        }
        
        //Calculating waiting time for processes who are not executing 
        for(int i = 1; i<=num_process; i++)
        {
            if(i!=next_process)
            {
                if(remainingExecTime[i]!=0 && remainingPeriod[i] != 0)
                {
                    waitingTime[i]++;
                }
            }
        }

        //Finding the next process to be executed
        for(int i = 1; i <= num_process; i++)
        {
            remainingPeriod[i]--;
            if((time+1) % period[i] == 0 && remainingPeriod[i] ==0 && k[i] > 1) 
            {
                if(real_time_exec[i] != execTime[i])
                {
                    no_of_misses[i]++;
                }
                remainingExecTime[i] = execTime[i];
                remainingPeriod[i] = period[i];
                real_time_exec[i] = 0;
                k[i]--; 
                next_process = i;
            }
        } 
    }

    //Calculating total number of processes that came into system and total number of misses
    int total_num_processes = 0, total_num_misses = 0;
    for(int j = 1; j<=num_process;j++)
    {
        total_num_processes += no_of_each_process[j];
        total_num_misses += no_of_misses[j];
    }
    
    float total_wait_time = 0;
    //Writing into stats file
    for(int i = 1; i<= num_process; i++)
    {
        stats << "Process" << processID[i] << ": " << endl;
        stats << "number of processes that came into system: " << no_of_each_process[i] << endl;
        stats << "Number of misses: "<< no_of_misses[i] << ".\n";
        stats << "number of processes that completed successfully: " << no_of_each_process[i] - no_of_misses[i] << endl;
        stats << "average waiting time: " << (float)waitingTime[i]/no_of_each_process[i] << endl << endl;
        total_wait_time += waitingTime[i];
    }
    
    stats << "\nTotal Number of processes that came into system is " << total_num_processes << endl;
    stats << "Total Number of processes that successfully completed are " << total_num_processes-total_num_misses << endl;
    stats << "Total Number of processes that missed their deadline are " << total_num_misses << endl;
    stats << "Total average waiting time is " << (float)total_wait_time/total_num_processes << endl;

    //Writing into log file by iterating over execQueue array
    int current_process = execQueue[0];
    int count[num_process+1];
    for(int h = 0; h <= num_process; h++)
    {
        count[h] = 0;
    }
    int i = 0;
    for(i = 0; i < total_time; i++)
    {
        if(current_process == execQueue[i])
        {
            count[current_process]++;
            time++;
        }
        else{
        if(current_process == 0)
        {
            log << "CPU is idle till "<< i << "\n";
            count[current_process] = 0;
            current_process = execQueue[i];
            count[current_process]++;
        }
        if(count[current_process] == execTime[current_process] && current_process != 0)
        {
            log << "Process" << current_process << ": starts execution at time " << i-count[current_process] << ".\n";
            log << "Process" << current_process << ": finishes execution at time " << i << ".\n";
            count[current_process] = 0;
            current_process = execQueue[i];
            count[current_process]++;
        }
        else if(count[current_process] != execTime[current_process]  && current_process != 0 && count[current_process] > 1 && execQueue[i] != 0)
        {
            log << "Process" << current_process << ": starts execution at time " << i-count[current_process] << ".\n";
            log << "Process" << current_process << ": is preempted by Process" << execQueue[i] << " at time " << i <<
                 " .Remaining processing time:" << execTime[current_process] - count[current_process] << endl;
            count[current_process] = 0;
            current_process = execQueue[i];
            count[current_process]++;
        }
        else if(count[current_process] != execTime[current_process]  && current_process != 0 && count[current_process] > 1 && execQueue[i] == 0)
        {
            log << "Process" << current_process << ": starts execution at time " << i-count[current_process]  << ".\n";
            log << "Process" << current_process << ": finishes execution at time " << i << ".\n";
            count[current_process] = 0;
            current_process = execQueue[i];
            count[current_process]++;
        }
        }
    }
    log << "CPU is idle \n";
    log << "Process" << execQueue[0] << ": starts execution at time " << i << ".\n";
    
    stats.close();
    log.close();
    return 0;
}
