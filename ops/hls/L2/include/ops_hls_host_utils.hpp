#pragma once

/*
 * Copyright 2019 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/** @file
  * @brief Host side utils
  * @author Beniel Thileepan (maintainer)
  * @details This is to manage support functions in the host side.
  */

#include <iomanip>
#include <iostream>
#include "ops_hls_kernel.hpp"

static ops::hls::SizeType default_d_p({0,0,0});
static ops::hls::SizeType default_d_m({0,0,0});

void printIdx(ops::hls::IdxType& idx, std::string prompt= "")
{
	std::cout << "-------------------------------" << std::endl;
	std::cout << "  index: " << prompt << ": (" << idx[0] << "," << idx[1] << "," << idx[2] << ")" << std::endl;
	std::cout << "-------------------------------" << std::endl;
}

void printGridProp(ops::hls::GridPropertyCore& gridProp, std::string prompt = "")
{
	std::cout << "-------------------------------" << std::endl;
	std::cout << "  grid properties: " << prompt << std::endl;
	std::cout << "-------------------------------" << std::endl;
	std::cout << std::setw(21) << std::right << "dim: "  << gridProp.dim << std::endl;
    std::cout << std::setw(21) << std::right << "batch_size: "  << gridProp.batch_size << std::endl;
	std::cout << std::setw(21) << std::right << "d_m: " << "(" << gridProp.d_m[0]
				<< ", " << gridProp.d_m[1] << ", " << gridProp.d_m[2] <<")"<< std::endl;
	std::cout << std::setw(21) << std::right << "d_p: " << "(" << gridProp.d_p[0]
				<< ", " << gridProp.d_p[1] << ", " << gridProp.d_p[2] <<")"<< std::endl;
	std::cout << std::setw(21) << std::right << "logical size: " << "(" << gridProp.size[0]
				<< ", " << gridProp.size[1] << ", " << gridProp.size[2] <<")"<< std::endl;
	std::cout << std::setw(21) << std::right << "actual size: " << "(" << gridProp.actual_size[0]
				<< ", " << gridProp.actual_size[1] << ", " << gridProp.actual_size[2] <<")"<< std::endl;
	std::cout << std::setw(21) << std::right << "grid size: " << "(" << gridProp.grid_size[0]
				<< ", " << gridProp.grid_size[1] << ", " << gridProp.grid_size[2] <<")"<< std::endl;
    ops::hls::SizeType ralizedGridSize = {gridProp.grid_size[0], gridProp.grid_size[1], gridProp.grid_size[2]};
    ralizedGridSize[gridProp.dim - 1] *= gridProp.batch_size;
    std::cout << std::setw(21) << std::right << "realized grid size: " << "(" << ralizedGridSize[0]
				<< ", " << ralizedGridSize[1] << ", " << ralizedGridSize[2] <<")"<< std::endl;
	std::cout << std::setw(21) << std::right << "xblocks: " << gridProp.xblocks << std::endl;
	std::cout << std::setw(21) << std::right << "total iterations: " << gridProp.total_itr << std::endl;
	std::cout << std::setw(21) << std::right << "outer limit: " << gridProp.outer_loop_limit << std::endl;
	std::cout << "-------------------------------" << std::endl;
}

void printGridProp(ops::hls::GridPropertyCoreV2& gridProp, std::string prompt = "")
{
	std::cout << "-------------------------------" << std::endl;
	std::cout << "  grid properties (V2): " << prompt << std::endl;
	std::cout << "-------------------------------" << std::endl;
	std::cout << std::setw(21) << std::right << "dim: "  << gridProp.dim << std::endl;
    std::cout << std::setw(21) << std::right << "batch_size: "  << gridProp.batch_size << std::endl;
	std::cout << std::setw(21) << std::right << "d_m: " << "(" << gridProp.d_m[0]
				<< ", " << gridProp.d_m[1] << ", " << gridProp.d_m[2] <<")"<< std::endl;
	std::cout << std::setw(21) << std::right << "d_p: " << "(" << gridProp.d_p[0]
				<< ", " << gridProp.d_p[1] << ", " << gridProp.d_p[2] <<")"<< std::endl;
	std::cout << std::setw(21) << std::right << "logical size: " << "(" << gridProp.size[0]
				<< ", " << gridProp.size[1] << ", " << gridProp.size[2] <<")"<< std::endl;
	std::cout << std::setw(21) << std::right << "actual size: " << "(" << gridProp.actual_size[0]
				<< ", " << gridProp.actual_size[1] << ", " << gridProp.actual_size[2] <<")"<< std::endl;
	std::cout << std::setw(21) << std::right << "grid size: " << "(" << gridProp.grid_size[0]
				<< ", " << gridProp.grid_size[1] << ", " << gridProp.grid_size[2] <<")"<< std::endl;
    ops::hls::SizeType ralizedGridSize = {gridProp.grid_size[0], gridProp.grid_size[1], gridProp.grid_size[2]};
    ralizedGridSize[gridProp.dim - 1] *= gridProp.batch_size;
    std::cout << std::setw(21) << std::right << "realized grid size: " << "(" << ralizedGridSize[0]
				<< ", " << ralizedGridSize[1] << ", " << ralizedGridSize[2] <<")"<< std::endl;
	std::cout << "-------------------------------" << std::endl;
}

void printStencilConfig(ops::hls::StencilConfigCore& stencilConfig, std::string prompt = "")
{
	std::cout << "-------------------------------" << std::endl;
	std::cout << "  stencil configuration (V2): " << prompt << std::endl;
	std::cout << "-------------------------------" << std::endl;
	std::cout << std::setw(15) << std::right << "grid_size (xblocks, y, z): " << "(" << stencilConfig.grid_size[0]
				<< ", " << stencilConfig.grid_size[1] << ", " << stencilConfig.grid_size[2] <<")"<< std::endl;
	std::cout << std::setw(15) << std::right << "lower_limit: " << "(" << stencilConfig.lower_limit[0]
				<< ", " << stencilConfig.lower_limit[1] << ", " << stencilConfig.lower_limit[2] <<")"<< std::endl;
	std::cout << std::setw(15) << std::right << "upper_limit: " << "(" << stencilConfig.upper_limit[0]
				<< ", " << stencilConfig.upper_limit[1] << ", " << stencilConfig.upper_limit[2] <<")"<< std::endl;
	std::cout << std::setw(15) << std::right << "dim: " << stencilConfig.dim << std::endl;
	std::cout << std::setw(15) << std::right << "outer_loop_limit: " << stencilConfig.outer_loop_limit << std::endl;
	std::cout << std::setw(15) << std::right << "total_itr: " << stencilConfig.total_itr << std::endl;
	std::cout << "-------------------------------" << std::endl;
}

