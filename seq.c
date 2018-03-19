#include <mpi.h>
#include "definitions.h"
#include "comm.h"

void randomize(int* buf)
{
  for (int i = 0; i < MESSAGE_ELEMENTS; i++)
  {
    buf[i] = ((int) rand());
  }
}


double communicate(int target, MPI_Datatype *many_ints, int** send_buffers, int** recv_buffers, int rounds, double* results)
{
  double start, end, round_start, round_end;
  MPI_Request reqs[2][BUFFER_COUNT];
  start = MPI_Wtime();
  for (int i = 0; i < rounds; i++)
  {
    round_start = MPI_Wtime();
    for (int j = 0; j < BUFFER_COUNT; j++)
    {
      randomize(send_buffers[j]);
      MPI_Isend(send_buffers[j], MESSAGE_ELEMENTS, *many_ints, target, 0, MPI_COMM_WORLD, &(reqs[0][j]));
      MPI_Irecv(recv_buffers[j], MESSAGE_ELEMENTS, *many_ints, target, 0, MPI_COMM_WORLD, &(reqs[1][j]));
    }
    MPI_Waitall(BUFFER_COUNT * 2, &reqs[0][0], MPI_STATUSES_IGNORE);
    round_end = MPI_Wtime();
    results[i] = round_end - round_start;
  }
  end = MPI_Wtime();
  return end - start;
}
