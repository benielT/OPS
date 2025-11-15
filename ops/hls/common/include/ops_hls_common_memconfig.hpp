#pragma once

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/** @file
  * @brief Vitis HLS specific common memory configuration definitions.
  * @author Beniel Thileepan
  * @details To contain common memory configuration definitions used in Vitis HLS components and Host code.
  */

namespace ops {
namespace hls {

    struct MemConfigTile {
        unsigned short start_x;
        unsigned short end_x;
        unsigned short start_y;
        unsigned short end_y;
        unsigned short start_z;
        unsigned short end_z;
        unsigned short grid_xblocks;
        unsigned short grid_size_y;
        unsigned short grid_size_z;
        unsigned int start_offset; 
        bool isContinous;
        unsigned short tile_count_x;
        unsigned short tile_count_y;
        unsigned int total_tile_count;
        unsigned short tile_size_x;
        unsigned short last_tile_size_x;
        unsigned short overlap_size_x;
        unsigned short effective_tile_size_x;
        unsigned short tile_size_y;
        unsigned short last_tile_size_y;
        unsigned short overlap_size_y;
        unsigned short effective_tile_size_y;
        unsigned short total_xblocks;
        unsigned int total_size_bytes;
    };
    struct MemConfig {
        unsigned short start_x;
        unsigned short end_x;
        unsigned short start_y;
        unsigned short end_y;
        unsigned short start_z;
        unsigned short end_z;
        unsigned short grid_xblocks;
        unsigned short grid_size_y;
        unsigned short grid_size_z;
        unsigned short num_xblocks;
        unsigned int x_tile_size;
        unsigned int x_tile_bytes;
        bool isContinous;
        unsigned int start_offset;
        unsigned short batch_size;
        unsigned int total_xblocks;
        unsigned int total_size_bytes;
    };

    template <unsigned short MEM_DATA_WIDTH, unsigned short AXIS_DATA_WIDTH, unsigned short DATA_WIDTH=32>
    void genMemConfigTile(SizeType& gridSize, AccessRange& range, unsigned short& tile_size_x, unsigned short& overlap_size_x, unsigned short& tile_size_y, unsigned short&overlap_size_y, MemConfigTile& config){
#ifndef __SYTHESIS__
        static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
                "MEM_DATA_WIDTH failed limit check");
#endif
        constexpr unsigned short data_vector_factor = MEM_DATA_WIDTH / DATA_WIDTH;
        unsigned short ShiftBits = (unsigned short)LOG2(data_vector_factor);
        unsigned short DataShiftBits = (unsigned short)LOG2(DATA_WIDTH/8);
        unsigned short start_x = range.start[0] >> ShiftBits;
        unsigned short end_x = (range.end[0] + data_vector_factor - 1) >> ShiftBits;
        unsigned short grid_xblocks = gridSize[0] >> ShiftBits; //GridSize[0] has to be MEM_DATA_WIDTH aligned
        unsigned short num_xblocks = end_x - start_x;
        // unsigned short read_x_size = num_xblocks << ShiftBits;
        unsigned short tile_size_x_beats = tile_size_x >> ShiftBits;
        unsigned short overlap_size_x_beats = overlap_size_x >> ShiftBits;
        // unsigned int read_x_size_bytes = read_x_size << DataShiftBits;

        config.start_x = start_x;
        config.end_x = end_x;
        config.grid_xblocks = grid_xblocks;
        // config.num_xblocks = num_xblocks;
        // config.read_x_size = read_x_size;
        // config.read_x_size_bytes = read_x_size_bytes;
        // Calculating tile count in X and Y directions
        config.tile_size_x = tile_size_x_beats;
        config.overlap_size_x = overlap_size_x_beats;
        unsigned short effective_tile_size_x = tile_size_x - overlap_size_x;
        unsigned short effective_tile_size_x_beats = effective_tile_size_x >> ShiftBits;

        config.effective_tile_size_x = effective_tile_size_x_beats;
        config.tile_size_y = tile_size_y;
        config.overlap_size_y = overlap_size_y;
        unsigned short effective_tile_size_y = tile_size_y - overlap_size_y;
        config.effective_tile_size_y = effective_tile_size_y;

        unsigned short tile_count_x = 1; 
        unsigned short tile_count_y = 1;
        unsigned short diff_y = 1;
        unsigned short diff_z = 1;

        if (range.dim > 1)
        {
            config.start_y = range.start[1];
            config.end_y = range.end[1];
            diff_y = range.end[1] - range.start[1];
            config.grid_size_y = gridSize[1];
            tile_count_x = num_xblocks <= tile_size_x_beats ? 1 : ((num_xblocks - tile_size_x_beats) + effective_tile_size_x_beats - 1) / effective_tile_size_x_beats + 1;
        }
        else
        {
            config.start_y = 0;
            config.end_y = 1;
            config.grid_size_y=1;
        }
        if (range.dim > 2)
        {
            config.start_z = range.start[2];
            config.end_z = range.end[2];
            diff_z = range.end[2] - range.start[2];
            config.grid_size_z = gridSize[2];
            tile_count_y = diff_y <= tile_size_y ? 1 : ((diff_y - tile_size_y ) + effective_tile_size_y - 1) / effective_tile_size_y + 1;
        }
        else
        {
            config.start_z = 0;
            config.end_z = 1;
            config.grid_size_z = 1;
        }