void printAccessRange(ops::hls::AccessRange& range, std::string prompt = "")
{
	std::cout << "-------------------------------" << std::endl;
	std::cout << " access range " << prompt << " - dim: " << range.dim << ", range: (" << range.start[0] << ", " << range.start[1] << ", "
			<< range.start[2] << ") --> (" << range.end[0] << ", " << range.end[1] << ", "<< range.end[2] << ")" << std::endl;
	std::cout << "-------------------------------" << std::endl;

}

template<unsigned short N_SLR, unsigned short P_SLR, unsigned short HALF_SPAN, unsigned short MEM_VECTOR_SIZE>
const unsigned short get_overlap_size() {
    auto val =  (((N_SLR * P_SLR) * HALF_SPAN  + MEM_VECTOR_SIZE - 1) / MEM_VECTOR_SIZE) * MEM_VECTOR_SIZE * 2;
	return val;
}

template<unsigned short TOTAL_SLR, unsigned short HALF_SPAN, unsigned short MEM_VECTOR_SIZE>
const unsigned short get_overlap_size() {
    auto val =  (((TOTAL_SLR) * HALF_SPAN  + MEM_VECTOR_SIZE - 1) / MEM_VECTOR_SIZE) * MEM_VECTOR_SIZE * 2;
	return val;
}

#if defined(OPS_HLS_TILE_INTERLEAVE)

