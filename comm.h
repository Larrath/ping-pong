#ifndef COMM_H
#define COMM_H
double communicate(int target, MPI_Datatype *many_ints, int** send_buffers, int** recv_buffers, int rounds, double* results);
#endif
