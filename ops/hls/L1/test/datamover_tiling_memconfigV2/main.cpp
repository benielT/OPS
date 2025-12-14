#include <iostream>
#include <random>
#include <vector>
#include "top.hpp"

#define STENCIL_SIZE 3
#define MEM_VECTOR_SIZE 16
#define TILE_SIZE_X 128
#define TILE_SIZE_Y 128

//#define DEBUG_LOG
int get_overlap_size(int num_slr, int p_slr, int stencil_size, int mem_vector_size){
    int half_span = (stencil_size -1)/2;
    int overlap_size = ((num_slr * p_slr * half_span + mem_vector_size - 1) / mem_vector_size) * mem_vector_size;

    return overlap_size;
}

int get_tile_count(int grid_size, int tile_size, int overlap_size){
    if (grid_size <= tile_size){
        return 1;
    }
    else{
        int effective_tile_size = tile_size - overlap_size;
        return ((grid_size - tile_size) + effective_tile_size - 1) / effective_tile_size + 1;
    }
}

struct Tile_descriptor{
    int start;
    int end;
    int size;
};

Tile_descriptor get_tile_desc(int tile_idx, int grid_size, int tile_size, int overlap_size){
    Tile_descriptor tile_desc;
    if (grid_size <= tile_size){
        tile_desc.start = 0;
        tile_desc.end = grid_size - 1;
        tile_desc.size = grid_size;
    }
    else{
        int effective_tile_size = tile_size - overlap_size;
        tile_desc.start = tile_idx * effective_tile_size;
        tile_desc.end = tile_desc.start + tile_size - 1;
        
        if (tile_desc.end >= grid_size){
            tile_desc.end = grid_size - 1;
        }

        tile_desc.size = tile_desc.end - tile_desc.start + 1;
    }
    return tile_desc;
}

bool  verify_memconfig_tile(ops::hls::MemConfigTile& memconfig, unsigned int tile_count_x, 
        unsigned int tile_count_y, 
        unsigned int grid_x_size, 
        unsigned int grid_y_size, 
        unsigned short overlap_size_x,
        unsigned short overlap_size_y)
{
    bool status = true;
    unsigned short expected_tile_size = memconfig.tile_size_x;
    unsigned short expected_overlap_size = overlap_size_x;
    unsigned int expected_total_tile_count = tile_count_x * tile_count_y;

    if (memconfig.total_tile_count != expected_total_tile_count){
        std::cout << "[ERROR] Total tile count mismatch. Expected: " << expected_total_tile_count << " Got: " << memconfig.total_tile_count << std::endl;
        status = false;
    }

    if (memconfig.tile_size_x != TILE_SIZE_X / MEM_VECTOR_SIZE){
        std::cout << "[ERROR] Tile size mismatch. Expected: " << TILE_SIZE_X / MEM_VECTOR_SIZE << " Got: " << memconfig.tile_size_x << std::endl;
        status = false;
    }

    if (memconfig.tile_size_y != TILE_SIZE_Y){
        std::cout << "[ERROR] Tile size Y mismatch. Expected: " << TILE_SIZE_Y << " Got: " << memconfig.tile_size_y << std::endl;
        status = false;
    }

    if (memconfig.tile_overlap_size_x != overlap_size_x / MEM_VECTOR_SIZE){
        std::cout << "[ERROR] Overlap size mismatch. Expected: " << overlap_size_x / MEM_VECTOR_SIZE << " Got: " << memconfig.tile_overlap_size_x << std::endl;
        status = false;
    }

    if (memconfig.tile_overlap_size_y != overlap_size_y ){
        std::cout << "[ERROR] Overlap size mismatch. Expected: " << overlap_size_y << " Got: " << memconfig.tile_overlap_size_y << std::endl;
        status = false;
    }

    return status;  
}