void genTileMetadataCPU(
            const unsigned short data_vector_factor,
            const unsigned short tile_dim,
            ops::hls::SizeType& grid_size, 
            ops::hls::AccessRange& range, 
            ops::hls::SizeType2d& tile_size,
            ops::hls::SizeType2d& overlap_size,
            ops::hls::SizeType2d& effective_tile_size,
            ops::hls::SizeType2d& last_tile_size,
            ops::hls::SizeType2d& tile_count,
            unsigned short& last_tile_upper_limit_x,
            unsigned int& total_xblocks,
            bool isWideMem = true) 
    {
        const unsigned short ShiftBits = (unsigned short)LOG2_NON_CONSTEXPR(data_vector_factor);
        const unsigned short start_x = range.start[0] >> ShiftBits;
        const unsigned short end_x = (range.end[0] + data_vector_factor - 1) >> ShiftBits;
        const unsigned short grid_xblocks = grid_size[0] >> ShiftBits;
        const unsigned short num_xblocks = end_x - start_x;

        const unsigned short tile_size_x_beats = tile_size[0] >> ShiftBits;
        const unsigned short overlap_size_x_beats = overlap_size[0] >> ShiftBits;

        const unsigned short effective_tile_size_x_beats = tile_size_x_beats - overlap_size_x_beats;

        const unsigned short effective_tile_size_y = tile_dim == 2 ? tile_size[1] - overlap_size[1] : grid_size[1];
        const unsigned short diff_y = range.end[1] - range.start[1];
        const unsigned short realized_tile_size_x_beats = tile_size_x_beats > num_xblocks ? num_xblocks : tile_size_x_beats;
        const unsigned short tile_count_x = ((num_xblocks - realized_tile_size_x_beats) + effective_tile_size_x_beats - 1) / effective_tile_size_x_beats + 1;
        const unsigned short last_tile_size_x_beats = tile_count_x > 1 ? num_xblocks - (tile_count_x - 1) * effective_tile_size_x_beats : realized_tile_size_x_beats;
        last_tile_upper_limit_x = range.end[0] - (((tile_count_x - 1) * effective_tile_size_x_beats) << ShiftBits);

        const unsigned short realized_tile_size_y = tile_dim == 2 ? tile_size[1] > diff_y ? diff_y : tile_size[1] : grid_size[1];
        const unsigned short tile_count_y = ((diff_y - realized_tile_size_y) + effective_tile_size_y - 1) / effective_tile_size_y + 1;
        const unsigned short last_tile_size_y = tile_count_y > 1 ? diff_y - (tile_count_y - 1) * effective_tile_size_y : realized_tile_size_y;
        // const unsigned short last_tile_upper_limit_y = TILE_DIM == 2 ? range.end[1] - (tile_count_y - 1) * effective_tile_size_y : range.end[1];

// #ifndef __SYNTHESIS__
//     #if defined(OPS_FPGA) && defined(OPS_TILING)
//         if (tile_size[0] != POW2(LOG2_NON_CONSTEXPR(tile_size[0]))) {
//             OPSException ex(OPS_RUNTIME_ERROR);
//             ex << "ERROR: x tile_size (" << tile_size[0] << ") has to be power of 2" 
//                     << "Please make sure appropriate OPS_TILESIZE_X runtime flag is properly set";
//             throw ex;
//         }

//         if (tile_size[0] <= overlap_size[0]) {
//             OPSException ex(OPS_RUNTIME_ERROR);
//             ex << "ERROR: x tile_size (" << tile_size[0] << ") is less than the minimum required overlap size (" << overlap_size[0] << ") in x direction. " 
//                     << "Please make sure appropriate OPS_TILESIZE_X runtime flag is properly set";
//             throw ex;
//         }

//         if (tile_size[0] > OPS_MAXTILESIZE_X) {
//             OPSException ex(OPS_RUNTIME_ERROR);
//             ex << "ERROR: x tile_size (" << tile_size[0] << ") is greater than the minimum tile supported by the generated hardware (" << OPS_MAXTILESIZE_X << ") in x direction. " 
//                     << "Please make sure appropriate OPS_TILESIZE_X runtime flag is properly set. If bigger tile size need, rebuild with bigger OPS_MAXTILESIZE_X";
//             throw ex;
//         }

//         if (tile_size_x_beats > realized_tile_size_x_beats) {
//             std::cout << "[OPS_WARNING]: Grid is smaller than tile_size in x direction. Running without tiling in x direction" << std::endl;
//         }
//         if (float(effective_tile_size_x_beats) / float(tile_size_x_beats) < 0.75) {
//             std::cout << "[OPS_WARNING]: Effective tile size is: " << float(effective_tile_size_x_beats) / float(tile_size_x_beats) << ", which is less than 75%. " 
//                     << " Please increase the tile size ( max: " << OPS_MAXTILESIZE_X << ") to utilize more performance"<< std::endl;
//         }

//         if (tile_dim == 2) {
//             if (tile_size[1] <= overlap_size[1]) {
//                 OPSException ex(OPS_RUNTIME_ERROR);
//                 ex << "ERROR: y tile_size (" << tile_size[1] << ") is less than the minimum required overlap size (" << overlap_size[1] << ") in y direction. " 
//                         << "Please make sure appropriate OPS_TILESIZE_Y runtime flag is properly set";
//                 throw ex;
//             }
//         #ifdef OPS_MAXTILESIZE_Y
//             if (tile_size[1] > OPS_MAXTILESIZE_Y) {
//                 OPSException ex(OPS_RUNTIME_ERROR);
//                 ex << "ERROR: y tile_size (" << tile_size[1] << ") is greater than the minimum tile supported by the generated hardware (" << OPS_MAXTILESIZE_Y << ") in y direction. " 
//                         << "Please make sure appropriate OPS_TILESIZE_Y runtime flag is properly set. If bigger tile size need, rebuild with bigger OPS_MAXTILESIZE_Y";
//                 throw ex;
//             }
//         #endif
//             if (tile_size[1] > realized_tile_size_y) {
//                 std::cout << "[OPS_WARNING]: Grid is smaller than tile_size in y direction. Running without tiling in y direction" << std::endl;
//             }
//         #ifdef OPS_MAXTILESIZE_Y
//             if (float(effective_tile_size_y) / float(tile_size[1] ) < 0.75) {
//                 std::cout << "[OPS_WARNING]: Effective tile size is: " << float(effective_tile_size_y) / float(tile_size[1] ) << ", which is less than 75%" 
//                         << " Please increase the tile size ( max: " << OPS_MAXTILESIZE_Y << ") to utilize more performance"<< std::endl;
//             }
//         #endif
//         }
//     #endif
// #endif
        // Total xblocks calculations
        const unsigned short diff_z = range.end[2] - range.start[2];
        const unsigned short tile_count_min_1_x = tile_count_x - 1;
        const unsigned short tile_count_min_1_y = tile_count_y - 1;
        unsigned int iterior_tile_count = tile_count_min_1_x * tile_count_min_1_y;
        unsigned int interior_x_blocks = iterior_tile_count * diff_z * realized_tile_size_x_beats * realized_tile_size_y;
        unsigned int ab_x_blocks = diff_z * last_tile_size_x_beats * realized_tile_size_y * tile_count_min_1_y;
        unsigned int ba_x_blocks = diff_z * last_tile_size_y * realized_tile_size_x_beats * tile_count_min_1_x;
        unsigned int last_tile_x_blocks = diff_z * last_tile_size_x_beats * last_tile_size_y;
        total_xblocks =  interior_x_blocks + ab_x_blocks + ba_x_blocks + last_tile_x_blocks;

        grid_size[0] = grid_xblocks;
        range.start[0] = start_x;
        range.end[0] = end_x;
        tile_size[0] = isWideMem ? realized_tile_size_x_beats : (realized_tile_size_x_beats << ShiftBits);
        tile_size[1] = realized_tile_size_y;
        overlap_size[0] = isWideMem ? overlap_size_x_beats : (overlap_size_x_beats << ShiftBits);
        overlap_size[1] = overlap_size[1];
        effective_tile_size[0] = isWideMem ? effective_tile_size_x_beats : (effective_tile_size_x_beats << ShiftBits);
        effective_tile_size[1] = effective_tile_size_y;
        last_tile_size[0] = isWideMem ? last_tile_size_x_beats : (last_tile_size_x_beats << ShiftBits);
        last_tile_size[1] = last_tile_size_y;
        tile_count[0] = tile_count_x;
        tile_count[1] = tile_count_y;

        // #ifdef DEBUG_LOG
        //     printf("|HLS DEBUG LOG|%s| genTileMetadata output -> tile_count: (%d, %d), tile_size: (%d, %d), overlap_size: (%d, %d), effective_tile_size: (%d, %d), last_tile_size: (%d, %d)\n",
        //     __func__, tile_count[0], tile_count[1], tile_size[0], tile_size[1], overlap_size[0], overlap_size[1], effective_tile_size[0], effective_tile_size[1], last_tile_size[0], last_tile_size[1]);
        //     printf("|HLS DEBUG LOG|%s| xblocks breakdown -> total_xblocks: %u, interior: %u, ab: %u, ba: %u, last_tile: %u\n",
        //     __func__, total_xblocks, interior_x_blocks, ab_x_blocks, ba_x_blocks, last_tile_x_blocks);
        // #endif
}

#include <cstdio> // Ensure printf is available if not already included

const unsigned short get_overlap_size(unsigned short mem_vector_factor, unsigned short half_span, unsigned short total_PEs) {
    auto val =  ((total_PEs * half_span  + mem_vector_factor - 1) / mem_vector_factor) * mem_vector_factor * 2;

// #ifdef DEBUG_LOG_PRINT
//     printf("|DEBUG_LOG|%s| inputs: mem_vector_factor=%u, half_span=%u, total_PEs=%u | return val=%u\n",
//             __func__, mem_vector_factor, half_span, total_PEs, (unsigned int)val);
// #endif

    return val;
}

unsigned short getMinTileSize(unsigned short vector_factor, unsigned short mem_vector_factor) 
{
    unsigned short val = (vector_factor * OPS_HLS_TILE_BANKS / mem_vector_factor) << 1;

// #ifdef DEBUG_LOG_PRINT
//     printf("|DEBUG_LOG|%s| inputs: vector_factor=%u, mem_vector_factor=%u | return val=%u\n",
//             __func__, vector_factor, mem_vector_factor, val);
// #endif

    return val;
}

