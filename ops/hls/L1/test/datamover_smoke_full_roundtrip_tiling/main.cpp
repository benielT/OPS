#include <iostream>
#include <random>
#include <vector>
#include "top.hpp"

//#define DEBUG_LOG
#define STENCIL_SIZE 3
#define MAX_SLR_NUM 3
#define MIN_P_SLR 2
#define MAX_P_SLR 3

unsigned short get_overlap_size(unsigned short num_slr, unsigned short p_slr, unsigned short stencil_size, unsigned short mem_vector_size){
    unsigned short half_span = (stencil_size -1)/2;
    unsigned short overlap_size = ((num_slr * p_slr * half_span + mem_vector_size - 1) / mem_vector_size) * mem_vector_size;

    return overlap_size;
}

bool verify(ap_uint<AXI_M_WIDTH>* buff_1, ap_uint<AXI_M_WIDTH>* buff_2, ops::hls::AccessRange& range, ops::hls::SizeType& gridSize)
{
    bool no_error = true;

#ifdef DEBUG_LOG
    std::cout << std:: endl << "[DEBUG] **** Verification ****" << std::endl;
    std::cout << "Range: (" << range.start[0] << ", " << range.start[1] << ", " << range.start[2] << ") -> (" 
              << range.end[0] << ", " << range.end[1] << ", " << range.end[2] << ")" << std::endl;
#endif

    for (unsigned short z = range.start[2]; z < range.end[2]; z++)
    {
        for (unsigned short y = range.start[1]; y < range.end[1]; y++)
        {
            for (unsigned short x = range.start[0]; x < range.end[0]; x++)
            {
                unsigned int index = z * gridSize[0] * gridSize[1] + y * gridSize[0] + x;
                ops::hls::DataConv converter_1, converter_2;

                converter_1.i = buff_1[index / (AXI_M_WIDTH / (sizeof(float) * 8))].range(((index % (AXI_M_WIDTH / (sizeof(float) * 8)) +1) * sizeof(float) * 8) -1, (index % (AXI_M_WIDTH / (sizeof(float) * 8))) * sizeof(float) * 8);
                converter_2.i = buff_2[index / (AXI_M_WIDTH / (sizeof(float) * 8))].range(((index % (AXI_M_WIDTH / (sizeof(float) * 8)) +1) * sizeof(float) * 8) -1, (index % (AXI_M_WIDTH / (sizeof(float) * 8))) * sizeof(float) * 8);

                if (converter_1.f != converter_2.f)
                {
#ifdef DEBUG_LOG
                    std::cerr << "[ERROR] Value mismatch at index: " << index 
                        << " buff_1 val: " << converter_1.f << " buff_2 val: " << converter_2.f  << std::endl;
#endif
                    no_error = false;
                }
                else
                {
#ifdef DEBUG_LOG
                    std::cout << "[INFO] Verification. Index: " << index
                            << " buff_1 val: " << converter_1.f << " buff_2 val: " << converter_2.f  << std::endl;  
#endif
                }
            }
        }
    }
    return no_error;
}
int main()
{
    std::cout << std::endl;
    std::cout << "*****************************************************************************" << std::endl;
    std::cout << "TESTING: ops::hls::mem2stream -> ops::hls::stream2mem 2mem smoke tiled (CSIM ONLY)" << std::endl;
    std::cout << "*****************************************************************************" << std::endl << std::endl;

    std::random_device rd;
    unsigned int seed = 7;
    std::mt19937 mtSeeded(seed);
    std::mt19937 mtRandom(rd());
    std::uniform_int_distribution<> distSize(40, 100);
    std::normal_distribution<float> distFloat(100, 10);
    std::uniform_int_distribution<unsigned short> distSlrNum(1, MAX_SLR_NUM);
    std::uniform_int_distribution<unsigned short> distPslr(MIN_P_SLR, MAX_P_SLR);
    ops::hls::DataConv converter;

    const int num_tests = 25;
    std::cout << "TOTAL NUMER OF TESTS: " << num_tests << std::endl;
    std::vector<bool> test_summary(10);

    for (int test_itr = 0; test_itr < num_tests; test_itr++)
    {
        std::cout << std::endl;
        std::cout << "**********************************" << std::endl;
        std::cout << " TEST " << test_itr << std::endl;
        std::cout << "**********************************" << std::endl;
        std::cout << std::endl;
        
        const unsigned short logical_x_size = distSize(mtSeeded);
        const unsigned short logical_y_size = distSize(mtSeeded);
        const unsigned short logical_z_size = distSize(mtSeeded); //logical_x_size;
        const unsigned short num_slr = distSlrNum(mtSeeded);
        const unsigned short p_slr = distPslr(mtSeeded);
        const unsigned short actual_x_size = logical_x_size + 2 * ((STENCIL_SIZE -1)/2);
        const unsigned short actual_y_size = logical_y_size + 2 * ((STENCIL_SIZE -1)/2);
        const unsigned short actual_z_size = logical_z_size + 2 * ((STENCIL_SIZE -1)/2);
        const unsigned short grid_x_size = ((actual_x_size + MEM_VECTOR_SIZE - 1) / MEM_VECTOR_SIZE) * MEM_VECTOR_SIZE;
        const unsigned short grid_y_size = actual_y_size;
        const unsigned short grid_z_size = actual_z_size;
        const unsigned short overlap_size_x = get_overlap_size(num_slr, p_slr, STENCIL_SIZE, MEM_VECTOR_SIZE);
        const unsigned short overlap_size_y = get_overlap_size(num_slr, p_slr, STENCIL_SIZE, 1);

        std::uniform_int_distribution<> distTileX(overlap_size_x + 8, 32);
        std::uniform_int_distribution<> distTileY(overlap_size_y + 8, 32);
        const int unconsolidated_tile_size_x = distTileX(mtSeeded);
        int tile_size_x = ((unconsolidated_tile_size_x + MEM_VECTOR_SIZE - 1) / MEM_VECTOR_SIZE) * MEM_VECTOR_SIZE;
        int tile_size_y = distTileY(mtSeeded);
        tile_size_x = tile_size_x > grid_x_size ? grid_x_size : tile_size_x;
        tile_size_y = tile_size_y > grid_y_size ? grid_y_size : tile_size_y;
        const int num_elems = grid_x_size * grid_y_size * grid_z_size;
        const int data_per_beat = AXI_M_WIDTH / (sizeof(float) * 8);
        const int num_beats = (num_elems + data_per_beat - 1) / data_per_beat;

        unsigned short d_p[] = {(STENCIL_SIZE -1)/2,(STENCIL_SIZE -1)/2, (STENCIL_SIZE -1)/2};
        short d_m[] = {-(STENCIL_SIZE -1)/2, -(STENCIL_SIZE -1)/2, -(STENCIL_SIZE -1)/2};
        
        unsigned short range_start_0 = 0;
        unsigned short range_end_0 = grid_x_size;
        unsigned short range_start_1 = 0;
        unsigned short range_end_1 = grid_y_size;
        unsigned short range_start_2 = 0;
        unsigned short range_end_2 = grid_z_size;
        unsigned short dim = 3;
        ops::hls::AccessRange range = {{range_start_0, range_start_1, range_start_2},
                                       {range_end_0, range_end_1, range_end_2},
                                       dim};
        ops::hls::SizeType gridSize = {grid_x_size, grid_y_size, grid_z_size};

        std::cout << "logical_x_size: " << logical_x_size << std::endl;
        std::cout << "logical_y_size: " << logical_y_size << std::endl;
        std::cout << "logical_z_size: " << logical_z_size << std::endl;
        std::cout << "actual_x_size: " << actual_x_size << std::endl;
        std::cout << "actual_y_size: " << actual_y_size << std::endl;
        std::cout << "actual_z_size: " << actual_z_size << std::endl;
        std::cout << "x_size: " << grid_x_size << std::endl;
        std::cout << "y_size: " << grid_y_size << std::endl;
        std::cout << "z_size: " << grid_z_size << std::endl;
        std::cout << "Number of total beats: " << num_beats << std::endl;
        std::cout << "num_slr: " << num_slr << " p_slr: " << p_slr << " total_p: " << num_slr * p_slr << std::endl;
        std::cout << "tile_size_x: " << tile_size_x << std::endl;
        std::cout << "tile_size_y: " << tile_size_y << std::endl;
        std::cout << "overlap_size_x: " << overlap_size_x << std::endl;
        std::cout << "overlap_size_y: " << overlap_size_y << std::endl;
        std::cout << "data_per_beat: " << data_per_beat << std::endl;


        ap_uint<AXI_M_WIDTH>* mem_in_b = new ap_uint<AXI_M_WIDTH>[num_beats];
        // ap_uint<AXI_M_WIDTH>* mem_in_b2 = new ap_uint<AXI_M_WIDTH>[num_beats];
        ap_uint<AXI_M_WIDTH>* mem_out_b = new ap_uint<AXI_M_WIDTH>[num_beats];
        // ap_uint<AXI_M_WIDTH>* mem_out_b2 = new ap_uint<AXI_M_WIDTH>[num_beats];

#ifdef DEBUG_LOG
        std::cout << std:: endl << "[DEBUG] **** mem values ****" << std::endl; 
#endif
        for (int beat = 0; beat < num_beats; beat++)
        {
            for (int i = 0; i < data_per_beat; i++)
            {
                unsigned int index = beat * data_per_beat + i;

                if (index < num_elems)
                {
					converter.f = index;//distFloat(mtRandom);
					mem_in_b[beat].range((i+1)*sizeof(float)*8 - 1, i * sizeof(float)*8) = converter.i;
#ifdef DEBUG_LOG
					std::cout << "index: " << index << " value: " << converter.f << std::endl;
#endif
                }
            }
        }
#ifdef DEBUG_LOG
        std::cout << std::endl << "Starting DUT execution..." << std::endl;
#endif
        //calling test dut
        dut(mem_in_b, mem_in_b, mem_out_b, mem_out_b, dim, grid_x_size, grid_y_size, grid_z_size,
            range_start_0, range_end_0, range_start_1, range_end_1, range_start_2, range_end_2, tile_size_x, tile_size_y, overlap_size_x, overlap_size_y);

        bool no_error = true;

//         for (int beat = 0; beat < num_beats; beat++)
//         {
//             for (int i = 0; i < data_per_beat; i++)
//             {
//                 int index = beat * data_per_beat + i;

//                 if (index < num_elems)
//                 {
//                     ops::hls::DataConv tmp_in, tmp_out;

//                     tmp_in.i = mem_in_b[beat].range((i+1)*sizeof(float)*8 - 1, i * sizeof(float)*8);
//                     tmp_out.i = mem_out_b[beat].range((i+1)*sizeof(float)*8 - 1, i * sizeof(float)*8);
// #ifdef DEBUG_LOG
//                     std::cout << "[INFO] Verification. Index: " << index
//                     		<< " mem_in val: " << tmp_in.f << " mem_out val: " << tmp_out.f  << std::endl;
// #endif
//                     if (mem_in_b[beat].range((i+1)*sizeof(float)*8 - 1, i * sizeof(float)*8) != mem_out_b[beat].range((i+1)*sizeof(float)*8 - 1, i * sizeof(float)*8))
//                     {
//                         no_error = false;

//                         std::cerr << "[ERROR] Value mismatch at index: " << index 
//                         		<< " mem_in val: " << tmp_in.f << " mem_out val: " << tmp_out.f  << std::endl;
//                     }
//                 }
//             }
//         }
        no_error = verify(mem_in_b, mem_out_b, range, gridSize);
        if (no_error)
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

        // Clean up memory
        delete[] mem_in_b;
        // delete[] mem_in_b2;
        delete[] mem_out_b;
        // delete[] mem_out_b2;
    }

    std::cout << std::endl;
    std::cout << "**********************************" << std::endl;
    std::cout << " TEST SUMMARY " << std::endl;
    std::cout << "**********************************" << std::endl;
    std::cout << std::endl;

    for (unsigned int test_itr = 0; test_itr < num_tests; test_itr++)
    {
        std::cout << "TEST " << test_itr <<": ";
        
        if (test_summary[test_itr])
            std::cout << "PASSED";
        else
            std::cout << "FAILED";

        std::cout << std::endl;
    }

    std::cout << std::endl;
    return 0;
}