int main()
{
    std::cout << std::endl;
    std::cout << "*****************************************************************************" << std::endl;
    std::cout << "TESTING: tiling memconfig 2D (CSIM ONLY)" << std::endl;
    std::cout << "*****************************************************************************" << std::endl << std::endl;

    std::random_device rd;
    unsigned int seed = 7;
    std::mt19937 mtSeeded(seed);
    std::mt19937 mtRandom(rd());
    std::uniform_int_distribution<> distInt(300, 40000);
    std::normal_distribution<float> distFloat(100, 10);
    std::uniform_int_distribution<> distSlrNum(1, MAX_SLR_NUM);
    std::uniform_int_distribution<> distPslr(MIN_P_SLR, MAX_P_SLR);
    ops::hls::DataConv converter;

    const int num_tests  = 15;
    std::cout << "TOTAL NUMER OF TESTS: " << num_tests << std::endl;
    std::vector<bool> test_summary(num_tests);

    for (int test_itr = 0; test_itr < num_tests; test_itr++)
    {
        std::cout << std::endl;
        std::cout << "**********************************" << std::endl;
        std::cout << " TEST " << test_itr << std::endl;
        std::cout << "**********************************" << std::endl;
        std::cout << std::endl;

        // const int logical_x_size = 300;
        // const int logical_y_size = 300;
        // const int logical_z_size = 300;
        // const int num_slr = 3;
        // const int p_slr = 18;
        const int logical_x_size = distInt(mtSeeded);
        const int logical_y_size = logical_x_size;
        const int logical_z_size = logical_x_size;
        const int num_slr = distSlrNum(mtSeeded);
        const int p_slr = distPslr(mtSeeded);
        const int actual_x_size = logical_x_size + 2 * ((STENCIL_SIZE -1)/2);
        const int actual_y_size = logical_y_size + 2 * ((STENCIL_SIZE -1)/2);
        const int actual_z_size = logical_z_size + 2 * ((STENCIL_SIZE -1)/2);
        const int grid_x_size = ((actual_x_size + MEM_VECTOR_SIZE - 1) / MEM_VECTOR_SIZE) * MEM_VECTOR_SIZE;
        const int grid_y_size = actual_y_size;
        const int grid_z_size = actual_z_size;

        int overlap_size_x = get_overlap_size(num_slr, p_slr, STENCIL_SIZE, MEM_VECTOR_SIZE);
        int overlap_size_y = get_overlap_size(num_slr, p_slr, STENCIL_SIZE, 1);
        int tile_count_x = get_tile_count(grid_x_size, TILE_SIZE_X, overlap_size_x);
        int tile_count_y = get_tile_count(grid_y_size, TILE_SIZE_Y, overlap_size_y);
        std::cout << "Logincal grid size: (" << logical_x_size << ", " << logical_y_size << ", " << logical_z_size << ")" << std::endl;
        std::cout << "Actual grid size: (" << actual_x_size << ", " << actual_y_size << ", " << actual_z_size << ")" << std::endl;
        std::cout << "Grid size (mem aligned): (" << grid_x_size << ", " << grid_y_size << ", " << grid_z_size << ")" << std::endl;

        std::cout << "num_slr: " << num_slr << " p_slr: " << p_slr << " total_p: " << num_slr * p_slr << std::endl;
        std::cout << "overlap_size_x: " << overlap_size_x << std::endl;
        std::cout << "tile_count_x: " << tile_count_x << " tile_count_y: " << tile_count_y << std::endl;
        
        // for (int tile_x = 0; tile_x < tile_count_x; tile_x++){
        //     Tile_descriptor tile_desc_x = get_tile_desc(tile_x, grid_x_size, TILE_SIZE_X, overlap_size_x);
        //     for (int tile_y = 0; tile_y < tile_count_y; tile_y++){
        //         Tile_descriptor tile_desc_y = get_tile_desc(tile_y, grid_y_size, TILE_SIZE_Y, overlap_size_x);
        //         std::cout << "Tile (" << tile_x << ", " << tile_y << "): ";
        //         std::cout << " X[" << tile_desc_x.start << ", " << tile_desc_x.end << "] Size: " << tile_desc_x.size;
        //         std::cout << " Y[" << tile_desc_y.start << ", " << tile_desc_y.end << "] Size: " << tile_desc_y.size;
        //         std::cout << std::endl;
        //     }
        // }

        ops::hls::SizeType gridSize = { (unsigned short)grid_x_size, (unsigned short)grid_y_size, (unsigned short)grid_z_size};
        unsigned short d_p[] = {(STENCIL_SIZE -1)/2,(STENCIL_SIZE -1)/2, (STENCIL_SIZE -1)/2};
        short d_m[] = {-(STENCIL_SIZE -1)/2, -(STENCIL_SIZE -1)/2, -(STENCIL_SIZE -1)/2};
        ops::hls::AccessRange range = {{ (unsigned short)(1 + d_m[0]), (unsigned short)(1 + d_m[1]), (unsigned short)(1 + d_m[2]) }, 
                { (unsigned short)(grid_x_size - 2 + d_p[0]), (unsigned short)(grid_y_size - 2 + d_p[1]), (unsigned short)(grid_z_size - 2 + d_p[2])}, 3};
        ops::hls::MemConfigTile memconfig;

        //initializing tile meta to use with ops::hls::genTileMetaData
        ops::hls::SizeType2d tile_size = {(unsigned short)TILE_SIZE_X, (unsigned short)TILE_SIZE_Y};
        ops::hls::SizeType2d tile_count = {(unsigned short)tile_count_x, (unsigned short)tile_count_y};
        ops::hls::SizeType2d overlap_size = {(unsigned short)overlap_size_x, (unsigned short)overlap_size_y};
        ops::hls::SizeType2d effective_tile_size = {(unsigned short)(TILE_SIZE_X - overlap_size_x), (unsigned short)(TILE_SIZE_Y - overlap_size_y)};
        ops::hls::SizeType2d last_tile_size = {(unsigned short)0, (unsigned short)0};
        
    #ifdef DEBUG_LOG
        std::cout << "[DEBUG] tile_size: (" << tile_size[0] << ", " << tile_size[1] << ")" << std::endl;
        std::cout << "[DEBUG] tile_count: (" << tile_count[0] << ", " << tile_count[1] << ")" << std::endl;
        std::cout << "[DEBUG] overlap_size: (" << overlap_size[0] << ", " << overlap_size[1] << ")" << std::endl;
        std::cout << "[DEBUG] effective_tile_size: (" << effective_tile_size[0] << ", " << effective_tile_size[1] << ")" << std::endl;
    #endif
        ops::hls::SizeType gridSize_copy = {gridSize[0], gridSize[1], gridSize[2]};
        ops::hls::AccessRange range_copy = {{range.start[0], range.start[1], range.start[2]},
                                                {range.end[0], range.end[1], range.end[2]}, range.dim};
        ops::hls::SizeType2d tile_size_copy = {tile_size[0], tile_size[1]};
        ops::hls::SizeType2d overlap_size_copy = {overlap_size[0], overlap_size[1]};

        // SizeType& gridSize, 
        //     AccessRange& range, 
        //     SizeType& tile_size,
        //     SizeType& tile_count,
        //     SizeType& overlap_size,
        //     SizeType& effective_tile_size,
        //     SizeType& last_tile_size,
        //     MemConfigTile& config)
        ops::hls::genTileMetadata<AXI_M_WIDTH, DATA_WIDTH>(gridSize_copy, range_copy, tile_size_copy, overlap_size_copy, effective_tile_size, last_tile_size, tile_count, true);
        dut(gridSize_copy, range_copy, tile_size_copy, tile_count, overlap_size_copy, effective_tile_size, last_tile_size, memconfig);

        //  dut(gridSize, range, TILE_SIZE_X, TILE_SIZE_Y, overlap_size_x, overlap_size_y, memconfig);
        // ops::hls::MemConfigTile expected_memconfig;
        // unsigned short tile_size_x = (unsigned short) TILE_SIZE_X;
        // unsigned short tile_size_y = (unsigned short) TILE_SIZE_Y;
        // unsigned short overlap_size_x_u = (unsigned short) overlap_size_x;
        // unsigned short overlap_size_y_u = (unsigned short) overlap_size_y;
        // ops::hls::genMemConfigTile<AXI_M_WIDTH, AXIS_WIDTH, DATA_WIDTH>(gridSize, range, tile_size_x, overlap_size_x_u, tile_size_y, 
        //         overlap_size_y_u, expected_memconfig);
        if(verify_memconfig_tile(memconfig, tile_count_x, tile_count_y, grid_x_size, grid_y_size, overlap_size_x, overlap_size_y))
        {
            std::cout << "TEST PASSED." << std::endl;
            test_summary[test_itr] = true;
        }
        else
        {
            std::cout << "TEST FAILED." << std::endl;
            test_summary[test_itr] = false;
        }
        std::cout << std::endl;
    }
//     for (int test_itr = 0; test_itr < num_tests; test_itr++)
//     {
//         std::cout << std::endl;
//         std::cout << "**********************************" << std::endl;
//         std::cout << " TEST " << test_itr << std::endl;
//         std::cout << "**********************************" << std::endl;
//         std::cout << std::endl;
        
//         const int logical_x_size = distInt(mtSeeded);
//         const int num = logical_x_size * logical_x_size * logical_x_size; 
//         const int size = num * sizeof(float);
//         const int bytes_per_beat = AXI_M_WIDTH / 8;
//         const int data_per_beat = bytes_per_beat / sizeof(float);
//         const int num_beats = (size + bytes_per_beat - 1) / bytes_per_beat;

//         std::cout << "logical_x_size: " << logical_x_size << std::endl;
//         std::cout << "Size(Bytes): " << size << std::endl;
//         std::cout << "Number of total beats: " << num_beats << std::endl;

//         ap_uint<AXI_M_WIDTH> mem0[num_beats];
//         ap_uint<AXI_M_WIDTH> mem1[num_beats];

// #ifdef DEBUG_LOG
//         std::cout << std:: endl << "[DEBUG] **** mem values ****" << std::endl; 
// #endif
//         for (int beat = 0; beat < num_beats; beat++)
//         {
//             for (int i = 0; i < data_per_beat; i++)
//             {
//                 unsigned int index = beat * data_per_beat + i;

//                 if (index < num)
//                 {
// 					converter.f = distFloat(mtRandom);
// 					mem0[beat].range((i+1)*sizeof(float)*8 - 1, i * sizeof(float)*8) = converter.i;
// #ifdef DEBUG_LOG

// 					std::cout << "index: " << index << " value: " << converter.f << std::endl;
// #endif
//                 }
//             }
//         }

        //calling test dut
//         dut(mem0, mem1, size);

//         bool no_error = true;

//         for (int beat = 0; beat < num_beats; beat++)
//         {
//             for (int i = 0; i < data_per_beat; i++)
//             {
//                 int index = beat * data_per_beat + i;

//                 if (index < num)
//                 {
//                     ops::hls::DataConv tmp1, tmp2;

//                     tmp1.i = mem0[beat].range((i+1)*sizeof(float)*8 - 1, i * sizeof(float)*8);
//                     tmp2.i = mem1[beat].range((i+1)*sizeof(float)*8 - 1, i * sizeof(float)*8);
// #ifdef DEBUG_LOG
//                     std::cout << "[desc] Verification. Index: " << beat * data_per_beat + i
//                     		<< " mem0 val: " << tmp1.f << " mem1 val: " << tmp2.f  << std::endl;
// #endif
//                     if (mem0[beat].range((i+1)*sizeof(float)*8 - 1, i * sizeof(float)*8) != mem1[beat].range((i+1)*sizeof(float)*8 - 1, i * sizeof(float)*8))
//                     {
//                         no_error = false;

//                         std::cerr << "[ERROR] Value mismatch. Index: " << beat * data_per_beat + i 
//                         		<< " mem0 val: " << tmp1.f << " mem1 val: " << tmp2.f  << std::endl;
//                     }
//                 }
//             }
//         }
//         if (no_error)
//         {
//             std::cout << "TEST PASSED." << std::endl;
//             test_summary[test_itr] = true;
//         }
//         else
//         {
//             std::cout << "TEST FAILED." << std::endl;
//             test_summary[test_itr] = false;
//         }
//         std::cout << std::endl;
//     }

    // std::cout << std::endl;
    // std::cout << "**********************************" << std::endl;
    // std::cout << " TEST SUMMARY " << std::endl;
    // std::cout << "**********************************" << std::endl;
    // std::cout << std::endl;

    // for (unsigned int test_itr = 0; test_itr < num_tests; test_itr++)
    // {
    //     std::cout << "TEST " << test_itr <<": ";
        
    //     if (test_summary[test_itr])
    //         std::cout << "PASSED";
    //     else
    //         std::cout << "FAILED";

    //     std::cout << std::endl;
    // }

    std::cout << std::endl;
    return 0;
}
