#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpix.h>
#include "definitions.h"
#include "comm.h"
#include "pairing.h"

void make_MPI_ints(MPI_Datatype *type)
{
  MPI_Type_contiguous(MANY_INTS_ELEMS, MPI_INT, type);
  MPI_Type_commit(type);
}

void init_network(int* network)
{
  MPIX_Hardware_t meta;
  MPIX_Hardware(&meta);
  for (int i = 0; i < MPIX_TORUS_MAX_DIMS; i++) network[i] = meta.Size[i];
}

/*
 * This code performs a ping-pong bisection pairing benchmark experiment using the Diagonals pairing scheme.
 * In diagonals pairing, a node u with coordinates (u_1, ..., u_5, t) is paired to (l_1 - u_1, ..., l_5 - u_5, t)
 * where l_i is the dimension length of the dimension i.
 * This traffic pattern is expected to be demanding on torus networks because much of the traffic must be routed
 * through the middle of the network.
 */
int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);
  int torus[TORUS_DIMENSIONS];
  int coordinates[TORUS_DIMENSIONS + 1], counterpart_coords[TORUS_DIMENSIONS + 1];
  int rank, counterpart, size;
  int local_errors = 0;
  int *buf[BUFFER_COUNT], *buf2[BUFFER_COUNT];
  int k;
  double total_test_time, total_warmup_time, pure_time;
  double all_times[WARMUP_ITERATIONS + ITERATIONS];
  double warmup_time[WARMUP_ITERATIONS], test_time[ITERATIONS];
  for (int i = 0; i < BUFFER_COUNT; i++)
  {
    buf[i] = (int*) malloc(MESSAGE_ELEMENTS * MANY_INTS_ELEMS * sizeof(int));
    buf2[i] = (int*) malloc(MESSAGE_ELEMENTS * MANY_INTS_ELEMS * sizeof(int));
    k = 0;
    while(buf[i] == NULL) 
    {
      buf[i] = (int*) malloc(MESSAGE_ELEMENTS * MANY_INTS_ELEMS * sizeof(int));
      k++;
      if (k == MAX_ALLOC_ATTEMPTS && buf[i] == NULL) 
      {
        local_errors++;
        break;
      }
    }
    k = 0;
    while(buf2[i] == NULL) 
    {
      buf2[i] = (int*) malloc(MESSAGE_ELEMENTS * MANY_INTS_ELEMS * sizeof(int));
      k++;
      if (k == MAX_ALLOC_ATTEMPTS && buf2[i] == NULL) 
      {
        local_errors++;
        break;
      }
    }
  }

  MPI_Datatype many_ints;
  int errors_count; 

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Allreduce(&local_errors, &errors_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  if (errors_count > 0)
  {
    LEADER printf("At least one process failed to allocate memory, execution will cancel\n");
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 3;
  }

#if defined DIAGONAL && defined SEQUENTIAL
  LEADER printf("\nBeginning experiment SEQUENTIAL-DIAGONAL\n\n");
#elif defined FURTHEST && defined SEQUENTIAL
  LEADER printf("\nBeginning experiment SEQUENTIAL-FURTHEST\n\n");
#elif defined DIAGONAL && defined FLOOD
  LEADER printf("\nBeginning experiment FLOOD-DIAGONAL\n\n");
#elif defined FURTHEST && defined FLOOD
  LEADER printf("\nBeginning experiment FLOOD-FURTHEST\n\n");
#endif
  LEADER printf("All parameters: nodes: %d, many_ints_size: %d, elements: %d, total megabytes per message: %d, warmup rounds: %d, real rounds: %d.\n\n",
    size, MANY_INTS_ELEMS, MESSAGE_ELEMENTS, MANY_INTS_ELEMS * MESSAGE_ELEMENTS * sizeof(int) / (1024 * 1024), WARMUP_ITERATIONS, ITERATIONS);
  // MPI Object initializations
  MPIX_Rank2torus(rank, coordinates);
  make_MPI_ints(&many_ints);
  init_network(torus);

  for (int i = 0; i < BUFFER_COUNT; i++)
  {
    for (long j = 0; j < MESSAGE_ELEMENTS * MANY_INTS_ELEMS; j++) buf[i][j] = rank*i + i;  // Dummy values to communicate
  }

  // Find coordinates of counterparts based on partitioning schemes
  get_counterpart(torus, coordinates, counterpart_coords);
  MPIX_Torus2rank(counterpart_coords, &counterpart);
  MPI_Barrier(MPI_COMM_WORLD);
  total_test_time = communicate(counterpart, &many_ints, buf, buf2, WARMUP_ITERATIONS + ITERATIONS, all_times); // Experiment
//  LEADER printf("\nBarrier: communication.\n");
  MPI_Barrier(MPI_COMM_WORLD);
#ifdef VERBOSE
  total_warmup_time = 0;
  pure_time = 0;  
  for (int i = 0; i < WARMUP_ITERATIONS; i++)
  {
     warmup_time[i] = all_times[i];
     total_warmup_time += warmup_time[i];
  }
  for (int i = 0; i < ITERATIONS; i++) 
  {
    test_time[i] = all_times[WARMUP_ITERATIONS + i];
    pure_time += test_time[i];
  }
  printf("Rank %d (%d,%d,%d,%d,%d), counterpart %d (%d,%d,%d,%d,%d): time %f (warmup %f, pure %f)\n", 
    rank, coordinates[0], coordinates[1], coordinates[2], coordinates[3], coordinates[4],
    counterpart, counterpart_coords[0], counterpart_coords[1], counterpart_coords[2], counterpart_coords[3],
    counterpart_coords[4], total_test_time, total_warmup_time, pure_time);
  fflush(stdout);
  MPI_Barrier(MPI_COMM_WORLD);
#endif

#if defined DIAGONAL && defined SEQUENTIAL
  LEADER printf("\nCompleted experiment SEQUENTIAL-DIAGONAL\n");
#elif defined FURTHEST && defined SEQUENTIAL
  LEADER printf("\nCompleted experiment SEQUENTIAL-FURTHEST\n");
#elif defined DIAGONAL && defined FLOOD
  LEADER printf("\nCompleted experiment FLOOD-DIAGONAL\n");
#elif defined FURTHEST && defined FLOOD
  LEADER printf("\nCompleted experiment FLOOD-FURTHEST\n");
#endif
//  LEADER printf("Total elapsed time (leader node only): %f seconds for test (%d iterations), %f seconds for warmup (%d iterations).\n\n\n",
//    total_test_time, ITERATIONS, total_warmup_time, WARMUP_ITERATIONS);
  // Cleanup
  for (int i = 0; i < BUFFER_COUNT; i++)
  {
    free(buf[i]);
    free(buf2[i]);
  }
  fflush(stdout);
  MPI_Barrier(MPI_COMM_WORLD);
#if defined VERBOSE && defined SEQUENTIAL
  for (int i = 0; i < WARMUP_ITERATIONS; i++)
  {
    printf("Warmup: rank %d round %d: time %f\n", rank, i, warmup_time[i]);
  }
  fflush(stdout);
  MPI_Barrier(MPI_COMM_WORLD);
  for (int i = 0; i < ITERATIONS; i++)
  {
    printf("Communication: rank %d round %d: time %f\n", rank, i, test_time[i]);
  }
  fflush(stdout);
  MPI_Barrier(MPI_COMM_WORLD);
#endif
  MPI_Type_free(&many_ints);
  MPI_Finalize();
  return 0;
}