        unsigned short last_xblock_start_x = (tile_count_x - 1) * effective_tile_size_x_beats; 
        unsigned short last_tile_size_x = grid_xblocks - 2 * last_xblock_start_x - 1;
        config.last_tile_size_x = last_tile_size_x;
        unsigned short last_yblock_start_y = (tile_count_y - 1) * effective_tile_size_y; 
        unsigned short last_tile_size_y = diff_y - 2 * last_yblock_start_y - 1;
        config.last_tile_size_y = last_tile_size_y;

        unsigned int total_tiles = tile_count_x * tile_count_y;
        config.total_tile_count = total_tiles;
        config.isContinous = total_tiles == 1 and (grid_xblocks == num_xblocks or range.dim == 1) and (diff_y == gridSize[1] or range.dim != 3);
        config.start_offset = start_x + config.start_y * grid_xblocks + config.start_z * gridSize[1] * grid_xblocks;
        config.total_xblocks = total_tiles * diff_z * tile_size_x_beats;
        config.total_size_bytes = config.total_xblocks << ShiftBits << DataShiftBits;

#ifndef __SYTHESIS__
#ifdef DEBUG_LOG
        printf("|HLS DEBUG LOG|%s| Input -> range_dim: % d, range: (%d, %d, %d) -> (%d, %d, %d), gridSize: (%d, %d, %d), Shiftbits: %d, DataShiftBits: %d\n",__func__, range.dim, range.start[0],
                range.start[1], range.start[2], range.end[0], range.end[1], range.end[2], gridSize[0], gridSize[1], gridSize[2], ShiftBits, DataShiftBits);
        printf("|HLS DEBUG_LOG|%s| memconfig tile generated -> range: (%d(xblocks), %d, %d) --> (%d(xblocks), %d, %d), grid_size: (%d(xblocks), %d, %d), diff_y: %d, diff_z: %d,\n\
            tile_count_x: %d, tile_count_y: %d, total_tile_count: %u, tile_size_x: %d, last_tile_size_x: %d, overlap_size_x: %d, effective_tile_size_x: %d,\n\
            tile_size_y: %d, last_tile_size_y: %d, overlap_size_y: %d, effective_tile_size_y: %d, total_xblocks: %u, total_size_bytes: %u, isContinous: %d, start_offset: %u\n", __func__,
            config.start_x, config.start_y, config.start_z,
            config.end_x, config.end_y, config.end_z,
            config.grid_xblocks, config.grid_size_y, config.grid_size_z, diff_y, diff_z,
            tile_count_x, tile_count_y, config.total_tile_count,
            config.tile_size_x, config.last_tile_size_x, config.overlap_size_x, config.effective_tile_size_x,
            config.tile_size_y, config.last_tile_size_y, config.overlap_size_y, config.effective_tile_size_y,
            config.total_xblocks, config.total_size_bytes, config.isContinous, config.start_offset);
#endif
#endif
    }

