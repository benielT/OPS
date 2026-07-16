#pragma once

 #ifndef DOXYGEN_SHOULD_SKIP_THIS 
/** @file
  * @brief OPS xilinx fpga tapa flow device top include
  * @author Beniel Thileepan
  * @details The top include file handling all the tapa flow related OPS componets. From the application,
  *     including this will enable acceess to all the OPS L1 tapa components.
  */

  #include <ap_int.h>
  #include <tapa.h>
  #include <stdio.h>
  #include "../../common/include/ops_hls_defs.hpp"
  #include "../../common/include/ops_hls_utils.hpp"
  #include "../../common/include/ops_hls_common_memconfig.hpp"
  #include "../../L1/include/ops_tapa_datamover.hpp"
  #include "../../L1/include/ops_hls_stencil_core_v2.hpp"
  #endif