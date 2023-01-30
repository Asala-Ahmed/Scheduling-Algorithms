#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <queue>
#include <iomanip>
#include <math.h>
#include <vector>

using namespace std;

typedef struct
{
    char name;
    int arrival_time=0,turnaround_time=0,finish_time=0,response_time=0,waiting_time=0;
    int service_time=0,running_time=0,lastexist=0;
    float normalized_turnaround=0.0,ratio=0.0;
    int initial_priority=0,current_priority=0;
    int index;
    vector<char> trace;
} Process;

Process Processes[256];
char mode[6];
vector<int> type,quantum;
int lastInstant,numofProcesses,numofPolicies=0;
char policies[9][6]={"","FCFS","RR","SPN","SRT","HRRN","FB-1","FB-2i","Aging"};

queue<Process> FCFS(queue<Process> ready_queue)
{
    int clock=0;
    Process p;
    queue<Process> completion;
    while(clock < lastInstant)
    {
        while(!ready_queue.empty())
        {
            p=ready_queue.front(); //pop the top of q which has arrived first
            while(clock < p.arrival_time) // loop while the clock is less than the arrival time of the process at the top of the ready queue
            {
                clock++;
            }
            ready_queue.pop();
            p.response_time=clock-p.arrival_time; //calculate the response time
            for(int r=p.arrival_time;r<p.arrival_time+p.response_time;r++) //set state to indicate waiting
            {
                p.trace[r]='.';
            }
            while(p.running_time!=0)
            {
                p.running_time--;
                p.trace[clock++]='*';
            }
            p.finish_time=clock;
            p.turnaround_time=(p.finish_time-p.arrival_time);
            p.normalized_turnaround=(p.turnaround_time*1.0)/p.service_time;
            p.waiting_time=(p.turnaround_time-p.service_time);
            completion.push(p); //process finished and left the system
        }
    }
    return completion;
}

queue<Process> RoundRobin(queue<Process> ready_queue,int quantum)
{
    int clock=0;
    Process p,ready;
    queue<Process> completion,temp;
    Process com[256];
    while(clock < lastInstant)
    {
        while(!ready_queue.empty() && ready_queue.front().arrival_time<=(clock+quantum))
        { //get processes that has arrived at the moment or before that
            ready=ready_queue.front();
            ready_queue.pop();
            temp.push(ready);
        } 
        if(!temp.empty())
        {
            p=temp.front(); //pop the top of q   
            if(p.arrival_time<=clock){
                temp.pop();
                if(p.lastexist) //check if process returns after quantum or serviced for the first time
                {
                    for(int r=p.lastexist;r<clock;r++)//set state to indicate waiting
                    {
                        p.trace[r]='.';
                    }
                }else{
                    for(int r=p.arrival_time;r<clock;r++)//set state to indicate waiting
                    {
                        p.trace[r]='.';
                    }
                }              
                if(p.running_time<=quantum) //process may not need all provided quantum, finishes early and leave system
                {
                    for(int r=clock;r<clock+p.running_time;r++)
                    {
                        p.trace[r]='*';
                    }
                    clock+=p.running_time;
                    p.running_time=0;
                    p.finish_time=clock;
                    p.turnaround_time=p.finish_time-p.arrival_time;
                    p.normalized_turnaround=(p.turnaround_time*1.0)/p.service_time;
                    p.waiting_time=(p.turnaround_time-p.service_time);
                    com[p.index]=p; //process finished and left the system
                }
                else
                {
                    for(int r=clock;r<clock+quantum;r++) //process runs for the whole quantum provided
                    {
                        p.trace[r]='*';
                    }
                    p.running_time-=quantum;
                    clock+=quantum;
                    p.lastexist=clock;
                    temp.push(p); //process reenters queue to finish remaining service time
                }
            }else
               clock++;
        }
    }
    for(int i=0;i<numofProcesses;i++)
    {
        completion.push(com[i]);
    }
    return completion;
}

