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
        unsigned short start_y;
        unsigned short start_z;
        unsigned short end_x;
        unsigned short end_y;
        unsigned short end_z;
        unsigned short grid_xblocks; //grid_size_x with mem_widen
        unsigned short grid_size_y;
        unsigned short grid_size_z;
        unsigned short tile_count_x;
        unsigned short tile_count_y;
        unsigned int total_tile_count;
        unsigned short tile_size_x;
        unsigned short tile_size_y;
        unsigned short tile_overlap_size_x;
        unsigned short tile_overlap_size_y;
        unsigned short effective_tile_size_x;
        unsigned short effective_tile_size_y;
        unsigned short last_tile_size_x;
        unsigned short last_tile_size_y;
        unsigned int start_offset;
        bool isContinous;
        unsigned int total_xblocks;
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

    /**
     * @brief Generates tile metadata for memory access patterns with configurable data width and memory width.
     * 
     * This template function calculates tiling parameters including tile sizes, overlap regions, effective
     * tile dimensions, and tile counts for both X and Y dimensions. It supports variable memory data widths
     * and adjusts calculations based on whether the memory is wide memory or not.
     * 
     * @tparam MEM_DATA_WIDTH The memory data width in bits (must be between min_mem_data_width and max_mem_data_width)
     * @tparam DATA_WIDTH The individual data element width in bits (default: 32)
     * 
     * @param[in,out] grid_size The total grid dimensions [x, y]
     * @param[in,out] range The access range with start and end coordinates
     * @param[in,out] tile_size Input tile dimensions, output adjusted tile dimensions [x, y]
     * @param[in,out] overlap_size Input overlap dimensions, output adjusted overlap dimensions [x, y]
     * @param[out] effective_tile_size The effective tile size excluding overlap [x, y]
     * @param[out] last_tile_size The size of the last tile which may be smaller [x, y]
     * @param[out] tile_count The number of tiles needed to cover the access range [x, y]
     * @param[in] isWidMem Flag indicating if memory is wide memory format (default: true).
     *            When true, X dimensions are in beats; when false, they are converted to byte units.
     * 
     * @note Uses compile-time assertions to validate MEM_DATA_WIDTH constraints
     * @note All calculations use bit-shift operations for efficiency
     */
    template <unsigned short MEM_DATA_WIDTH, unsigned short DATA_WIDTH=32>
    void genTileMetadata(
            SizeType& grid_size, 
            AccessRange& range, 
            SizeType2d& tile_size,
            SizeType2d& overlap_size,
            SizeType2d& effective_tile_size,
            SizeType2d& last_tile_size,
            SizeType2d& tile_count,
            unsigned int& total_xblocks,
            bool isWidMem = true)
    {
#ifndef __SYTHESIS__
        static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
                "MEM_DATA_WIDTH failed limit check");