unsigned short getInterleaveGridSizeX(unsigned short actual_grid_size_x, unsigned short half_span, unsigned short vector_factor, unsigned short mem_vector_factor, unsigned short total_PEs) 
{
    unsigned short init_grid_size_x = ((actual_grid_size_x + mem_vector_factor - 1) / mem_vector_factor) * mem_vector_factor;

#ifdef DEBUG_LOG_PRINT
    printf("|DEBUG_LOG|%s| inputs: actual_grid_size_x=%u, half_span=%u, vector_factor=%u, mem_vector_factor=%u, total_PEs=%u\n",
            __func__, actual_grid_size_x, half_span, vector_factor, mem_vector_factor, total_PEs);
    printf("|DEBUG_LOG|%s| init_grid_size_x: %u\n", __func__, init_grid_size_x);
#endif

    ops::hls::SizeType mock_grid_size = {init_grid_size_x, 1, 1};
    ops::hls::AccessRange mock_read_range;
    mock_read_range.start[0] = 0;
    mock_read_range.start[1] = 0;
    mock_read_range.start[2] = 0;
    mock_read_range.end[0] = init_grid_size_x;
    mock_read_range.end[1] = 1;
    mock_read_range.end[2] = 1;
    mock_read_range.dim = 3;
    ops::hls::SizeType2d tile_size = {ops::hls::FPGA::getInstance()->getOPSTileSizeX(),  ops::hls::FPGA::getInstance()->getOPSTileSizeY()};
    ops::hls::SizeType2d overlap_size = {get_overlap_size(mem_vector_factor, half_span, total_PEs), get_overlap_size(1,half_span, total_PEs)};
    ops::hls::SizeType2d tile_count;
    ops::hls::SizeType2d effective_tile_size;
    ops::hls::SizeType2d last_tile_size;
    unsigned int total_xblocks_widen;
    unsigned short last_tile_upper_limit_x;

    genTileMetadataCPU(mem_vector_factor, 2, mock_grid_size, mock_read_range, tile_size, overlap_size,
            effective_tile_size, last_tile_size, tile_count, last_tile_upper_limit_x, total_xblocks_widen);
    
    unsigned short min_tile_size_val = getMinTileSize(vector_factor, mem_vector_factor);
    unsigned short adjusted_last_tile_x = last_tile_size[0] < min_tile_size_val ? min_tile_size_val : last_tile_size[0];

    unsigned short final_grid_size_x = (init_grid_size_x + (adjusted_last_tile_x - last_tile_size[0]) * mem_vector_factor);

#ifdef DEBUG_LOG_PRINT
    printf("|DEBUG_LOG|%s| genTileMetadataCPU results: effective_tile_size={%u, %u}, last_tile_size={%u, %u}, tile_count={%u, %u}\n",
            __func__, effective_tile_size[0], effective_tile_size[1], last_tile_size[0], last_tile_size[1], tile_count[0], tile_count[1]);
    printf("|DEBUG_LOG|%s| genTileMetadataCPU results: last_tile_upper_limit_x=%u, total_xblocks_widen=%u\n",
            __func__, last_tile_upper_limit_x, total_xblocks_widen);
    printf("|DEBUG_LOG|%s| adjusted_last_tile_x=%u | return final_grid_size_x=%u\n",
            __func__, adjusted_last_tile_x, final_grid_size_x);
#endif

    return final_grid_size_x;
}
#endif

