#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#define size_matrix 100
#define num_thread_join 2
#define num_thread_dtach 2
#define num_proces 2

int ID [6] = {1,2,1,1,1,6,2}; // id : 1211162 in A
int bir_id [9] = {2,4,2,5,9,5,7,4,8,6}; // id*2003 =2425957486 in B
int A [size_matrix] [size_matrix]; // matrix a 
int B [size_matrix] [size_matrix]; // matrix b 
int C_th_joinable [size_matrix] [size_matrix]; // matrix c is result a*b by trhead joinable
int C_proc [size_matrix] [size_matrix]; // matrix c is result a*b by proces
int C_th_dtached[size_matrix] [size_matrix]; // matrix c is result a*b by trhead dtached
int C_native [size_matrix] [size_matrix] ; // matrix c is result a*b by native appoach

// multiplication by thread dtached
void* mult_th_dtached(void *args){ 
   int thread_id = *((int *)args);
   int size = size_matrix / num_thread_dtach ; // how much row for each thread
   int start=thread_id*size; // assume we have 3 thread so size = 100/3=99 , so start index 0 and end 33
   int end;
   if(thread_id==num_thread_dtach-1) end=size_matrix;
   else end=start+size;  // if we arrive last thread end equle 100

   // insted loops for mult matrix A*B =C with spesfic index for thread
   for(int i=start ; i<end ; i++){
    for(int j=0 ; j<size_matrix ; j++){
        C_th_dtached[i][j] =0;
        for(int f =0 ; f<size_matrix ; f++){
            C_th_dtached[i][j]+= A[i][f]*B[f][j];
        }
    }
   }
}
// multiplication by thread joinable
void* mult_th_joinable(void *args){ 
   int thread_id = *((int *)args);
   int size = size_matrix / num_thread_join ; // how much row for each thread
   int start=thread_id*size; // assume we have 3 thread so size = 100/3=99 , so start index 0 and end 33
   int end;
   if(thread_id==num_thread_join-1) end=size_matrix;
   else end=start+size;  // if we arrive last thread end equle 100

   // insted loops for mult matrix A*B =C with spesfic index for thread
   for(int i=start ; i<end ; i++){
    for(int j=0 ; j<size_matrix ; j++){
        C_th_joinable[i][j] =0;
        for(int f =0 ; f<size_matrix ; f++){
            C_th_joinable[i][j]+= A[i][f]*B[f][j];
        }
    }
   }
}
// multiplication by proces
void mult_proces (int start , int end , int res [size_matrix][size_matrix]){ 
    // insted loops for mult matrix A*B =C with spesfic index for procese
    for(int i=start ; i<end ; i++){
     for(int j=0 ; j<size_matrix ; j++){
        res [i][j] =0 ;
        for(int f =0 ; f<size_matrix ; f++){
            res [i][j]+=  A[i][f]*B[f][j];
        }
    }
   }
}
// multilplication matrix by native appoach
void mult_matrix (){ 
    // insted loops for mult matrix A*B =C with spesfic index for procese
    for(int i=0 ; i<size_matrix ; i++){
     for(int j=0 ; j<size_matrix ; j++){
        C_native [i][j] =0 ;
        for(int f =0 ; f<size_matrix ; f++){
            C_native [i][j]+=  A[i][f]*B[f][j];
        }
    }
   }
}
// main function 
int main (){
    int mess [num_proces] [2]; // for pipes 
    pthread_t th [num_thread_join]; // identifier threads
    pthread_t th_dt [num_thread_dtach]; // identifier threads
    int id_th_join[num_thread_dtach]; // id for each thread  join able to make it stand out 
    int id_th_dtached[num_thread_dtach];// id for each thread dtached to make it stand out 
    int result [size_matrix][size_matrix];// local array to save first result from proces
    pid_t id[num_proces]; int i ;
    pthread_attr_t dtachedThread ;
    pthread_attr_init (&dtachedThread);
    pthread_attr_setdetachstate(&dtachedThread,PTHREAD_CREATE_DETACHED);
    clock_t start , end ;
    double time ;
    
    // to  fill A , B matrix 
    for(int j =0 ; j<size_matrix  ; j++){ 
        for(int k =0 ; k<size_matrix  ; k++){
            A[j][k]=ID[rand()%7]; // fill matrix  A from array my university number 
            B[j][k]=bir_id[rand()%10]; // fill matrix  B from array my university number * birthdate 
        }
    } 

    // multilplication matrix by native appoach
    start = clock();
    mult_matrix(); // call function mult matrix without any proces or thread
    end = clock();
    time=((double) end - start);
    printf("exiction time without proces or thread : %f \n\n",time);


    // implemantion for pipe && handling
    for(int i =0 ;i<num_proces;i++){
        if(pipe(mess[i])==-1){ 
        printf("error in pipe\n");
        return 1;
        }
    }
    // createion child (proceses)
    start = clock();
    for(int i = 0 ; i < num_proces ; i++){
        id[i] = fork();
        if (id[i]== -1){
            // error handling 
            printf (" error in create child");
            return 1 ;
        }
        else if(id[i] ==0){//child proces
        int start_r = i*(size_matrix/num_proces);
        int end_r = (i==num_proces-1) ? size_matrix : (i+1) * (size_matrix/num_proces);
        close(mess[i][0]);
        mult_proces(start_r,end_r,result); // call function multiplication proces
        // send result by pipe
        if(write(mess[i][1],result+start_r,sizeof(int)*size_matrix*(end_r-start_r)) == -1){
            // error handling
            printf("error in write of pipe \n");
            return 4;
        } 
        close(mess[i][1]);
        exit(0);
        }
    } 
    // read from pipe and save it in C_proc
    for(int i =0 ;i<num_proces;i++){
        close(mess[i][1]);
        int start_row = i*(size_matrix/num_proces);
        int end_row = (i==num_proces-1) ? size_matrix : (i+1) * (size_matrix/num_proces); 
        if(read(mess[i][0],C_proc+start_row,sizeof(int)*size_matrix*(end_row -start_row))==-1){
            // error handling
            printf("error in read from pipe \n");
            return 5 ;
        }
        close(mess[i][0]);
    }
    //wait all proces end 
    end= clock();
    time=((double) end - start);
    printf("exiction time for %d proces : %f \n\n",time,num_proces);
    while (wait(NULL)!= -1 || errno != ECHILD);


       
    
    // create multi threads  (joinable)
    start = clock();
    for(i=0; i<num_thread_join ;i++){
        id_th_join[i]=i;
        if(pthread_create(th+i,NULL,&mult_th_joinable,(void*)&id_th_join[i])!=0){
            // error handling thread 
            return 2;
        } 
    }
    // wait for all tread joinable
    for(i=0 ;i<num_thread_join ;i++){
          if(pthread_join(th[i],NULL)!= 0){
            // error handling thread 
            return 3 ;
        } 
    } 
    end = clock();
    time=((double) end - start);
    printf("exiction time for %d thread joinable : %f \n\n",time,num_thread_join);


  // create multi threads  (dtached)
  start = clock();
    for(i=0; i<num_thread_dtach ;i++){
        id_th_dtached[i]=i;
        if(pthread_create(&th_dt[i],&dtachedThread,&mult_th_dtached,(void*)&id_th_dtached[i])!=0){
            // error handling thread 
            return 6;
        } 
    }
    pthread_attr_destroy(&dtachedThread);
    end = clock();
    time=((double) end - start);
    printf("exiction time for %d thread dtached : %f \n\n\n",time,num_thread_dtach);
    pthread_exit(0);
    return 0;
}