#endif
        constexpr unsigned short data_vector_factor = MEM_DATA_WIDTH / DATA_WIDTH;
        const unsigned short ShiftBits = (unsigned short)LOG2(data_vector_factor);
        const unsigned short DataShiftBits = (unsigned short)LOG2(DATA_WIDTH/8);
        const unsigned short start_x = range.start[0] >> ShiftBits;
        const unsigned short end_x = (range.end[0] + data_vector_factor - 1) >> ShiftBits;
        const unsigned short grid_xblocks = grid_size[0] >> ShiftBits;
        const unsigned short num_xblocks = end_x - start_x;

        const unsigned short tile_size_x_beats = tile_size[0] >> ShiftBits;
        const unsigned short overlap_size_x_beats = overlap_size[0] >> ShiftBits;

        const unsigned short effective_tile_size_x_beats = tile_size_x_beats - overlap_size_x_beats;
        const unsigned short effective_tile_size_y = tile_size[1] - overlap_size[1];

        const unsigned short diff_y = range.end[1] - range.start[1];

        const unsigned short realized_tile_size_x_beats = tile_size_x_beats > grid_xblocks ? grid_xblocks : tile_size_x_beats;
        const unsigned short tile_count_x = ((grid_xblocks - realized_tile_size_x_beats) + effective_tile_size_x_beats - 1) / effective_tile_size_x_beats + 1;
        const unsigned short last_tile_size_x_beats = tile_count_x > 1 ? grid_xblocks - (tile_count_x - 1) * effective_tile_size_x_beats : realized_tile_size_x_beats;

        const unsigned short realized_tile_size_y = tile_size[1] > diff_y ? diff_y : tile_size[1];
        const unsigned short tile_count_y = ((diff_y - realized_tile_size_y) + effective_tile_size_y - 1) / effective_tile_size_y + 1;
        const unsigned short last_tile_size_y = tile_count_y > 1 ? diff_y - (tile_count_y - 1) * effective_tile_size_y : realized_tile_size_y;
        
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
        tile_size[0] = isWidMem ? realized_tile_size_x_beats : (realized_tile_size_x_beats << ShiftBits);
        tile_size[1] = realized_tile_size_y;
        overlap_size[0] = isWidMem ? overlap_size_x_beats : (overlap_size_x_beats << ShiftBits);
        overlap_size[1] = overlap_size[1];
        effective_tile_size[0] = isWidMem ? effective_tile_size_x_beats : (effective_tile_size_x_beats << ShiftBits);
        effective_tile_size[1] = effective_tile_size_y;
        last_tile_size[0] = isWidMem ? last_tile_size_x_beats : (last_tile_size_x_beats << ShiftBits);
        last_tile_size[1] = last_tile_size_y;
        tile_count[0] = tile_count_x;
        tile_count[1] = tile_count_y;
        #ifndef __SYTHESIS__
        // #ifdef DEBUG_LOG
            printf("|HLS DEBUG LOG|%s| genTileMetadata output -> tile_count: (%d, %d), tile_size: (%d, %d), overlap_size: (%d, %d), effective_tile_size: (%d, %d), last_tile_size: (%d, %d)\n",
            __func__, tile_count[0], tile_count[1], tile_size[0], tile_size[1], overlap_size[0], overlap_size[1], effective_tile_size[0], effective_tile_size[1], last_tile_size[0], last_tile_size[1]);
            printf("|HLS DEBUG LOG|%s| xblocks breakdown -> total_xblocks: %u, interior: %u, ab: %u, ba: %u, last_tile: %u\n",
            __func__, total_xblocks, interior_x_blocks, ab_x_blocks, ba_x_blocks, last_tile_x_blocks);
        // #endif
        #endif
    }

    /**
     * @brief Generates memory configuration for tiled access patterns with specified data widths.
     * 
     * @tparam MEM_DATA_WIDTH The width of memory data in bits. Must be between min_mem_data_width and max_mem_data_width.
     * @tparam DATA_WIDTH The width of individual data elements in bits (default: 32).
     * 
     * @param gridSize Reference to the grid dimensions [x, y, z] in elements.
     * @param range Reference to the access range containing start and end coordinates for [x, y, z] dimensions.
     * @param tile_size Reference to the tile dimensions [x, y] in elements.
     * @param tile_count Reference to the number of tiles [x, y] to be generated.
     * @param overlap_size Reference to the overlap between tiles [x, y] in elements.
     * @param effective_tile_size Reference to the effective tile size [x, y] after accounting for overlap.
     * @param last_tile_size Reference to the size of the last tile [x, y] in elements.
     * @param config Reference to the output MemConfigTile structure to be populated with configuration.
     * 
     * @details 
     * This function computes memory configuration parameters for tiled data access, including:
     * - Tile dimensions and counts
     * - Overlap regions between tiles
     * - Memory alignment and byte offset calculations
     * - Continuity detection for optimized access patterns
     * - Total memory size requirements
     * 
     * The function calculates shift bits for vectorization based on MEM_DATA_WIDTH and DATA_WIDTH ratio,
     * and performs boundary condition checks for multi-dimensional tiling scenarios.
     * 
     * Debug logging is available when DEBUG_LOG is defined during non-synthesis compilation.
     * 
     * @note Memory access coordinates are assumed to be aligned according to MEM_DATA_WIDTH specifications.
     */
    template <unsigned short MEM_DATA_WIDTH, unsigned short DATA_WIDTH=32>
    void genMemConfigTileV2(
            const SizeType& gridSize, 
            const AccessRange& range, 
            const SizeType2d& tile_size,
            const SizeType2d& tile_count,
            const SizeType2d& overlap_size,
            const SizeType2d& effective_tile_size,
            const SizeType2d& last_tile_size,
            const unsigned int& total_xblocks,
            MemConfigTile& config)
    {
#ifndef __SYTHESIS__
        static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
                "MEM_DATA_WIDTH failed limit check");
