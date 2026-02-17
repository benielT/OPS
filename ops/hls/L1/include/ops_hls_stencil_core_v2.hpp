#pragma once

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/** @file
  * @brief Vitis HLS specific L1 abstract stencil core class
  * @author Beniel Thileepan
  * @details Implements of the template stencil class.
  * 
  */

#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <cstdarg>
#include <math.h>
#include "../../common/include/ops_hls_defs.hpp"
#include "../../common/include/ops_hls_utils.hpp"
#include <stdio.h>

/**
 * TODO: This version assume no reduction and arg_dat and arg_const passed 
 */
namespace ops
{
namespace hls
{

template <typename T, unsigned short NUM_POINTS, unsigned short VEC_FACTOR, CoefTypes COEF_TYPE,
        unsigned short STENCIL_SIZE_X, unsigned short STENCIL_DIM, bool TILED=false>
class StencilCoreV2
{
    public:
    	static constexpr unsigned short s_datatype_size = sizeof(T) * 8;
    	static constexpr unsigned short s_axis_width = VEC_FACTOR * s_datatype_size;
    	static constexpr unsigned short s_datatype_bytes = sizeof(T);
    	static constexpr unsigned short s_mask_width = VEC_FACTOR * s_datatype_bytes;
        static constexpr unsigned short s_singe_index_size = size_singleIndex;
        typedef ap_uint<s_axis_width> widen_dt;
        typedef ap_uint<s_mask_width> mask_dt;
        typedef ::hls::stream<widen_dt> widen_stream_dt;
        typedef ::hls::stream<mask_dt> mask_stream_dt;

        typedef typename TypeSelector<TILED, StencilConfigCoreSingleTile, StencilConfigCore>::Result configType;

        StencilCoreV2()
        {
#ifndef __SYTHESIS__
            static_assert(s_dim <= ops_max_dim, "Stencil cannot have more than maximum dimention supported by OPS_MAX_s_dim");
            static_assert(s_axis_width >= min_axis_data_width && s_axis_width <= max_axis_data_width,
			        "axis_width failed limit check. VEC_FACTOR and T should be within limits");
#endif        
        }

        void setConfig(const short& PEId, const configType& stencilConfig)
        {
        	// setConfigImpl(PEId, stencilConfig, BoolType<TILED>{});
            m_PEId = PEId;
            m_stencilConfig = stencilConfig;
        }

        // void setPoints(const unsigned short * stencilPoints)
        // {
        //     for (unsigned short i = 0; i < NUM_POINTS * s_dim; i++)
        //     {
        //         m_stencilPoints[i] = stencilPoints[i];
        //     }
        // }


#ifndef __SYTHESIS__

        // void getPoints(unsigned short* stencilPoints)
        // {
        //     for (unsigned short i = 0; i < NUM_POINTS * s_dim; i++)
        //     {
        //         stencilPoints[i] = m_stencilPoints[i];
        //     }
        // }

        void getConfig(StencilConfigCore& stencilConfig)
        {
            stencilConfig = m_stencilConfig;
        }
#endif

    private:

        // void setConfigImpl(const short& PEId, const StencilConfigCore& stencilConfig, TrueTag)
        // {
        // 	m_PEId = PEId;
        //     m_stencilConfig = stencilConfig;
        // }

        // void setConfigImpl(const short& PEId, const StencilConfigCoreTiled& stencilConfig, FalseTag)
        // {
        // 	m_PEId = PEId;
        //     m_stencilConfig = stencilConfig;
        // }

    protected:
        static const unsigned short s_dim = STENCIL_DIM;
        // static const unsigned short s_size_x = STENCIL_SIZE_X;
        // static const unsigned short s_stencil_span_x = s_size_x - 1;
        // static const unsigned short s_stencil_half_span_x = s_stencil_span_x / 2;

