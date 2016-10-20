#include <iostream>
#include <mpi/mpi.h>
#include <string.h>
#include <fstream>
#include <cmath>
#include <sys/types.h>
#include <unistd.h>
#include "core_algo.h"

using namespace std;

#define TILESIZE 90
#define MASTER 0
#define FINISHED_TAG 2
#define INDEL -1

#define MAXSIZE 22400


void master_send_work(int worker_id,int corner, int x_vals[],int y_vals[],int tile_x,int tile_y){

    printf("Master sending work to slave %d - tile(%d,%d) \n",worker_id,tile_x,tile_y);
    MPI_Send(&tile_x, 1, MPI_INT, worker_id, 0, MPI_COMM_WORLD);
    MPI_Send(&tile_y, 1, MPI_INT, worker_id, 0, MPI_COMM_WORLD);
    MPI_Send(&corner, 1, MPI_INT, worker_id, 0, MPI_COMM_WORLD);
    MPI_Send(x_vals + tile_x*TILESIZE, TILESIZE, MPI_INT, worker_id, 0, MPI_COMM_WORLD);
    MPI_Send(y_vals + tile_y*TILESIZE, TILESIZE, MPI_INT, worker_id, 0, MPI_COMM_WORLD);
}

int slave_recv_work(int *corner, int *up,int *left,int *tile_x,int *tile_y) {

    int rank;
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    printf("Slave %d waiting for work...\n",rank);
    MPI_Recv(tile_x, 1, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if (status.MPI_TAG == FINISHED_TAG ) {
        return -1;
    }
    MPI_Recv(tile_y, 1, MPI_INT,MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    MPI_Recv(corner, 1, MPI_INT,MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    MPI_Recv(left, TILESIZE, MPI_INT, MASTER,MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    MPI_Recv(up, TILESIZE, MPI_INT, MASTER,MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    printf("Slave %d receiving work : tile(%d,%d) \n", rank, *tile_x, *tile_y);
    return 0;
}

void slave_job_done(int corner,int low[],int right[],int tile_x,int tile_y){
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    printf("Slave %d sending ack for tile(%d,%d) \n",rank,tile_x,tile_y);
    MPI_Send(&tile_x, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
    MPI_Send(&tile_y, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
    MPI_Send(&corner, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
    MPI_Send(right, TILESIZE, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
    MPI_Send(low, TILESIZE, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
}

int master_recv_work(int corners[MAXSIZE/TILESIZE][MAXSIZE/TILESIZE],int x_vals[],int y_vals[]){
    MPI_Status status;
    int tile_x = 0,tile_y = 0;

    printf("Master waiting for ack \n");
    MPI_Recv(&tile_x, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    MPI_Recv(&tile_y, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    MPI_Recv(&corners[tile_x+1][tile_y+1], 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    MPI_Recv(x_vals + tile_x*TILESIZE, TILESIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    MPI_Recv(y_vals + tile_y*TILESIZE, TILESIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    printf("Master received ack for tile(%d,%d) - slave %d \n",tile_x,tile_y,status.MPI_SOURCE);

    return status.MPI_SOURCE;
}

int main() {
    int numtasks,rank;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0) {
        //Code for the master

        bool do_work = false;
        printf("Master : parsing data \n");
        // Get the data from stdin
        string xstr, ystr;
        cin >> xstr >> ystr;
        int x_size = (int) xstr.size();
        int y_size = (int) ystr.size();
        if (x_size > MAXSIZE || y_size > MAXSIZE) {
            for(int i = 1; i <= numtasks-1; i ++) {
                MPI_Send(&do_work, 1, MPI_C_BOOL, i, 0, MPI_COMM_WORLD);
            }
            printf("Error : the size of the sequences is too long.\n");
            printf("MAXSIZE : %d, first sequence size : %d, second sequence size : %d\n", MAXSIZE, x_size, y_size);
            MPI_Finalize();
            return 0;
        }
        char *x_chars = (char *) xstr.c_str();
        char *y_chars = (char *) ystr.c_str();

        printf("Master : init\n");
        int x_vals[MAXSIZE];
        int y_vals[MAXSIZE];
        int corners[MAXSIZE/TILESIZE][MAXSIZE/TILESIZE];
        for (int i = 0; i < MAXSIZE; i++) {
            x_vals[i] = (i + 1) * INDEL;
            y_vals[i] = x_vals[i];
        }

        int size_tile_y, size_tile_x;
        size_tile_x = TILESIZE;
        size_tile_y = TILESIZE;
        if (x_size < TILESIZE) size_tile_x = x_size;
        if (y_size < TILESIZE) size_tile_y = y_size;

        int nb_tiles_x = ((TILESIZE + x_size - 1) / TILESIZE);
        int nb_tiles_y = ((TILESIZE + y_size - 1) / TILESIZE);
        printf("nb_tiles_x : %d, size_tile_x : %d",nb_tiles_x, size_tile_x);
        printf("nb_tiles_y : %d, size_tile_y : %d",nb_tiles_y, size_tile_y);

        // The masters compute the first tile
        int corner = 0;
        nw_tile(x_vals, y_vals, x_chars, y_chars, size_tile_x, size_tile_y, &corner);
        // Handles the case where there is only one tile : no need to use slaves process
        if (nb_tiles_x + nb_tiles_y <= 2) {
            for(int i = 1; i <= numtasks-1; i ++) {
                MPI_Send(&do_work, 1, MPI_C_BOOL, i, 0, MPI_COMM_WORLD);
            }
            printf("Final score: %d\n", x_vals[x_size - 1]);
            MPI_Finalize();
            return 0;
        }
        corners[1][1] = corner;

        // Initialisation of the corner values
        for (int i = 1; i <= nb_tiles_x; i++){
            corners[i][0] = i * TILESIZE * INDEL;
        }
        for (int i = 1; i <= nb_tiles_y; i++){
            corners[0][i] = i * TILESIZE * INDEL;
        }

        //Initialisation : sends the char array to all the slaves
        do_work = true;
        for(int i = 1; i <= numtasks-1; i ++){
            printf("master sending init data...\n");
            MPI_Send(&do_work, 1, MPI_C_BOOL, i, 0, MPI_COMM_WORLD);
            MPI_Send(&x_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&y_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(x_chars, x_size, MPI_CHAR, i, 0, MPI_COMM_WORLD);
            MPI_Send(y_chars, y_size, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }
        printf("Master : working\n");
        int worker_id;
        int diag_size = 1;
        int first_i = 1,first_j = 0;
        int nb_jobs_done,next_job;
        for(int diagonal = 2; diagonal < nb_tiles_x + nb_tiles_y; diagonal++) {
            if(diagonal <= nb_tiles_x){
                if(diag_size < nb_tiles_x && diag_size < nb_tiles_y)
                    diag_size++;
                first_i++;
            }else{
                if(diagonal > nb_tiles_y)
                    diag_size--;
                first_j++;
            }
            printf("Master : new_diag - size : %d\n",diag_size);
            nb_jobs_done = 0;
            next_job = min(numtasks,diag_size + 1);
            // sends jobs for the diagonal
            for(int k = 0; k < (numtasks - 1) && k < diag_size; k++){
                master_send_work(k + 1,
                                 corners[first_i - k - 1][first_j + k],
                                 x_vals,
                                 y_vals,
                                 first_i - k - 1,
                                 first_j + k);
            }
            while(nb_jobs_done < diag_size){
                worker_id = master_recv_work(corners, x_vals, y_vals);
                nb_jobs_done++;
                if(next_job <= diag_size){
                    master_send_work(worker_id,
                                     corners[first_i - next_job][first_j + next_job - 1],
                                     x_vals,
                                     y_vals,
                                     first_i - next_job,
                                     first_j + next_job - 1);
                    next_job++;
                }
            }
        }

        printf("Master : sending all the threads to sleep\n");
        // Terminates the slaves work by sending them a request to sleep
        for(int i = 1; i <= numtasks-1; i ++){
            printf("Master : thread %d -> GO TO SLEEP DUDE!\n",i);
            MPI_Send(&nb_tiles_x, 1, MPI_INT, i, FINISHED_TAG, MPI_COMM_WORLD);
        }

        printf("Final score: %d\n", x_vals[x_size - 1]);
    }else{
        //Code for the slaves
        MPI_Status status;
        bool do_work = false;
        MPI_Recv(&do_work, 1, MPI_C_BOOL, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(!do_work){
            MPI_Finalize();
            return 0;
        }

        // Initialisation : all the slaves thread receive the char array containing the DNA sequence
        printf("Slave %d : init\n",rank);
        int x_size,y_size;
        MPI_Recv(&x_size, 1, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&y_size, 1, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        char x_chars[x_size];
        char y_chars[y_size];
        MPI_Recv(x_chars, x_size, MPI_CHAR, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(y_chars, y_size, MPI_CHAR, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf("Slave received init data... \n");

        // Setting the number of tiles
        int nb_tiles_x = ((TILESIZE + x_size - 1) / TILESIZE);
        int nb_tiles_y = ((TILESIZE + y_size - 1) / TILESIZE);

        // Setting the tile size
        int size_tile_y, size_tile_x;
        size_tile_x = TILESIZE;
        size_tile_y = TILESIZE;

        // Taking into account the edges of the array
        int edge_x = x_size % TILESIZE;
        int edge_y = y_size % TILESIZE;
        if (!edge_x) edge_x = TILESIZE;
        if (!edge_y) edge_y = TILESIZE;

        int left[TILESIZE];
        int up[TILESIZE];
        char* char_left;
        char* char_up;
        int tile_x,tile_y;
        int corner;
        while( slave_recv_work(&corner, up, left, &tile_x, &tile_y) >= 0 ){
            char_up = &y_chars[tile_y*TILESIZE];
            char_left = &x_chars[tile_x*TILESIZE];
            printf("Slave %d : calculating solution for :\n",rank);
            nw_tile(left,
                    up,
                    char_left,
                    char_up,
                    (tile_x == nb_tiles_x - 1)? edge_x : size_tile_x,
                    (tile_y == nb_tiles_y - 1)? edge_y : size_tile_y,
                    &corner);
            slave_job_done(corner, up, left, tile_x, tile_y);
        }
        printf("Slave %d : go to sleep \n",rank);
    }
    MPI_Finalize();

    return 0;
}