#ifndef OPS_HLS_V2
ops::hls::GridPropertyCore createGridPropery(const unsigned short dim,
        const unsigned short multidim_dim,
		const ops::hls::SizeType& size,
		const ops::hls::SizeType& d_m,
		const ops::hls::SizeType& d_p,
        const int batch_size = 1,
		const unsigned short vector_factor=8,
		const unsigned short mem_vector_factor=16,
		const unsigned short total_PEs=1)
{
	ops::hls::GridPropertyCore gridProp;
	gridProp.dim = dim;
    gridProp.multidim_dim = multidim_dim;
    gridProp.batch_size = batch_size;

	for (int i = 0; i < ops_max_dim; i++)
	{
		gridProp.size[i] = size[i];
		gridProp.d_m[i] = d_m[i];
		gridProp.d_p[i] = d_p[i];
		gridProp.actual_size[i] = gridProp.size[i] + gridProp.d_p[i] + gridProp.d_m[i];
		gridProp.grid_size[i] = gridProp.actual_size[i];
	}
#if defined(OPS_HLS_TILE_INTERLEAVE)
	
	// unsigned short adj_mem_vector_factor = mem_vector_factor * OPS_HLS_TILE_BANKS;
	// gridProp.xblocks = (gridProp.actual_size[0] + adj_mem_vector_factor - 1) / adj_mem_vector_factor;
	// gridProp.grid_size[0] = gridProp.xblocks * adj_mem_vector_factor;
	gridProp.grid_size[0] = getInterleaveGridSizeX(gridProp.actual_size[0], gridProp.d_p[0], vector_factor, mem_vector_factor, total_PEs);
	printf("[WARNING]  OPS_HLS_TILE_INTERLEAVE based grid_size_x adjustment. Original size_x: %d, actual size_x: %d, mem_vector_factor: %d, bank_size: %d, adjusted grid size_x:%d, total_PEs: %d\n", gridProp.size[0], gridProp.actual_size[0], mem_vector_factor, OPS_HLS_TILE_BANKS, gridProp.grid_size[0], total_PEs);
// #elif defined(OPS_HLS_TILE_BANKS)
// 	gridProp.xblocks = (gridProp.actual_size[0] + mem_vector_factor - 1) / mem_vector_factor;
// 	gridProp.grid_size[0] = gridProp.xblocks * mem_vector_factor;
// 	//making sure, grid_size[1] >= banks
// 	gridProp.grid_size[1] = ((gridProp.grid_size[1] + OPS_HLS_TILE_BANKS - 1) / OPS_HLS_TILE_BANKS) * OPS_HLS_TILE_BANKS;
#else 
	gridProp.xblocks = (gridProp.actual_size[0] + mem_vector_factor - 1) / mem_vector_factor;
	gridProp.grid_size[0] = gridProp.xblocks * mem_vector_factor;
#endif

	//this will be changed according to the stencils
	gridProp.outer_loop_limit = gridProp.actual_size[gridProp.dim - 1] + gridProp.d_p[gridProp.dim - 1];

	gridProp.total_itr = gridProp.xblocks;

	for (int i = 1; i < gridProp.dim; i++)
	{
		gridProp.total_itr *= gridProp.actual_size[i];
	}

	return gridProp;
}
#else
ops::hls::GridPropertyCoreV2 createGridPropery(const unsigned short dim,
        const unsigned short multidim_dim,
		const ops::hls::SizeType& size,
		const ops::hls::SizeType& d_m,
		const ops::hls::SizeType& d_p,
        const int batch_size = 1,
		const unsigned short vector_factor=8,
		const unsigned short mem_vector_factor=16,
		const unsigned short total_PEs=1)
{
	ops::hls::GridPropertyCoreV2 gridProp;
    gridProp.multidim_dim = multidim_dim;
	gridProp.dim = dim;
    gridProp.batch_size = batch_size;

	for (int i = 0; i < ops_max_dim; i++)
	{
		gridProp.size[i] = size[i];
		gridProp.d_m[i] = d_m[i];
		gridProp.d_p[i] = d_p[i];
		gridProp.actual_size[i] = gridProp.size[i] + gridProp.d_p[i] + gridProp.d_m[i];
		gridProp.grid_size[i] = gridProp.actual_size[i];
	}

#if defined(OPS_HLS_TILE_INTERLEAVE)
	// unsigned short adj_mem_vector_factor = mem_vector_factor * OPS_HLS_TILE_BANKS;
	// unsigned short xblocks = (gridProp.actual_size[0] + adj_mem_vector_factor - 1) / adj_mem_vector_factor;
	// gridProp.grid_size[0] = xblocks * adj_mem_vector_factor;
	gridProp.grid_size[0] = getInterleaveGridSizeX(gridProp.actual_size[0], gridProp.d_p[0], vector_factor, mem_vector_factor, total_PEs);
	printf("[INFO]  OPS_HLS_TILE_INTERLEAVE based grid_size_x adjustment. Original size_x: %d, actual size_x: %d, mem_vector_factor: %d, bank_size: %d, adjusted grid size_x:%d, total_PEs: %d\n", gridProp.size[0], gridProp.actual_size[0], mem_vector_factor, OPS_HLS_TILE_BANKS, gridProp.grid_size[0], total_PEs);
// #elif defined(OPS_HLS_TILE_BANKS)
// 	unsigned short xblocks = (gridProp.actual_size[0] + mem_vector_factor - 1) / mem_vector_factor;
// 	gridProp.grid_size[0] = xblocks * mem_vector_factor;
// 	//making sure, grid_size[1] >= banks
// 	gridProp.grid_size[1] = ((gridProp.grid_size[1] + OPS_HLS_TILE_BANKS - 1) / OPS_HLS_TILE_BANKS) * OPS_HLS_TILE_BANKS;
// 	printf("[INFO]  ROW_TILING based grid_size_x adjustment. original size_x: %d, actual size_x: %d, actual size_y: %d, mem_vector_factor: %d, bank_size: %d, adjusted grid size_x:%d, adjusted grid size_y:%d, total_PEs: %d\n", gridProp.size[0], gridProp.actual_size[0], gridProp.actual_size[1], mem_vector_factor, OPS_HLS_TILE_BANKS, gridProp.grid_size[0],  gridProp.grid_size[1], total_PEs);
#else 
	unsigned short xblocks = (gridProp.actual_size[0] + mem_vector_factor - 1) / mem_vector_factor;
	gridProp.grid_size[0] = xblocks * mem_vector_factor;
#endif

	return gridProp;
}
#endif

ops::hls::Block ops_hls_decl_block(int dims, std::string name)
{
	ops::hls::Block block;
	block.dims = dims;
	block.name = name;

	return block;
}

ops::hls::Block ops_hls_decl_block_batch(int dims, std::string name, int batch_size)
{
    ops::hls::Block block;
    block.dims = dims;
    block.name = std::string(name);
    block.batch_size = batch_size;
    
    return block;
}

template <typename T>
ops::hls::Grid<T> ops_hls_decl_dat(ops::hls::Block& block, int elem_size, int* size,
		int * base, int* d_m, int* d_p, T* data_ptr, std::string type, std::string name,
		unsigned short vector_factor=8,
		unsigned short mem_vector_factor=16,
		unsigned short total_PEs=1)
{
	ops::hls::SizeType size_, d_m_, d_p_;

	for (unsigned int i = 0; i < ops_max_dim; i++)
	{
		if (i < block.dims)
		{
			size_[i] = static_cast<unsigned short>(size[i]);
			d_m_[i] = static_cast<unsigned short>(-d_m[i]);
			d_p_[i] = static_cast<unsigned short>(d_p[i]);
		}
		else
		{
			size_[i] = 1;
			d_m_[i] = 0;
			d_p_[i] = 0;
		}
	}

	ops::hls::Grid<T> grid;
	grid.originalProperty = createGridPropery(block.dims, elem_size, size_, d_m_, d_p_, block.batch_size, vector_factor, mem_vector_factor, total_PEs);

	unsigned int data_size = elem_size;
    
	for (int i = 0; i < block.dims; i++)
		data_size *= grid.originalProperty.grid_size[i];
	
    data_size *= grid.originalProperty.batch_size;

	
	grid.hostBuffer.resize(data_size);
#ifdef DEBUG_LOG
	printf("[Host Buffer allocated] {\n");
	printf("  grid_size : (");
	for (int i = 0; i < ops_max_dim; ++i) {
		printf("%d", grid.originalProperty.grid_size[i]);
		if (i < ops_max_dim - 1) printf(", ");
	}
	printf(")\n");
	printf("  total grid_elements: %u\n", data_size);
	printf("  host_ptr  : %p\n",        static_cast<void*>(grid.hostBuffer.data()));
	printf("  size      : %zu bytes\n", grid.hostBuffer.size() * sizeof(T));
	printf("  size_mb   : %.3f MB\n",   (grid.hostBuffer.size() * sizeof(T)) / (1024.0 * 1024.0));
	printf("  elements  : %zu\n",       grid.hostBuffer.size());
	printf("  type_size : %zu bytes\n", sizeof(T));
	printf("  cached    : false\n");
	printf("}\n");
#endif
    // std::cout << "What " << std::endl;
#ifdef OPS_TILING
    // If tiling is enabled, we may need to allocate extra buffer space for row tiles
    if (grid.alt_banks > 1)
    {
        auto alt_buffer_sizes = grid.getAltBufferSizes();
        for (int bank = 0; bank < grid.alt_banks; bank++) {
            grid.altHostBuffers[bank].resize(alt_buffer_sizes[bank]);
        }
    }
#endif
	grid.isSetAsArg = false;

	if (data_ptr != nullptr)
	{
		memcpy(grid.hostBuffer.data(), data_ptr, data_size);
		grid.isHostBufDirty = true;
		grid.isDevBufDirty = false;
#ifdef OPS_TILING
        grid.splitGrid();
#endif
	}
	else
	{
		grid.isHostBufDirty = false;
		grid.isDevBufDirty = false;
	}

#ifndef OPS_TILING
	grid.deviceBuffer = ops::hls::FPGA::getInstance()->createDeviceBuffer(CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, grid.hostBuffer);
#else
    if (grid.alt_banks == 1) {
        grid.deviceBuffer.push_back(ops::hls::FPGA::getInstance()->createDeviceBuffer(CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, grid.hostBuffer));
    }
    else {
        for (int bank = 0; bank < grid.alt_banks; bank++) {
            grid.deviceBuffer.push_back(ops::hls::FPGA::getInstance()->createDeviceBuffer(CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, grid.altHostBuffers[bank]));
        }
    }
#endif
	return grid;
}