queue<Process> SPN(queue<Process> ready_queue)
{
    int clock=0 ;
    Process current[256],ready,p;
    queue<Process> completion,temp;
    Process com[256];
    while(clock < lastInstant)
    {
        int index=0;
        while(!ready_queue.empty() && ready_queue.front().arrival_time<=clock)
        { //get processes that has arrived at the moment or before that
            ready=ready_queue.front();
            ready_queue.pop();
            temp.push(ready);
        } 
        while(!temp.empty())
        {
            current[index]=temp.front();
            temp.pop();
            index++;
        }
        if(index>1){
            for(int i=0;i<index-1;i++) //sort current array according to service time
            {
                for(int j=0;j<index-i-1;j++)
                {
                    if (current[j].service_time > current[j+1].service_time)
                    {
                        ready=current[j];
                        current[j]=current[j+1];
                        current[j+1]=ready;
                    }
                }
            }
        }
        if(index){
            for(int i=0;i<index;i++)
            {
                temp.push(current[i]);
            }
        }
        if(!temp.empty())
        {
            p=temp.front();//run process with shortest service time until it finishes and exit the system
            temp.pop();
            p.response_time=clock-p.arrival_time; //calculate the response time
            for(int r=p.arrival_time;r<p.arrival_time+p.response_time;r++)
            {
                p.trace[r]='.'; //set state to indicate waiting
            }
            while(p.running_time!=0)
            {
                p.running_time--;
                p.trace[clock++]='*';
            }
            p.finish_time=clock;
            p.turnaround_time=(p.finish_time-p.arrival_time);
            p.normalized_turnaround=(p.turnaround_time*1.0)/p.service_time;
            p.waiting_time=(p.turnaround_time-p.service_time);
            com[p.index]=p; //process finished and left the system
        }
    }
    for(int i=0;i<numofProcesses;i++)
    {
        completion.push(com[i]);
    }
    return completion;
}

queue<Process>  SRT(queue<Process> ready_queue)
{
    int clock=0;
    Process p,ready;
    queue<Process> completion,temp;
    Process com[256],current[256];
    while(clock < lastInstant)
    {
        int index=0;
        while(!ready_queue.empty() && ready_queue.front().arrival_time<=clock)
        { //get processes that has arrived at the moment or before that
            ready=ready_queue.front();
            ready_queue.pop();
            temp.push(ready);
        }
        while(!temp.empty())
        {
            current[index]=temp.front();
            temp.pop();
            index++;
        }
        if(index>1){
            for(int i=0;i<index-1;i++) //sort current array according to remainig service time
            {
                for(int j=0;j<index-i-1;j++)
                {
                    if (current[j].running_time > current[j+1].running_time)
                    {
                        ready=current[j];
                        current[j]=current[j+1];
                        current[j+1]=ready;
                    }
                }
            }
        }
        if(index){
            for(int i=0;i<index;i++)
            {
                temp.push(current[i]);
            }
        } 
        if(!temp.empty())
        {
            p=temp.front(); //pop the top of q which has least remaining service time   
            if(p.arrival_time<=clock){
                temp.pop();
                if(p.lastexist) //check if process returns after quantum or serviced for the first time
                {
                    for(int r=p.lastexist;r<clock;r++)//set state to indicate waiting
                    {
                        p.trace[r]='.';
                    }
                }else{
                    for(int r=p.arrival_time;r<clock;r++)//set state to indicate waiting
                    {
                        p.trace[r]='.';
                    }
                }              
                if(p.running_time!=0)
                {
                    p.running_time--;
                    p.trace[clock]='*';
                    clock++;
                    p.lastexist=clock;              
                    if(p.running_time!=0)//check if process still needs more service time
                    {
                        temp.push(p);
                    }
                    else
                    {
                        p.finish_time=clock;
                        p.turnaround_time=(p.finish_time-p.arrival_time);
                        p.normalized_turnaround=(p.turnaround_time*1.0)/p.service_time;
                        com[p.index]=p; //process finished and left the system
                    }
                }
            }else
               clock++;
        }
    }
    for(int i=0;i<numofProcesses;i++)
    {
        completion.push(com[i]);
    }
    return completion;
}

