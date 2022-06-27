#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

struct nearest_Point
{
	int x;
	int y;
	double distance;
};

/*We declare number of threads, number of points in set, points per thread,
source and destination coordinates and min_array GLOBALLY so that all 
threads can access easily.*/
int number_of_threads,number_of_points_in_set,points_per_thread;
int source_x,source_y;
int *destination_x, *destination_y;
struct nearest_Point* min_array;

/*This function extracts the x-coordinate from the string*/
int x_coordinate(char* string)
{
	int k = 1,m = 0;
	char *x;
	x = (char*)malloc(sizeof(char)*100);
	while(string[k]!=',')
	{
		x[m] = string[k];
		m++;
		k++;
	}

	return atoi(x);
}

/*This function extracts the y-coordinate from the string*/
int y_coordinate(char* string)
{
	int k = 1,n = 0;
	char *y;
	y = (char*)malloc(sizeof(char)*100);
	for(int m = 0; m < strlen(string); m++)
		if(string[m]==',')
		{
			k = m;
			break;
		}
	
	k++;
	while(string[k]!=')')
	{
		y[n] = string[k];
		n++;
		k++;
	}

	return atoi(y);
}

/*This function calculates distances and finds minimum among them*/
struct nearest_Point nearest_point(int start, int end)
{
	int number_of_points = end-start+1;
	double distance_array[number_of_points];

	for(int i = 0; i < number_of_points; i++)
		distance_array[i] = sqrt(pow((source_x-destination_x[start+i]),2)+pow((source_y-destination_y[start+i]),2));
	
	double min = distance_array[0];
	int min_index = 0;

	for(int j = 0; j < number_of_points; j++)
		if(min>distance_array[j])
		{
			min = distance_array[j];
			min_index = j;
		}

	struct nearest_Point nearestPoint;
	nearestPoint.x = destination_x[start+min_index];
	nearestPoint.y = destination_y[start+min_index];
	nearestPoint.distance = min;

	return nearestPoint;
}

/* this function is called by the thread, it intialises the values for start, 
and end for each thread*/
void* distance(void* args)
{
	int i = (int)(long)(args);
	int start = i*(points_per_thread);
	int end = ((i+1)*(points_per_thread)-1);

	if(end > number_of_points_in_set - 1)
		end = number_of_points_in_set - 1;
	
	if(i == number_of_threads - 1)
		end = number_of_points_in_set - 1;
	
	min_array[i] = nearest_point(start,end);

	pthread_exit(0);
}


int main()
{
	FILE* input = fopen("input.txt", "r");
	FILE* output = fopen("output.txt", "w");

	if(!input)
	{
		fprintf(stderr,"File has not opened!\n");
		return 1;
	}

	char source[100];
	
	fscanf(input, "%d", &number_of_threads); 
	fscanf(input, "%s", source); 
	fscanf(input, "%d", &number_of_points_in_set); 

	points_per_thread = number_of_points_in_set/number_of_threads;

	char destination_points[number_of_points_in_set][100];

	for(int i = 0; i < number_of_points_in_set; i++)
	{
		fscanf(input, "%s", destination_points[i]);
	}

	destination_x = (int*)malloc(number_of_points_in_set*sizeof(int));
	destination_y = (int*)malloc(number_of_points_in_set*sizeof(int));

	source_x = x_coordinate(source);
	source_y = y_coordinate(source);

	for(int i = 0; i < number_of_points_in_set; i++)
	{
		destination_x[i] = x_coordinate(destination_points[i]);
		destination_y[i] = y_coordinate(destination_points[i]);
	}

	clock_t clock_start = clock();

	pthread_t tid[number_of_threads];
	pthread_attr_t attr[number_of_threads];
	
	min_array = (struct nearest_Point*)malloc(number_of_threads*sizeof(struct nearest_Point));

	for(int i = 0; i < number_of_threads; i++)
	{
		pthread_attr_init(&attr[i]);
		pthread_create(&tid[i], &attr[i], distance, (void*)(long)(i));
	}

	for(int i = 0; i < number_of_threads; i++)
		pthread_join(tid[i],NULL);
	
	/*Here The main thread finds the nearest point from the data obtained from all worker threads*/
	struct nearest_Point required_point = min_array[0];

	for(int j = 1; j < number_of_threads; j++)
		if(required_point.distance > min_array[j].distance)
			required_point = min_array[j];
	
	clock_t clock_end = clock();
	
	/*Printing output*/
	printf("nearest point is : (%d,%d)\n",required_point.x,required_point.y);
	
	unsigned int time_taken_microsec = (float)(clock_end-clock_start)/CLOCKS_PER_SEC*1000000;
	printf("time taken is %d microseconds.\n",time_taken_microsec);

	return 0;
}