template <typename T>
void ops_free_dat(ops::hls::Grid<T>& grid)
{
	for (auto event : grid.activeEvents)
	{
		event.wait();
	}
	grid.activeEvents.clear();
	grid.allEvents.clear();
	ops::hls::FPGA::getInstance()->deleteDeviceBuffer(grid.hostBuffer);
}

void getAdjustedRange(
#ifndef OPS_HLS_V2
    ops::hls::GridPropertyCore& gridProp, 
#else
    ops::hls::GridPropertyCoreV2& gridProp,
#endif
    ops::hls::AccessRange& original, ops::hls::AccessRange& adjusted, ops::hls::SizeType d_m=default_d_m, ops::hls::SizeType d_p=default_d_p)
{
	adjusted.dim = original.dim;

	for (int i = 0; i < 3; i++)
	{
		if (i < original.dim)
		{
			adjusted.end[i] = original.end[i] + d_p[i];
			adjusted.start[i] = original.start[i] - d_m[i];

			assert((adjusted.end[i]) <= gridProp.actual_size[i]);
			assert((adjusted.start[i]) >= 0);
		}
		else
		{
			adjusted.end[i] = 1;
			adjusted.start[i] = 0;
		}
	}
}

#ifndef OPS_HLS_V2
void getRangeAdjustedGridProp(ops::hls::GridPropertyCore& original,
		ops::hls::AccessRange& range, ops::hls::GridPropertyCore& adjusted, const unsigned short vector_factor=8, ops::hls::SizeType d_m=default_d_m, ops::hls::SizeType d_p=default_d_p)
{
	assert(range.dim == original.dim);

	for (int i = 0; i < range.dim; i++)
	{
		assert((range.end[i] + d_p[i]) <= original.actual_size[i]);
		assert((range.start[i] - d_m[i]) >= 0);
	}
	adjusted.dim = original.dim;

	for (int i = 0; i < range.dim; i++)
	{
		adjusted.size[i] = range.end[i] - range.start[i];
		adjusted.d_m[i] = d_m[i];
		adjusted.d_p[i] = d_p[i];
		adjusted.actual_size[i] = adjusted.size[i] + adjusted.d_p[i] + adjusted.d_m[i];

		if (i == 0)
		{
			adjusted.xblocks = (adjusted.actual_size[0] + vector_factor - 1) / vector_factor;
			adjusted.grid_size[0] = adjusted.xblocks * vector_factor;
			adjusted.total_itr = adjusted.xblocks;
		}
		else
		{
			adjusted.grid_size[i] = adjusted.actual_size[i];
			adjusted.total_itr *= adjusted.actual_size[i];
		}
	}
    adjusted.batch_size = original.batch_size;
	adjusted.outer_loop_limit = adjusted.actual_size[adjusted.dim - 1] + (adjusted.d_m[adjusted.dim-1] + adjusted.d_p[adjusted.dim -1])/2;
}
#else
ops::hls::StencilConfigCore getStencilConfig(ops::hls::GridPropertyCoreV2& original, ops::hls::AccessRange& range, const unsigned short stencil_vector_factor=1,
        const unsigned short mem_vector_factor=1, ops::hls::SizeType d_m=default_d_m, ops::hls::SizeType d_p=default_d_p)
{
    assert(range.dim == original.dim);
    assert(mem_vector_factor % stencil_vector_factor == 0);
    auto vector_factor_ratio = mem_vector_factor / stencil_vector_factor;

    ops::hls::StencilConfigCore stencilConfig;
    stencilConfig.dim = original.dim;

    for (unsigned short i = 0; i < range.dim; i++)
    {
        if (i == 0)
        {
            unsigned short start_x = range.start[i] - d_m[i];
            unsigned short end_x = range.end[i] + d_p[i];
            assert(start_x >= 0);
            assert(end_x <= original.actual_size[i]);
            unsigned short start_xblock = start_x / mem_vector_factor * vector_factor_ratio;
            unsigned short end_xblock = (end_x + mem_vector_factor - 1) / mem_vector_factor * vector_factor_ratio;
            unsigned short start_xblock_aligned = start_xblock * stencil_vector_factor;
//            unsigned short end_xblock_aligned = end_xblock * stencil_vector_factor;
            stencilConfig.lower_limit[i] = range.start[i] - start_xblock_aligned;
            unsigned short size = range.end[i] - range.start[i];
            stencilConfig.upper_limit[i] = stencilConfig.lower_limit[i] + size;
            stencilConfig.grid_size[i] = end_xblock - start_xblock; //xblocks
            stencilConfig.total_itr = stencilConfig.grid_size[i];

        	if (i == range.dim - 1)
        	{
        		stencilConfig.outer_loop_limit = stencilConfig.grid_size[i] + d_m[i];
        	}
        }
        else
        {
        	unsigned short start = range.start[i] - d_m[i];
        	unsigned short end = range.end[i] + d_p[i];
        	stencilConfig.lower_limit[i] = d_m[i];
        	unsigned short size = range.end[i] - range.start[i];
        	stencilConfig.upper_limit[i] = stencilConfig.lower_limit[i] + size;
        	stencilConfig.grid_size[i] = end - start;

        	if (i == range.dim - 1)
        	{
        		stencilConfig.outer_loop_limit = stencilConfig.grid_size[i] + d_m[i];
        		stencilConfig.total_itr *= stencilConfig.grid_size[i];
        	}
        	else
        	{
        		stencilConfig.total_itr *= stencilConfig.grid_size[i];
        	}
        }
    }

    stencilConfig.batch_size = original.batch_size;

    return stencilConfig;
}