queue<Process> HRRN(queue<Process> ready_queue)
{
    int clock=0 ;
    Process current[256],ready,p;
    queue<Process> completion,temp;
    Process com[256];
    while(clock < lastInstant)
    {
        int index=0;
        while(!ready_queue.empty() && ready_queue.front().arrival_time<=clock)
        { //get processes that has arrived at the moment or before that
            ready=ready_queue.front();
            ready_queue.pop();
            temp.push(ready);
        } 
        while(!temp.empty()) //calculation of highest response ration
        {
            current[index]=temp.front();
            current[index].waiting_time=clock-current[index].arrival_time;
            current[index].ratio=(current[index].waiting_time+current[index].service_time)/current[index].service_time;
            temp.pop();
            index++;
        }
        if(index>1){
            for(int i=0;i<index-1;i++) //sort current array according to highest response ratio
            {
                for(int j=0;j<index-i-1;j++)
                {
                    if (current[j].ratio < current[j+1].ratio)
                    {
                        ready=current[j];
                        current[j]=current[j+1];
                        current[j+1]=ready;
                    }
                }
            }
        }
        if(index){
            for(int i=0;i<index;i++)
            {
                temp.push(current[i]);
            }
        }
        if(!temp.empty())
        {
            p=temp.front();
            temp.pop();
            p.response_time=clock-p.arrival_time; //calculate the response time
            for(int r=p.arrival_time;r<p.arrival_time+p.response_time;r++)
            {
                p.trace[r]='.'; //set state to indicate waiting
            }
            while(p.running_time!=0)
            {
                p.running_time--;
                p.trace[clock++]='*';
            }
            p.finish_time=clock;
            p.turnaround_time=(p.finish_time-p.arrival_time);
            p.normalized_turnaround=(p.turnaround_time*1.0)/p.service_time;
            p.waiting_time=(p.turnaround_time-p.service_time);
            com[p.index]=p; //process finished and left the system
        }
    }
    for(int i=0;i<numofProcesses;i++)
    {
        completion.push(com[i]);
    }
    return completion;
}

