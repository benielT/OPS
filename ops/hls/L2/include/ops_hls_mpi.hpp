#ifndef __OPS_HLS_MPI_H
#define __OPS_HLS_MPI_H

/* 
* Open source copyright declaration based on BSD open source template:
* http://www.opensource.org/licenses/bsd-license.php
*
* This file is part of the OPS distribution.
*
* Copyright (c) 2013, Mike Giles and others. Please see the AUTHORS file in
* the main source directory for a full list of copyright holders.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
*/

/** @file
  * @brief core header file for OPS HLS MPI backend
  * @author Beniel Thileepan
  * @details Header file for OPS HLS MPI backend. This can be moved to ops_mpi_core.h when ops hls
  *     target fully integrated with OPS backend memory and transaction layers
  */
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <mpi.h>

/** Define the root MPI process **/
#ifdef MPI_ROOT
#undef MPI_ROOT
#endif
#define MPI_ROOT 0

//
// MPI Communicator for halo creation and exchange
//

extern MPI_Comm OPS_MPI_GLOBAL;
extern int ops_comm_global_size;
extern int ops_my_global_rank;
extern MPI_Comm OPS_MPI_LOCAL;
extern int ops_my_local_rank;
extern int ops_local_size;

void ops_init_hls(int argc, char** argv);
void ops_exit_hls();

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
#endif /* __OPS_HLS_MPI_H */