ops::hls::StencilConfigCore getStencilConfig(ops::hls::GridPropertyCoreV2& original, ops::hls::AccessRange& range, ops::hls::StencilConfigCore& reference, signed short stencil_vector_factor=1,
        const unsigned short mem_vector_factor=1, ops::hls::SizeType d_m=default_d_m, ops::hls::SizeType d_p=default_d_p)
{

    ops::hls::StencilConfigCore stencilConfig = reference;
    stencilConfig.outer_loop_limit = reference.grid_size[reference.dim - 1] + d_m[reference.dim - 1];
    return stencilConfig;
}
#endif


#ifndef OPS_HLS_V2
unsigned int getOffset(const int* stencilOffset, ops::hls::GridPropertyCore& gridProp,const unsigned short i, const unsigned short j = 0, const unsigned short k = 0)
#else
unsigned int getOffset(const int* stencilOffset, ops::hls::GridPropertyCoreV2& gridProp, const unsigned short i, const unsigned short j = 0, const unsigned short k = 0)
#endif
{
    return ((i + (stencilOffset[0]
            + (j + stencilOffset[1]) * gridProp.grid_size[0]
            + (k + stencilOffset[2]) * gridProp.grid_size[0] * gridProp.grid_size[1])) * gridProp.multidim_dim);

}

template<typename T>
unsigned int getTotalBytes(ops::hls::GridPropertyCore& gridProp)
{
	unsigned int total_bytes = sizeof(T);

	for (unsigned short i = 0; i < gridProp.dim; i++)
	{
		total_bytes *= gridProp.grid_size[i];
	}

	return total_bytes;
}

template<typename T>
void printGrid2D(ops::hls::Grid<T> p_grid, std::string prompt="")
{
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << " [DEBUG] grid values: " << prompt << std::endl;
	std::cout << "----------------------------------------------" << std::endl;

    for (int k = 0; k < p_grid.originalProperty.batch_size; k++)
    {
        std::cout << "----------- batch: " << k <<"----------" << std::endl;
        for (int j = 0; j < p_grid.originalProperty.grid_size[1]; j++)
        {
            for (int i = 0; i < p_grid.originalProperty.grid_size[0]; i++)
            {
                int index = i + j * p_grid.originalProperty.grid_size[0] 
                        + k * p_grid.originalProperty.grid_size[0] * p_grid.originalProperty.grid_size[1];
                std::cout << std::setw(12) << p_grid.hostBuffer[index];
            }
            std::cout << std::endl;
        }
        std::cout << "----------------------------------------------" << std::endl;
    }
}

template<typename T>
#ifndef OPS_HLS_V2
void printGrid2D(T* p_grid, ops::hls::GridPropertyCore& gridProperty, std::string prompt="")
#else
void printGrid2D(T* p_grid, ops::hls::GridPropertyCoreV2& gridProperty, std::string prompt="")
#endif
{
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << " [DEBUG] grid values: " << prompt << std::endl;
	std::cout << "----------------------------------------------" << std::endl;

    for (int k = 0; k < gridProperty.batch_size; k++)
    {
        std::cout << "----------- batch: " << k <<"----------" << std::endl;
        for (int j = 0; j < gridProperty.grid_size[1]; j++)
        {
            for (int i = 0; i < gridProperty.grid_size[0]; i++)
            {
                int index = i + j * gridProperty.grid_size[0] + 
                        k * gridProperty.grid_size[0] * gridProperty.grid_size[1];

                if (gridProperty.multidim_dim == 1)
                {
                    std::cout << std::setw(12) << p_grid[index];
                }
                else
                {
                    std::cout << "[";
                    for (int m_dim = 0; m_dim < gridProperty.multidim_dim; m_dim++)
                    {
                        std::cout << std::setw(12) << p_grid[index * gridProperty.multidim_dim + m_dim];
                    }
                    std::cout << "]";
                }
            }
            std::cout << std::endl; 
        }
        std::cout << "----------------------------------------------" << std::endl;
		
	}
}

template<typename T>
#ifndef OPS_HLS_V2
void printGrid3D(T* p_grid, ops::hls::GridPropertyCore& gridProperty, std::string prompt="")
#else
void printGrid3D(T* p_grid, ops::hls::GridPropertyCoreV2& gridProperty, std::string prompt="")
#endif
{
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << " [DEBUG] grid values: " << prompt << std::endl;
	std::cout << "----------------------------------------------" << std::endl;

#if defined(OPS_HLS_TILE_INTERLEAVE)
	unsigned short alt_bank_counter = 0;
	unsigned short bank_data_counter = 0;
#endif 
	for (int k = 0; k < gridProperty.grid_size[2]; k++)
	{
		std::cout << "----------- plane: " << k <<"----------" << std::endl;

		for (int j = 0; j < gridProperty.grid_size[1]; j++)
		{
			for (int i = 0; i < gridProperty.grid_size[0]; i++)
			{
				int index = i + j * gridProperty.grid_size[0] + k * gridProperty.grid_size[0] * gridProperty.grid_size[1];
#if defined(OPS_HLS_TILE_INTERLEAVE)
				if (bank_data_counter == 0)
					std::cout << std::setw(4) << "|b" << alt_bank_counter;
#endif
				std::cout << std::setw(12) << p_grid[index];
#if defined(OPS_HLS_TILE_INTERLEAVE)
				bank_data_counter++;
				
				if (bank_data_counter == mem_vector_factor) {
					alt_bank_counter++;
					bank_data_counter = 0;
				}

				if (alt_bank_counter == OPS_HLS_TILE_BANKS or i == gridProperty.grid_size[0] - 1) {
					alt_bank_counter = 0;
				}
#endif
			}
#if defined(OPS_HLS_TILE_INTERLEAVE)
				std::cout << "|";
#endif
			std::cout << std::endl;
		}
	}
}