queue<Process> Feedback(queue<Process> ready_queue,int quantum)
{
    queue<Process> RQ[5];
    int clock=0,i=0,k;
    int quans[5];
    Process p,ready;
    queue<Process> completion;
    Process com[256];
    if(quantum)
    {
        for(int q=0;q<5;q++) //all queues have same quantum in case of feedback-1
        {
            quans[q]=1;
        }
    }else{
        for (int q = 0; q < 5; q++) //calculate quantum for each queue in case of feedback-21
        {
            quans[q]=pow(2,q);
        }
        
    }
    while(clock < lastInstant)
    {
        while(!ready_queue.empty() && ready_queue.front().arrival_time<=clock)
        { //get processes that has arrived at the moment or before that
            ready=ready_queue.front();
            ready_queue.pop();
            RQ[0].push(ready);
        } 
        i=0;
        while(RQ[i].empty() && i<5) //check for first queue thats not empty
        {
            i++;
        }
        if(!RQ[i].empty() && i<5)
        {
            p=RQ[i].front(); //pop the top of q which has arrived first   
            if(p.arrival_time<=clock)
            {
                RQ[i].pop();
                if(p.lastexist)//check if process returns after quantum or serviced for the first time
                {
                    for(int r=p.lastexist;r<clock;r++)//set state to indicate waiting
                    {
                        p.trace[r]='.';
                    }
                }else{
                    for(int r=p.arrival_time;r<clock;r++)//set state to indicate waiting
                    {
                        p.trace[r]='.';
                    }
                }              
                if(p.running_time<=quans[i]) //process may not need all provided quantum, finishes early and leave system
                {
                    for(int r=clock;r<clock+p.running_time;r++)
                    {
                        p.trace[r]='*';
                    }
                    clock+=p.running_time;
                    p.running_time=0;
                    p.finish_time=clock;
                    p.turnaround_time=p.finish_time-p.arrival_time;
                    p.normalized_turnaround=(p.turnaround_time*1.0)/p.service_time;
                    p.waiting_time=(p.turnaround_time-p.service_time);
                    com[p.index]=p; //process finished and left the system
                }
                else
                {
                    for(int r=clock;r<clock+quans[i];r++) //process runs for the whole quantum provided
                    {
                        p.trace[r]='*';
                    }
                    p.running_time-=quans[i];
                    clock+=quans[i];
                    p.lastexist=clock;
                    if(i<4) //check if process in least priority queue or not
                    {
                        k=0;
                        while(RQ[k].empty() && k<5) //check all queues if empty
                        {
                            k++;
                        }
                        if(k==5 && ready_queue.front().arrival_time>clock) //if system empty and no new process will come in the next instance, process remains in same priority queue
                            RQ[i].push(p);
                        else
                            RQ[i+1].push(p); //process pushed to lower priority queue
                    }                         
                    else if(i==4) //process in least priority queue stays in the same queue
                        RQ[i].push(p);    
                }
            }else
                clock++;        
        }
    }
    for(int i=0;i<numofProcesses;i++)
    {
        completion.push(com[i]);
    }
    return completion;
}

queue<Process> Aging(queue<Process> ready_queue,int quantum)
{
    int clock=0,readycount=0;
    Process ready,p,incrementing;
    queue<Process> completion,temp;
    Process highest_priority[256],result[256];
    while (clock < lastInstant)
    {
        if(p.lastexist) //check if there is a process running
            p.current_priority=p.initial_priority;//priority of current process set to its intial priority
        while(!ready_queue.empty() && ready_queue.front().arrival_time<=clock)
        { //get processes that has arrived at the moment or before that
            ready=ready_queue.front();
            ready_queue.pop();
            temp.push(ready);
        }
        for(int i=0;i<temp.size();i++) //incrementing priority of all ready processes
        {
            incrementing=temp.front();
            temp.pop();
            incrementing.current_priority++;
            temp.push(incrementing);
        }
        if(p.lastexist) //preemption
            temp.push(p);
        readycount=0;
        while(!temp.empty())
        {
            highest_priority[readycount]=temp.front();
            temp.pop();
            readycount++;
        }
        if(readycount>1){
            for(int i=0;i<readycount-1;i++) //sort current array according to current priority
            {
                for(int j=0;j<readycount-i-1;j++)
                {
                    if (highest_priority[j].current_priority < highest_priority[j+1].current_priority)
                    {
                        ready=highest_priority[j];
                        highest_priority[j]=highest_priority[j+1];
                        highest_priority[j+1]=ready;
                    }
                }
            }
        }
        if(readycount)
        {
            for(int i=0;i<readycount;i++)
            {
                temp.push(highest_priority[i]);
            }
        }
        if(!temp.empty())
        {
            p=temp.front(); //process of highest priority in ready processes
            if(p.arrival_time<=clock)
            {
                temp.pop();
                if(p.lastexist) //check if process returns after quantum or serviced for the first time
                {
                    for(int r=p.lastexist;r<clock;r++)//set state to indicate waiting
                    {
                        p.trace[r]='.';
                    }
                }else{
                    for(int r=p.arrival_time;r<clock;r++)//set state to indicate waiting
                    {
                        p.trace[r]='.';
                    }
                }
                for(int r=clock;r<clock+quantum;r++) //process will run for the specified quantum
                {
                    p.trace[r]='*';
                }
                clock+=quantum;
                p.lastexist=clock;
            }
            else
                clock++;
            
        }
    }
    temp.push(p);//preemption of last running process
    while(!temp.empty()) //store all processes in queue in their initial position
    {
        p=temp.front();
        temp.pop();
        int startwait=p.lastexist?p.lastexist:p.arrival_time;
        for(int r=startwait;r<clock;r++) //set state to indicate waiting since last time process was serviced 
        {
            p.trace[r]='.';
        }     
        result[p.index]=p;
    }
    while(!ready_queue.empty())//store all processes in queue in their initial position
    {
        p=ready_queue.front();
        ready_queue.pop();
        result[p.index]=p;
    }
    for(int i=0;i<numofProcesses;i++)
    {
        completion.push(result[i]);
    }
    return completion;   
}