#endif
        constexpr unsigned short data_vector_factor = MEM_DATA_WIDTH / DATA_WIDTH;
        const unsigned short ShiftBits = (unsigned short)LOG2(data_vector_factor);
        const unsigned short DataShiftBits = (unsigned short)LOG2(DATA_WIDTH/8);
        // const unsigned short start_x = range.start[0] >> ShiftBits;
        // const unsigned short end_x = (range.end[0] + data_vector_factor - 1) >> ShiftBits;
        // const unsigned short grid_xblocks = gridSize[0] >> ShiftBits; //GridSize[0] has to be MEM_DATA_WIDTH aligned

        const unsigned short num_xblocks = range.end[0] - range.start[0];
        
        config.start_x = range.start[0];
        config.start_y = range.start[1];
        config.start_z = range.start[2];
        config.end_x = range.end[0];
        config.end_y = range.end[1];
        config.end_z = range.end[2];
        config.grid_xblocks = gridSize[0];
        config.grid_size_y = gridSize[1];
        config.grid_size_z = gridSize[2];

        config.effective_tile_size_x = effective_tile_size[0];
        config.tile_size_x = tile_size[0];
        config.tile_overlap_size_x = overlap_size[0];
        config.last_tile_size_x = last_tile_size[0];
        config.tile_count_x = tile_count[0];
        config.tile_size_y = tile_size[1];
        config.tile_overlap_size_y = overlap_size[1];
        config.effective_tile_size_y = effective_tile_size[1];
        config.tile_count_y = tile_count[1];
        config.last_tile_size_y = last_tile_size[1];
        unsigned int total_tiles = tile_count[0] * tile_count[1];
        // const unsigned short tile_count_min_1_x = tile_count[0] - 1;
        // const unsigned short tile_count_min_1_y = tile_count[1] - 1;
        const unsigned short diff_y = range.end[1] - range.start[1];
        const unsigned short diff_z = range.end[2] - range.start[2];
        // unsigned int iterior_tile_count = tile_count_min_1_x * tile_count_min_1_y;
        // unsigned int interior_x_blocks = iterior_tile_count * diff_z * tile_size[0] * tile_size[1];
        // unsigned int ab_x_blocks = diff_z * last_tile_size[0] * tile_size[1] * tile_count_min_1_y;
        // unsigned int ba_x_blocks = diff_z * last_tile_size[1] * tile_size[0] * tile_count_min_1_x;
        // unsigned int last_tile_x_blocks = diff_z * last_tile_size[0] * last_tile_size[1];
        config.total_tile_count = total_tiles;
        
        config.isContinous = total_tiles == 1 and (gridSize[0] == num_xblocks or range.dim == 1) and (diff_y == gridSize[1] or range.dim != 3);
        config.start_offset = range.start[0] + range.start[1] * gridSize[0] + range.start[2] * gridSize[1] * gridSize[0];
        config.total_xblocks = total_xblocks;
        config.total_size_bytes = config.total_xblocks <<   ShiftBits << DataShiftBits;


    #ifndef __SYTHESIS__
    #ifdef DEBUG_LOG
        printf("|HLS DEBUG LOG|%s| Input -> range_dim: %d, range: (%d, %d, %d) -> (%d, %d, %d), gridSize: (%d, %d, %d), Shiftbits: %d, DataShiftBits: %d\n",__func__, range.dim, range.start[0],
            range.start[1], range.start[2], range.end[0], range.end[1], range.end[2], gridSize[0], gridSize[1], gridSize[2], ShiftBits, DataShiftBits);
        printf("|HLS DEBUG_LOG|%s| memconfig tile generated:\n"
               "  range: (%d, %d, %d) --> (%d, %d, %d) (xblocks)\n"
               "  grid_size: (%d, %d, %d) (xblocks)\n"
               "  diff_y: %d, diff_z: %d\n"
               "  tile_count: (%d, %d), total_tile_count: %u\n"
               "  tile_size_x: %d, last_tile_size_x: %d, overlap_x: %d, effective_x: %d\n"
               "  tile_size_y: %d, last_tile_size_y: %d, overlap_y: %d, effective_y: %d\n"
               "  total_xblocks: %u, total_size_bytes: %u\n"
               "  isContinous: %d, start_offset: %u\n",
               __func__,
               config.start_x, config.start_y, config.start_z,
               config.end_x, config.end_y, config.end_z,
               config.grid_xblocks, config.grid_size_y, config.grid_size_z,
               diff_y, diff_z,
               config.tile_count_x, config.tile_count_y, config.total_tile_count,
               config.tile_size_x, config.last_tile_size_x, config.tile_overlap_size_x, config.effective_tile_size_x,
               config.tile_size_y, config.last_tile_size_y, config.tile_overlap_size_y, config.effective_tile_size_y,
               config.total_xblocks, config.total_size_bytes, config.isContinous, config.start_offset);
    #endif
    #endif

    }

    template <unsigned short MEM_DATA_WIDTH, unsigned short AXIS_DATA_WIDTH, unsigned short DATA_WIDTH=32>
    void genMemConfigTile(
            SizeType& gridSize, 
            AccessRange& range, 
            unsigned short& tile_size_x, 
            unsigned short& overlap_size_x, 
            unsigned short& tile_size_y, 
            unsigned short&overlap_size_y, 
            MemConfigTile& config)
    {
#ifndef __SYTHESIS__
        static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
                "MEM_DATA_WIDTH failed limit check");