#ifdef OPS_TILING
template<typename T>
void print3D_host_tiles(ops::hls::Grid<T>& grid, std::string prompt="")
{
	if (grid.alt_banks > 1)
	{
		std::cout << "----------------------------------------------" << std::endl;
		std::cout << " [DEBUG] tile grid values: " << prompt << std::endl;
		std::cout << "----------------------------------------------" << std::endl;

	#if defined(OPS_HLS_TILE_INTERLEAVE)
		size_t total_vect_element_x = (grid.originalProperty.grid_size[0] + mem_vector_factor - 1) / mem_vector_factor;
        auto size_y_mult_size_z = grid.originalProperty.grid_size[1] * grid.originalProperty.grid_size[2];

		for (unsigned int b = 0; b < grid.originalProperty.batch_size; b++) 
		{
			for (int tile_bank = 0; tile_bank < grid.alt_banks; tile_bank++)
			{
				std::cout << "		----------------------------------------------" << std::endl;
				std::cout << " 		[DEBUG] bank: " << tile_bank  << std::endl;
				std::cout << "		----------------------------------------------" << std::endl;

				for (int k = 0; k < grid.originalProperty.grid_size[2]; k++)
				{
					std::cout << "----------- plane: " << k <<"----------" << std::endl;

					for (int j = 0; j < grid.originalProperty.grid_size[1]; j++)
					{
						std::cout << "----------- row: " << j <<"----------" << std::endl;
						for (unsigned int i = 0; i < grid.originalProperty.grid_size[0]; i+=mem_vector_factor) {
							size_t src_offset = i + j * grid.originalProperty.grid_size[0] 
										+ k * grid.originalProperty.grid_size[0] * grid.originalProperty.grid_size[1] 
										+ b * grid.originalProperty.grid_size[0] * grid.originalProperty.grid_size[1] * grid.originalProperty.grid_size[2];
							size_t vect_i = i / mem_vector_factor;
							// unsigned long vectored_index = abs_index / mem_vector_factor;
							unsigned int bank = vect_i % grid.alt_banks;
							unsigned int bank_size_x = (total_vect_element_x / grid.alt_banks) + (bank < (total_vect_element_x % grid.alt_banks) ? 1 : 0);
							size_t bank_index = vect_i / grid.alt_banks 
									+ j * bank_size_x 
									+ k * bank_size_x * grid.originalProperty.grid_size[1];
							bank_index *= mem_vector_factor;

							if (bank == tile_bank) {
								for (unsigned short v = 0; v < mem_vector_factor; v++) 
									std::cout << std::setw(12) << grid.altHostBuffers[tile_bank][bank_index + v];
							}
						}
						std::cout << std::endl;
					}
				}
			}
		}
	#else
		auto alt_buf_row_counts = grid.getAltBufferRowsCounts();

		for (int tile_bank = 0; tile_bank < grid.alt_banks; tile_bank++)
		{
			std::cout << "		----------------------------------------------" << std::endl;
			std::cout << " 		[DEBUG] bank: " << tile_bank << " row_count: " << alt_buf_row_counts[tile_bank] << std::endl;
			std::cout << "		----------------------------------------------" << std::endl;

			// for (int k = 0; k < grid.originalProperty.grid_size[2]; k++)
			// {
			// 	std::cout << "----------- plane: " << k <<"----------" << std::endl;

				for (int j = 0; j < alt_buf_row_counts[tile_bank]; j++)
				{
					for (int i = 0; i < grid.originalProperty.grid_size[0]; i++)
					{
						int index = i + j * grid.originalProperty.grid_size[0]; // + k * grid.originalProperty.grid_size[0] * alt_buf_row_counts[tile_bank];
						std::cout << std::setw(12) << grid.altHostBuffers[tile_bank][index];
					}
					std::cout << std::endl;
				}
			// }
		}
	#endif 
	}
	else
	{
		// printGrid3D<float>(u_raw, u[bat].originalProperty, "u after computation");
		printGrid3D<T>(grid.hostBuffer.data(), grid.originalProperty, prompt);
	}
}
#else
template<typename T>
void print3D_host_tiles(ops::hls::Grid<T>& grid, std::string prompt="") {}
#endif

#ifndef OPS_HLS_V2
void opsRange2hlsRange(int dim, int* ops_range, ops::hls::AccessRange& range, ops::hls::GridPropertyCore& p_grid)
#else
void opsRange2hlsRange(int dim, int* ops_range, ops::hls::AccessRange& range, ops::hls::GridPropertyCoreV2& p_grid)
#endif
{
	assert(static_cast<unsigned int>(dim) <= ops_max_dim);
	range.dim = static_cast<unsigned short>(dim);

	for (int i = 0; i < dim; i++)
	{
		range.start[i] = ops_range[i*2] + p_grid.d_m[i];
		range.end[i] = ops_range[i*2 + 1] + p_grid.d_m[i];
	}

//	std::cout << "[DEBUG]|" <<__func__ <<"| " << "ops_grid: " << ""
//#ifdef DEBUG_LOG
//#endif
}

template<typename T>
void ops_dat_fetch_data(ops::hls::Grid<T>& p_grid, int part, char* data)
{
	auto gridsize = p_grid.originalProperty.grid_size;
	assert(data != nullptr);

	for (unsigned short d = p_grid.originalProperty.dim; d < ops_max_dim; d++)
	{
		gridsize[d] = 1;
	}

	T* cast_data = (T*)data;
	T* grid_host_data = (T*)p_grid.get_raw_pointer();

    for (unsigned int l = 0; l < p_grid.originalProperty.batch_size; l++)
    {
        unsigned int offset = l * gridsize[0] * gridsize[1] * gridsize[2];
        for (unsigned int k = 0; k < gridsize[2]; k++)
        {
            for (unsigned int j = 0; j < gridsize[1]; j++)
            {
                for (unsigned int i = 0; i < gridsize[0]; i++)
                {
                    unsigned int index = i + j * gridsize[0]
                            + k * gridsize[0] * gridsize[1] + offset;

                    grid_host_data[index] = cast_data[index];
                }
            }
        }
    }

	p_grid.isHostBufDirty = true;
	p_grid.isDevBufDirty = false;
}