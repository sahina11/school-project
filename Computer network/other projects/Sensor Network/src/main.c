#include <stdio.h>
#include <string.h>
#include <stdlib.h> //malloc
#include <math.h>
#include "library.h"
#include "support.c"
#include "parse.c"

int main(int argc, char const *argv[])
{

	int size, sRange, cRange, powerCapacity, numPath, mode;

	
	size = getNumFromLine();
	if (COUT) printf("size :%d\n", size);
	if(size ==-1){
		printf("wrong input! Quit right now\n");
		return -1;
	}

	sRange = getNumFromLine();
	if (COUT) printf("sensing range :%d\n", sRange);
	if(sRange ==-1){
		printf("wrong input! Quit right now\n");
		return -1;
	}

	cRange = getNumFromLine();
	if (COUT) printf("commu range :%d\n", cRange);
	if(cRange ==-1){
		printf("wrong input! Quit right now\n");
		return -1;
	}
	powerCapacity = getNumFromLine();
	if (COUT) printf("power capacity :%d\n", powerCapacity);
	if(powerCapacity ==-1){
		printf("wrong input! Quit right now\n");
		return -1;
	}
	numPath = getNumFromLine();
	if (COUT) printf("number of path :%d\n", numPath);
	if(numPath ==-1){
		printf("wrong input! Quit right now\n");
		return -1;
	}

	mode = getMode();
	if (mode == DISJOINT){
		if (COUT) printf("mode: disjoint\n");}
	else{
		if (COUT) printf("mode: braided\n");}

	if (sRange>cRange){
		printf("Sensing range > Communication range. Exit right now!\n");
		return -1;
	}


	struct cordinate *APlist;
	struct cordinate *Slist;
	APlist = makeCordinateList();
	Slist = makeCordinateList();

	struct event *ev;
	ev = makeEventList();


	int arraySize;
	arraySize = getLen(APlist) + getLen(Slist);

	struct vertex *graph[arraySize]; //array of pointers
	int i;
	for(i =0; i<arraySize; i++){
		graph[i] = calloc(1,sizeof(struct vertex));
	}

	struct cordinate *ptr;
	ptr = APlist;

	for(i = 0; i< getLen(APlist); i++){
		graph[i]->kind = AP;
		graph[i]->nameNum = i;
		graph[i]->power = 999999;
		graph[i]->location = ptr;
		ptr = ptr->next;
		graph[i]->neighbor =NULL;
		
	}

	ptr = Slist;
	for(i = 0+getLen(APlist); i<arraySize; i++){
		if (size < ptr->x || size < ptr->y){
			printf("wrong cordinate in wrong size! Exit right now\n");
			return -1;
		}

		graph[i]->kind = SENSOR;
		graph[i]->nameNum = i;
		graph[i]->power = powerCapacity;
		graph[i]->location = ptr;
		ptr = ptr->next;
		graph[i]->neighbor =NULL;
		
	}

	for (i =0;i<arraySize;i++){ //initialized queue
		graph[i]->neighbor = newQ();
	}


	int j;
	for	(i=0;i<arraySize-1;i++){ //building neighbor relationships
		for (j=i+1;j<arraySize;j++){
			if ( isNeighbor(graph[i],graph[j],cRange) ){
				enqueue(graph[i]->neighbor,j);
				enqueue(graph[j]->neighbor,i);
			}
		}
	}


	if (COUT){
		printf("vertex array with neighbors:\n");
		for(i =0;i<arraySize;i++){
			printVertex(graph[i]);
		}	
		printf("\n");//print vetex array
	}


	//BFS start from here...-------------------------------------------
	struct event *eventNow;
	eventNow = ev;

	while(eventNow != NULL){
		int powerCons; 
		powerCons = eventNow->powerCons;

		struct queue *sensorQ;
		sensorQ = newQ(); //would be the source queue

		for(i=getLen(APlist);i<arraySize;i++){ //building sensor pool
		/*two condition
			1.should ba within the sensing range,
		 	2.power should be enough*/
			if (isAround(eventNow, graph[i], sRange) &&
				graph[i]->power >= powerCons){
				enqueue(sensorQ,i);
			}
		}

		// if (COUT){
		// 	printEvent(eventNow);
		// 	printf("sensorQ:");
		// 	printQ(sensorQ);
		// 	printf("\n");}



		int activeNode[arraySize]; //for Disjoint
		for(i=0;i<arraySize;i++){ 
			activeNode[i] = 1;
		}

		struct queue *kpathPool[numPath];
		int kindex =0;
		
		int k;
		for(k=0;k<numPath;k++){ //get you kpathpool of k or lesser shortest path
			int source;
			struct queue *pathPool[PATHARRAYSIZE];
			int index = 0;
			
			for (i=0;i<arraySize;i++){
				if (!isPowerful(graph[i], powerCons)){
					activeNode[i]=0;
				}
			}

			for(j=0;j<qLen(sensorQ);j++){

				source = sensorQ->array[j];
				if(activeNode[source] == 0) continue;
				//use this to eliminate min ==2, same sensor again situation ////

				

				int visit[arraySize];
				int prev[arraySize];

				for(i=0;i<arraySize;i++){ //prepare
					visit[i] = 0;
					prev[i] = -1;
				}

				struct queue *bfsQ; //q for bfs
				bfsQ = newQ();

				enqueue(bfsQ,source);
				visit[source] = 1;

				while(bfsQ->fullOrEmpty != -1){ //bfs, produce prev array

					int visiting = dequeue(bfsQ);

					for(i=0;i<qLen(graph[visiting]->neighbor);i++){
						int neighbor = graph[visiting]->neighbor->array[i];
						if (visit[neighbor] == 0 && activeNode[neighbor] ==1){
							enqueue(bfsQ,neighbor);
							visit[neighbor] = 1;
							prev[neighbor] = visiting;
						}
					}
				}

				for (i=0;i<getLen(APlist);i++){
					struct queue *onePath;
					onePath = newQ();

					if (prev[i] == -1) continue; //didn't reach this AP

					int k;
					k = i; //i == destination AP

					while(k != source){
						enqueue(onePath,k);
						k = prev[k];
					}
					enqueue(onePath, source);
					pathPool[index] = onePath; //output path
					index ++; //output path to diff AP to pathPool	
				} //get me sensor * AP pathpool

			}
			
			// for(i=0;i<index;i++){
			// 	printPath(pathPool[i]);
			// }
			// printf("\n");

			struct queue *shortestPath;
			int min;
			min = qLen(pathPool[0]);
			
			for(i=1;i<index;i++){ //find the # of min hops
				if (qLen(pathPool[i]) < min){
					min = qLen(pathPool[i]);
				}
			}

			for(i=0;i<index;i++){
				if (qLen(pathPool[i]) == min){
					shortestPath = pathPool[i];
					break;
				}
			}
			
			kpathPool[kindex] = shortestPath;
			kindex++;

			if (min > 2){			//update activeNode
				for(i=1;i<qLen(shortestPath)-1;i++){
					activeNode[ shortestPath->array[i] ] =0;
				}
			}
			else if (min == 2){
				activeNode[shortestPath->array[qLen(shortestPath)-1]] =0;
			}
		}

		if (COUT){
			for(i=0;i<kindex;i++){
				printPath(kpathPool[i]);
			}
		}

		//choose one path for real routing
		if (kindex ==0){
			printf("Failed to route this event:");
			printEvent(eventNow);
			printf("\n");

			eventNow = eventNow->next;
			continue;
		}

		int bottleneck[kindex];

		for(i =0;i<kindex;i++){
			int min;
			for(j=1;j<qLen(kpathPool[i]);j++){
				min = graph[ kpathPool[i]->array[0] ]->power;
				if (graph[ kpathPool[i]->array[j] ]->power < min){
					min = graph[ kpathPool[i]->array[j] ]->power;
				}
			}
			bottleneck[i] = min;
		}

		if(COUT){
			for(i=0;i<kindex;i++){
				printf("%d\t", bottleneck[i]);
			}	
		}
		
		int max;
		max = bottleneck[0];
		for (i=1;i<kindex;i++){
			if(bottleneck[i]>max){
				max = bottleneck[i];
			}
		}
		
		int choice;
		for (i=0;i<kindex;i++){ //choice decided
			if(max == bottleneck[i]){
				choice = i;
				break;
			}

		}

		// printf("the choice is %d\n", choice);

		for(i=1;i<qLen(kpathPool[choice]);i++){
			 graph[ kpathPool[choice]->array[i] ]->power -=powerCons;
		}

		printf("for event:");
		printEvent(eventNow);
		printf("\n");
		for(i=0;i<kindex;i++){
			for (j = qLen(kpathPool[i])-1; j>=0; j--){
				printf("->");
				printCordinate(graph[ kpathPool[i]->array[j] ]->location);
				printf("::%d ",graph[ kpathPool[i]->array[j] ]->power );
			}
			printf("\n");
		}
		printf("\n");

		eventNow = eventNow->next;
	}
	


	return 0;
}