#endif
        constexpr unsigned short data_vector_factor = MEM_DATA_WIDTH / DATA_WIDTH;
        const unsigned short ShiftBits = (unsigned short)LOG2(data_vector_factor);
        const unsigned short DataShiftBits = (unsigned short)LOG2(DATA_WIDTH/8);
        const unsigned short start_x = range.start[0] >> ShiftBits;
        const unsigned short end_x = (range.end[0] + data_vector_factor - 1) >> ShiftBits;
        const unsigned short grid_xblocks = gridSize[0] >> ShiftBits; //GridSize[0] has to be MEM_DATA_WIDTH aligned
        const unsigned short num_xblocks = end_x - start_x;

        const unsigned short tile_size_x_beats = tile_size_x >> ShiftBits;
        const unsigned short overlap_size_x_beats = overlap_size_x >> ShiftBits;

        const unsigned short effective_tile_size_x_beats = tile_size_x_beats - overlap_size_x_beats;
        const unsigned short effective_tile_size_y = tile_size_y - overlap_size_y;

        const unsigned short diff_y = range.end[1] - range.start[1];
        const unsigned short diff_z = range.end[2] - range.start[2];

        // Initial values for config
        config.start_x = start_x;
        config.start_y = range.start[1];
        config.start_z = range.start[2];
        config.end_x = end_x;
        config.end_y = range.end[1];
        config.end_z = range.end[2];
        config.grid_xblocks = grid_xblocks;
        config.grid_size_y = gridSize[1];
        config.grid_size_z = gridSize[2];

        const unsigned short realized_tile_size_x_beats = tile_size_x_beats > grid_xblocks? grid_xblocks : tile_size_x_beats;
        const unsigned short tile_count_x = ((grid_xblocks - realized_tile_size_x_beats) + effective_tile_size_x_beats - 1) / effective_tile_size_x_beats + 1;
        const unsigned short last_tile_size_x_beats = tile_count_x > 1 ? grid_xblocks - (tile_count_x - 1) * effective_tile_size_x_beats : realized_tile_size_x_beats;

        const unsigned short realized_tile_size_y = tile_size_y > diff_y ? diff_y : tile_size_y;
        const unsigned short tile_count_y = ((diff_y - realized_tile_size_y) + effective_tile_size_y - 1) / effective_tile_size_y + 1;
        const unsigned short last_tile_size_y = tile_count_y > 1 ? diff_y - (tile_count_y - 1) * effective_tile_size_y : realized_tile_size_y;
        
        config.effective_tile_size_x = effective_tile_size_x_beats;
        config.tile_size_x = realized_tile_size_x_beats;
        config.tile_overlap_size_x = overlap_size_x_beats;
        config.last_tile_size_x = last_tile_size_x_beats;
        config.tile_count_x = tile_count_x;

        config.tile_size_y = realized_tile_size_y;
        config.tile_overlap_size_y = overlap_size_y;
        config.effective_tile_size_y = effective_tile_size_y;
        config.tile_count_y = tile_count_y;
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
    printf("|HLS DEBUG_LOG|%s| memconfig tile generated:\n"
               "  range: (%d, %d, %d) --> (%d, %d, %d) (xblocks)\n"
               "  grid_size: (%d, %d, %d) (xblocks)\n"
               "  diff_y: %d, diff_z: %d\n"
               "  tile_count: (%d, %d), total_tile_count: %u\n"
               "  tile_size_x: %d, last_tile_size_x: %d, overlap_x: %d, effective_x: %d\n"
               "  tile_size_y: %d, last_tile_size_y: %d, overlap_y: %d, effective_y: %d\n"
               "  total_xblocks: %u, total_size_bytes: %u\n"
               "  isContinous: %d, start_offset: %u\n", __func__,
        config.start_x, config.start_y, config.start_z,
        config.end_x, config.end_y, config.end_z,
        config.grid_xblocks, config.grid_size_y, config.grid_size_z, diff_y, diff_z,
        tile_count_x, tile_count_y, config.total_tile_count,
        config.tile_size_x, config.last_tile_size_x, config.tile_overlap_size_x, config.effective_tile_size_x,
        config.tile_size_y, config.last_tile_size_y, config.tile_overlap_size_y, config.effective_tile_size_y,
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