void printattribute(int value)
{
    int len,tpad,lpad,rpad;
    len=to_string(value).length();
    tpad=5-len; //calculate remaining spaces relative to the value to be printed
    if(tpad%2 == 0) //calculate left spaces
    {
        lpad=tpad/2;
    }
    else{
        lpad=tpad%2;
    }
    rpad=tpad-lpad; //calculate right spaces
    cout << string(lpad,' ');
    printf("%d",value);
    cout << string(rpad,' ');
    printf("|");
}

void printattributefloat(float value)
{
    int len,tpad,lpad,rpad;
    len=to_string(value).substr(0,to_string(value).find(".")+3).length();
    tpad=5-len; //calculate remaining spaces relative to the value to be printed
    if(tpad%2 == 0)//calculate left spaces
    {
        lpad=tpad/2;
    }
    else{
        lpad=tpad%2;
    }
    rpad=tpad-lpad; //calculate right spaces
    cout << string(lpad,' ');
    printf("%.2f",value);
    cout << string(rpad,' ');
    printf("|");
}

void print(queue<Process> final,char policy[],int quantum,int type){
    Process proc;
    float turnMean=0,normMean=0;
    int iterations,remaining;
    int totalchar,space=6;
    int quandigits;
    printf("%s",policy);
    if(quantum && type==2) //only round robin need printing of its quantum
    {
        printf("-%d",quantum);
        quandigits=to_string(quantum).length();
        space=space-1-quandigits; //calculate how many spaces left between name of policy and start of timeline
    }
    if(!strcmp(mode,"trace"))
    {
        space-=strlen(policy);
        for(int s=0;s<space;s++)
        {
            printf(" ");
        }
        totalchar=(2*lastInstant)+8;
        char timeline[21];
        strcpy(timeline," 1 2 3 4 5 6 7 8 9 0");
        iterations=lastInstant/10; //calculate timeline whole(integer) instances
        remaining=lastInstant%10;  //calculate timeline remaining(digits)
        
        printf("0");
        for(int c=0;c<iterations;c++) 
        {
            printf("%s",timeline);
        }
        for(int c=0;c<2*remaining;c++)
        {
            printf("%c",timeline[c]);
        }
        printf(" \n");
        /* for(int d=0;d<totalchar;d++) //dynamic dash line printing
        {
            printf("-");
        } */
        printf("------------------------------------------------"); //static dash line printing
        printf("\n");
        for (int j = 0; j < numofProcesses; j++)
        {
            proc=final.front();
            printf("%c     |",proc.name);
            for(int k=0;k<lastInstant;k++)
            {
                printf("%c|",proc.trace[k]==0?' ':proc.trace[k]);
            }
            printf(" \n");
            final.pop();
        }
        /* for(int d=0;d<totalchar;d++) //dynamic dash line printing
        {
            printf("-");
        } */
        printf("------------------------------------------------");
        printf("\n\n");
        
    }else if(!strcmp(mode,"stats"))
    {
        printf("\nProcess    |");
        for(int j=0;j<numofProcesses;j++)
        {
            proc=final.front();
            printf("  %c  |",proc.name);
            final.pop();
            final.push(proc);
        }
        printf("\nArrival    |");
        for(int j=0;j<numofProcesses;j++)
        {
            proc=final.front();
            printattribute(proc.arrival_time);
            final.pop();
            final.push(proc);
        }
        printf("\nService    |");
        for(int j=0;j<numofProcesses;j++)
        {
            proc=final.front();
            printattribute(proc.service_time);
            final.pop();
            final.push(proc);
        }
        printf(" Mean|\n");
        printf("Finish     |");
        for(int j=0;j<numofProcesses;j++)
        {
            proc=final.front();
            printattribute(proc.finish_time);
            final.pop();
            final.push(proc);
        }
        printf("-----|\n");
        printf("Turnaround |");
        for(int j=0;j<numofProcesses;j++)
        {
            proc=final.front();
            printattribute(proc.turnaround_time);
            turnMean+=proc.turnaround_time;
            final.pop();
            final.push(proc);
        }
        printattributefloat((1.0*turnMean)/numofProcesses);
        printf("\n");
        printf("NormTurn   |");
        for(int j=0;j<numofProcesses;j++)
        {
            proc=final.front();
            printattributefloat(proc.normalized_turnaround);
            normMean+=proc.normalized_turnaround;
            final.pop();
            final.push(proc);
        }
        printattributefloat((1.0*normMean)/numofProcesses);
        printf("\n\n");
    } 
}

