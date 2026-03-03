// Auto-generated at 2026-01-27 16:58:13.332103 by ops-translator
#pragma once

typedef float stencil_type;
//#define OPS_HLS_ROW_TILES 1

constexpr unsigned short data_width = sizeof(stencil_type) * 8;
constexpr unsigned short max_depth = max_depth_bytes / data_width;
constexpr unsigned short line_buff_2d_depth = max_depth;
constexpr unsigned short line_buff_3d_depth = max_depth / 2;
constexpr unsigned short vector_factor = 8;
constexpr unsigned short mem_vector_factor = 16;
constexpr unsigned short iter_par_factor = 2;
constexpr unsigned short mem_data_width = data_width * mem_vector_factor;
constexpr unsigned short shift_bits = 3; 
constexpr unsigned short axis_data_width = data_width * vector_factor;
constexpr unsigned short num_of_pkts_per_beat = mem_data_width / axis_data_width;
#ifdef DEBUG_LOG
    #include <stdio.h>
#endif
