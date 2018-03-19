#include "definitions.h"
#include "pairing.h"

void get_counterpart(int* network, int* source, int* dest)
{
  dest[A_DIM] = (source[A_DIM] + network[A_DIM] / 2) % network[A_DIM];
  dest[B_DIM] = (source[B_DIM] + network[B_DIM] / 2) % network[B_DIM];
  dest[C_DIM] = (source[C_DIM] + network[C_DIM] / 2) % network[C_DIM];
  dest[D_DIM] = (source[D_DIM] + network[D_DIM] / 2) % network[D_DIM];
  dest[E_DIM] = (source[E_DIM] + network[E_DIM] / 2) % network[E_DIM];
  dest[T_DIM] = source[T_DIM]; // We ignore T_DIM, as it relates to core placement which we ignore.
}