        configType m_stencilConfig;
        // unsigned short m_stencilPoints[NUM_POINTS * 2];
        // unsigned short m_sizes[s_dim];
        short m_PEId;
        // SizeType m_lowerLimits;
        // SizeType m_upperLimits;

};

/**
 * TODO: INDEX_STENCIL_V2 is not completed. Not required now.
*/
/*
template <unsigned short VEC_FACTOR, unsigned short STENCIL_DIM>
class INDEX_STENCIL_V2 : public ops::hls::StencilCoreV2<unsigned short, 1, VEC_FACTOR, ops::hls::CoefTypes::CONST_COEF, 
        0, STENCIL_DIM>
{
public:
    static constexpr unsigned short s_index_size = size_IndexType;

    typedef ap_int<s_index_size> index_dt;
    using ops::hls::StencilCore<int, 1, VEC_FACTOR, ops::hls::CoefTypes::CONST_COEF, 
    0, STENCIL_DIM>::m_stencilConfig;
    // typedef typename ops::hls::StencilCore<int, 1, VEC_FACTOR, ops::hls::CoefTypes::CONST_COEF,
    //         0, STENCIL_DIM>::widen_dt widen_dt;
    typedef ::hls::stream<index_dt> index_stream_dt;    

    void idxRead(index_stream_dt idx_bus[vector_factor])
    {
        unsigned short i = 0;
#if(STENCIL_DIM > 1)
        unsigned short j = 0;
        unsigned short i_l = 0; // Line buffer index
#endif
#if(STENCIL_DIM > 2)
        unsigned short k = 0;
        unsigned short i_p = 0; // Plane buffer index
#endif
        
        ::ops::hls::StencilConfigCore stencilConfig = m_stencilConfig;
        unsigned short iter_limit = stencilConfig.total_itr;
// #if(STENCIL_DIM == 1)
//         unsigned short iter_limit = stencilConfig.total_itr;
// #elif(STENCIL_DIM == 2)      
//         unsigned short iter_limit = gridProp.outer_loop_limit * gridProp.xblocks;
// #else
//         unsigned short iter_limit = gridProp.outer_loop_limit *
//                 gridProp.grid_size[1] * gridProp.xblocks;
// #endif
        unsigned short total_itr = gridProp.total_itr;
        widen_dt read_val = 0;

        widen_dt stencilValues[1];

        //No buffer
        stencil_type rowArr_0[vector_factor];
        #pragma HLS ARRAY_PARTITION variable = rowArr_0 STENCIL_DIM=1 complete

        for (unsigned short itr = 0; itr < iter_limit; itr++)
        {
        #pragma HLS LOOP_TRIPCOUNT min=min_grid_size max=max_grid_size avg=avg_grid_size
        #pragma HLS PIPELINE II=1

            index_gen:
            {
                bool cond_x_terminate = (i == gridProp.xblocks - 1); 
#if(STENCIL_DIM == 2)
                bool cond_y_terminate = (j == gridProp.outer_loop_limit - 1);
#elif(STENCIL_DIM == 3)
                bool cond_y_terminate = (j == gridProp.grid_size[1] - 1);
                bool cond_z_terminate = (k == gridProp.outer_loop_limit - 1);
#endif

                if (cond_x_terminate)
                    i = 0;
                else
                    i++;
#if(STENCIL_DIM == 2)
                if (cond_x_terminate && cond_y_terminate)
                    j = 0;
                else if (cond_x_terminate)
                    j++;
#elif(STENCIL_DIM == 3)
                if (cond_x_terminate && cond_y_terminate && cond_z_terminate)
                    k = 0;
                else if (cond_x_terminate && cond_y_terminate)
                    k++;
#endif

#if(STENCIL_DIM > 1)
                bool cond_end_of_line_buff = (i_l) >= (gridProp.xblocks - 1);
#endif
#if(STENCIL_DIM > 2)
                bool cond_end_of_plane_buff = (i_p) >= (gridProp.plane_diff);
#endif

#if(STENCIL_DIM > 1)
                if (cond_end_of_line_buff)
                    i_l = 0;
                else
                    i_l++;
#endif
#if(STENCIL_DIM > 2)
                if (cond_end_of_plane_buff)
                    i_p = 0;
                else
                    i_p++;
#endif
            }
            
            process: for (unsigned short x = 0; x < VEC_FACTOR; x++)
            {
#pragma HLS UNROLL factor=VEC_FACTOR

                unsigned short index = (i << LOG2(VEC_FACTOR)) + x;
                index_dt indexPkt;
                ops::hls::IndexConv indexConv;
                indexConv.index[0] = index;
#if(STENCIL_DIM > 1)
                indexConv.index[1] = j;
#endif
#if(STENCIL_DIM > 2)
                indexConv.index[2] = k;
#endif
                indexPkt = indexConv.flatten;
                idx_bus[x].write(indexPkt);
            }
        }
    }  
};
*/

static StencilConfigCoreSingleTile stencilConfigCoreSingleTileGen2D(const StencilConfigCoreTiled& srcConfig, const unsigned short tile_x)
{
    StencilConfigCoreSingleTile tileConfig;

    tileConfig.dim = srcConfig.dim;
    const bool is_last_tile_x = (tile_x == srcConfig.tile_count[0] - 1);
    const bool is_last_tile_y = true;
    tileConfig.tile_size[0] = is_last_tile_x ? srcConfig.last_tile_size[0] : srcConfig.tile_size[0];
    tileConfig.tile_size[1] = srcConfig.grid_size[1];
    tileConfig.tile_size[2] = 1;
    tileConfig.outer_loop_limit = srcConfig.outer_loop_limit;
    //tileConfig.is_tiled[0] = srcConfig.tile_count[0] > 1;
    //tileConfig.is_tiled[1] = srcConfig.tile_count[1] > 1;
    tileConfig.is_first[0] = tile_x == 0;
    tileConfig.is_first[1] = true;
    tileConfig.is_last[0] = is_last_tile_x;
    tileConfig.is_last[1] = is_last_tile_y;
    tileConfig.last_tile_upper_limit_x = srcConfig.last_tile_upper_limit_x;

    return tileConfig;
}

static StencilConfigCoreSingleTile stencilConfigCoreSingleTileGen3D(const StencilConfigCoreTiled& srcConfig, const unsigned short tile_x, const unsigned short tile_y)
{
    StencilConfigCoreSingleTile tileConfig;

    tileConfig.dim = srcConfig.dim;
    const bool is_last_tile_x = (tile_x == srcConfig.tile_count[0] - 1);
    const bool is_last_tile_y = (tile_y == srcConfig.tile_count[1] - 1);
    tileConfig.tile_size[0] = is_last_tile_x ? srcConfig.last_tile_size[0] : srcConfig.tile_size[0];
    tileConfig.tile_size[1] = is_last_tile_y ? srcConfig.last_tile_size[1] : srcConfig.tile_size[1];
    tileConfig.tile_size[2] = srcConfig.grid_size[2];
    tileConfig.outer_loop_limit = srcConfig.outer_loop_limit;
    //tileConfig.is_tiled[0] = srcConfig.tile_count[0] > 1;
    //tileConfig.is_tiled[1] = srcConfig.tile_count[1] > 1;
    tileConfig.is_first[0] = tile_x == 0;
    tileConfig.is_first[1] = tile_y == 0;
    tileConfig.is_last[0] = is_last_tile_x;
    tileConfig.is_last[1] = is_last_tile_y;
    tileConfig.last_tile_upper_limit_x = srcConfig.last_tile_upper_limit_x;

    return tileConfig;
}

}
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
