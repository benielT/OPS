#pragma once

namespace ops
{
namespace hls
{

/** @brief Which physical QSFP port faces which logical neighbour.
 *  Matches AuroraFlow's reference ring wiring: r{R}_i1 <-> r{R+1}_i0.
 */
enum class LinkDirection : unsigned { LOWER = 0, UPPER = 1 };

/** @brief Per-run link health snapshot. */
struct LinkStats
{
    unsigned long frames_with_errors = 0;
    unsigned long fifo_rx_overflows  = 0;
    bool          channel_up         = false;
};

} // namespace hls
} // namespace ops