void read()
{
    int i=0,flag=1;
    char t;
    queue<Process> ready_queue;
    scanf("%s\n",mode); //trace or stats
    do{
        scanf("%c",&t);
        type.push_back(t-'0'); //policy number to be implemented
        scanf("%c",&t);
        if(t=='-')
        {
            scanf("%c",&t);
            quantum.push_back(t-'0'); //policy quantum if specified
            scanf("%c",&t);
        }else  
            quantum.push_back(0); //if quantum not needed,set to zero, so that each policy and its quantum have same index 
        numofPolicies++;
    }while(t != '\n');
    scanf("%d\n",&lastInstant);
    scanf("%d\n",&numofProcesses);
    while (i<numofProcesses)
    {
        scanf("%c,%d,%d\n",&Processes[i].name,&Processes[i].arrival_time,&Processes[i].service_time);
        Processes[i].initial_priority=Processes[i].service_time;
        Processes[i].current_priority=Processes[i].initial_priority;
        Processes[i].running_time=Processes[i].service_time;
        Processes[i].index=i; //keep track of inital position
        Processes[i].trace.resize(lastInstant,' ');
        ready_queue.push(Processes[i]);
        i++;
    }
    queue<Process> completion_queue;
    for(int c=0;c<numofPolicies;c++)
    {
        if(type[c] == 6)
        {
            quantum[c]=1;
        }else if(type[c] == 7)
        {
            quantum[c]=0; //quantum for each queue will be calculated later on for feedback-2i
        }
        switch (type[c])
        {
            case 1:
                completion_queue=FCFS(ready_queue);
                break;
            case 2:
                completion_queue=RoundRobin(ready_queue,quantum[c]);
                break;
            case 3:
                completion_queue=SPN(ready_queue);
                break;
            case 4:
                completion_queue=SRT(ready_queue);
                break;
            case 5:
                completion_queue=HRRN(ready_queue);
                break;
            case 6:
                completion_queue=Feedback(ready_queue,quantum[c]);
                break;   
            case 7:
                completion_queue=Feedback(ready_queue,quantum[c]);
                break; 
            case 8:
                completion_queue=Aging(ready_queue,quantum[c]);
                break;    
            default:
                flag=0;
                break;
        }
        if(flag)
            print(completion_queue,policies[type[c]],quantum[c],type[c]);
        else{
            printf("Policy Not Found!");
            flag=1;
        }
    }
}

int main(){
    read();
    return 0 ;
}