    template <unsigned short MEM_DATA_WIDTH, unsigned short AXIS_DATA_WIDTH, unsigned short DATA_WIDTH=32>
    void genMemConfig(SizeType& gridSize, AccessRange& range, MemConfig& config, const unsigned short& batch_size = 1){
        constexpr unsigned short data_vector_factor = MEM_DATA_WIDTH / DATA_WIDTH;
        unsigned short ShiftBits = (unsigned short)LOG2(data_vector_factor);
        unsigned short DataShiftBits = (unsigned short)LOG2(DATA_WIDTH/8);
        unsigned short start_x = range.start[0] >> ShiftBits;
        unsigned short end_x = (range.end[0] + data_vector_factor - 1) >> ShiftBits;
        unsigned short grid_xblocks = gridSize[0] >> ShiftBits; //GridSize[0] has to be MEM_DATA_WIDTH aligned
        unsigned short num_xblocks = end_x - start_x;
        unsigned short x_tile_size = num_xblocks << ShiftBits;
        unsigned int x_tile_size_bytes = x_tile_size << DataShiftBits;

        config.start_x = start_x;
        config.end_x = end_x;
        config.grid_xblocks = grid_xblocks;
        config.num_xblocks = num_xblocks;
        config.x_tile_size = x_tile_size;
        config.x_tile_bytes = x_tile_size_bytes;
        unsigned short diff_y = 1;
        unsigned short diff_z = 1;

        if (range.dim > 1)
        {
            config.start_y = range.start[1];
            config.end_y = range.end[1];
            diff_y = range.end[1] - range.start[1];
            config.grid_size_y = gridSize[1];
        }
        else
        {
            config.start_y = 0;
            config.end_y = 1;
            config.grid_size_y=1;
        }
        if (range.dim > 2)
        {
            config.start_z = range.start[2];
            config.end_z = range.end[2];
            diff_z = range.end[2] - range.start[2];
            config.grid_size_z = gridSize[2];
        }
        else
        {
            config.start_z = 0;
            config.end_z = 1;
            config.grid_size_z = 1;
        }

        config.isContinous = (grid_xblocks == num_xblocks or range.dim == 1) and (diff_y == gridSize[1] or range.dim != 3);
        config.start_offset = start_x + config.start_y * grid_xblocks + config.start_z * gridSize[1] * grid_xblocks;
        config.batch_size = batch_size;
        config.total_xblocks = num_xblocks * diff_y * diff_z * batch_size;
        config.total_size_bytes = config.total_xblocks << ShiftBits << DataShiftBits;

#ifndef __SYTHESIS__
#ifdef DEBUG_LOG
        printf("|HLS DEBUG LOG|%s| Input -> range_dim: % d, range: (%d, %d, %d) -> (%d, %d, %d), gridSize: (%d, %d, %d), Shiftbits: %d, DataShiftBits: %d\n",__func__, range.dim, range.start[0],
                range.start[1], range.start[2], range.end[0], range.end[1], range.end[2], gridSize[0], gridSize[1], gridSize[2], ShiftBits, DataShiftBits);
        printf("|HLS DEBUG_LOG|%s| memconfig generated -> range: (%d(xblocks), %d, %d) --> (%d(xblocks), %d, %d), grid_size: (%d(xblocks), %d, %d), diff_x: %d, diff_y: %d,\n\
                num_xblocks: %d, x_tile_size: %d, x_tile_bytes: %d, isContinous: %d, start_offset: %d(xblocks), total_xblocks: %d, total_size_bytes: %d\n", __func__, config.start_x, config.start_y, config.start_z,
                config.end_x, config.end_y, config.end_z, config.grid_xblocks, config.grid_size_y, config.grid_size_z, diff_y, diff_z, config.num_xblocks, config.x_tile_size, config.x_tile_bytes, config.isContinous, config.start_offset,
                config.total_xblocks, config.total_size_bytes);
#endif
#endif
    }

    template <unsigned short MULTIDIM_DIM>
    void multidimConfigConverter(MemConfig& orig_cfg, MemConfig& mdim_cfg)
    {
        mdim_cfg.start_x = orig_cfg.start_x * MULTIDIM_DIM;
        mdim_cfg.end_x = orig_cfg.end_x * MULTIDIM_DIM;
        mdim_cfg.start_y = orig_cfg.start_y;
        mdim_cfg.end_y = orig_cfg.end_y;
        mdim_cfg.start_z = orig_cfg.start_z;
        mdim_cfg.end_z = orig_cfg.end_z ;
        mdim_cfg.grid_xblocks = orig_cfg.grid_xblocks * MULTIDIM_DIM;
        mdim_cfg.grid_size_y = orig_cfg.grid_size_y;
        mdim_cfg.grid_size_z = orig_cfg.grid_size_z;
        mdim_cfg.num_xblocks = orig_cfg.num_xblocks * MULTIDIM_DIM;
        mdim_cfg.x_tile_size = orig_cfg.x_tile_size * MULTIDIM_DIM;
        mdim_cfg.x_tile_bytes = orig_cfg.x_tile_bytes * MULTIDIM_DIM;
        mdim_cfg.isContinous = orig_cfg.isContinous;
        mdim_cfg.start_offset = orig_cfg.start_offset * MULTIDIM_DIM;
        mdim_cfg.batch_size = orig_cfg.batch_size;
        mdim_cfg.total_xblocks = orig_cfg.total_xblocks * MULTIDIM_DIM;
        mdim_cfg.total_size_bytes = orig_cfg.total_size_bytes * MULTIDIM_DIM;

#ifndef __SYTHESIS__
#ifdef DEBUG_LOG
        printf("|HLS DEBUG_LOG|%s| multidim memconfig generated for dim: %d -> range: (%d(xblocks), %d, %d) --> (%d(xblocks), %d, %d), grid_size: (%d(xblocks), %d, %d), \n\
                num_xblocks: %d, x_tile_size: %d, x_tile_bytes: %d, isContinous: %d, start_offset: %d(xblocks), batch_size: %d, total_xblocks: %d, total_size_bytes: %d\n", __func__, MULTIDIM_DIM, mdim_cfg.start_x, mdim_cfg.start_y, mdim_cfg.start_z,
                mdim_cfg.end_x, mdim_cfg.end_y, mdim_cfg.end_z, mdim_cfg.grid_xblocks, mdim_cfg.grid_size_y, mdim_cfg.grid_size_z, mdim_cfg.num_xblocks, mdim_cfg.x_tile_size, mdim_cfg.x_tile_bytes, mdim_cfg.isContinous, mdim_cfg.start_offset,
                mdim_cfg.batch_size, mdim_cfg.total_xblocks, mdim_cfg.total_size_bytes);
#endif
#endif
    }
}
}

#endif /* DOXYGEN_SHOULD_SKIP_THIS */