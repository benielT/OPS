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
  * @brief core source file for OPS HLS MPI backend
  * @author Beniel Thileepan
  * @details Source file for OPS HLS MPI backend
*/

#include "../include/ops_hls_mpi.hpp"
#include "../include/ops_hls_fpga.hpp"

MPI_Comm OPS_MPI_GLOBAL;
int ops_comm_global_size;
int ops_my_global_rank;
MPI_Comm OPS_MPI_LOCAL;
int ops_my_local_rank;
int ops_local_size;

void ops_init_hls(int argc, char** argv)
{
    int flag = 0;
    void *v;

    MPI_Initialized(&flag);
    if (!flag)
    {
        MPI_Init(&argc, &argv);
    }

    // Splitting the communication world for MPMD app. This is not necessary for now.
    MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_APPNUM, &v, &flag);

    if (!flag){
        MPI_Comm_dup(MPI_COMM_WORLD, &OPS_MPI_GLOBAL);
    } else {
        int appnum = *(int*)v;
        int rank;

        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_split(MPI_COMM_WORLD, appnum, rank, &OPS_MPI_GLOBAL);
    }

    MPI_Comm_size(OPS_MPI_GLOBAL, &ops_comm_global_size);
    MPI_Comm_rank(OPS_MPI_GLOBAL, &ops_my_global_rank);

    MPI_Comm_split_type(OPS_MPI_GLOBAL, MPI_COMM_TYPE_SHARED,
            ops_my_global_rank, MPI_INFO_NULL, &OPS_MPI_LOCAL);
    MPI_Comm_rank(OPS_MPI_LOCAL, &ops_my_local_rank);
    MPI_Comm_size(OPS_MPI_LOCAL, &ops_local_size);

#ifdef DEBUG_LOG
    printf("[DEBUG][MPI] rank=%d size=%d\n", ops_my_global_rank, ops_comm_global_size);
#endif

    ops::hls::FPGA::getInstance()->setMPIContext(ops_my_global_rank, ops_comm_global_size, 
        ops_my_local_rank, ops_local_size);
    ops_init_backend(argc, argv);

    auto* fpga = ops::hls::FPGA::getInstance();
    fpga->initAurora(ops_my_global_rank, ops_comm_global_size);

        int local_ok = fpga->auroraLinkCheck(3000) ? 1 : 0;
    int all_ok = 0;

    MPI_Allreduce(&local_ok, &all_ok, 1, MPI_INT, MPI_MIN, OPS_MPI_GLOBAL); //using allreduce insted of barrier

    if (!all_ok) {
        if (ops_my_global_rank == MPI_ROOT)
            std::cerr << "[ERROR][AURORA] link bring-up failed, aborting\n";
        MPI_Abort(OPS_MPI_GLOBAL, 1);
    }

    fpga->auroraResetCounters();
    MPI_Barrier(OPS_MPI_GLOBAL);
}

void ops_exit_hls() {
    MPI_Comm_free(&OPS_MPI_LOCAL);
    MPI_Comm_free(&OPS_MPI_GLOBAL);
    ops_exit_backend();
}