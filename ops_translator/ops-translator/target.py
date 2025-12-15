from __future__ import annotations

from typing import Any, Dict, Tuple
from util import Findable
from enum import Enum
from store import Application, CodegenError, CodeGenWarning

#TODO: Add documentaion (numpy style)
class Target(Findable):
    name: str
    kernel_translation: bool
    config: Dict[str, Any]
    # contrains will be key-type-contrains if type is "numeric" contrains will be (lower, upper) tuple
        # if lower or upper is None it means unbounded on that side
    # if type is "select" contrains will be a set of allowed values
    # if type is "bool" contrains will be (True, False)
    # if type is "illegal" contrains will be a set of illegal values
    
    __config_contrains__: Dict[str, Tuple[str,Any]] = {} 
    __config_verified__: bool = False

    def __str__(self) -> str:
        return f"{self.name}: config: {self.config}"
    
    def __eq__(self, other) -> bool:
        return self.name == other.name if type(other) is type(self) else False

    def __hash__(self) -> int:
        return hash(self.name)

    def matches(self, key: str) -> bool:
        return self.name == key.lower()
    
    def verify_config(self, app: Application) -> None:
        if self.__config_verified__:
            return
        for key, (constraint_type, constraint_value) in self.__config_contrains__.items():
            if key in self.config:
                value = self.config[key]
                if constraint_type == "numeric":
                    lower, upper = constraint_value
                    if (lower is not None and value < lower) or (upper is not None and value > upper):
                        raise CodegenError(f"Target {self.name} config error: {key}={value} not in range [{lower}, {upper}]")
                elif constraint_type == "select":
                    if value not in constraint_value:
                        raise CodegenError(f"Target {self.name} config error: {key}={value} not in allowed values {constraint_value}")
                elif constraint_type == "bool":
                    if value not in constraint_value:
                        raise CodegenError(f"Target {self.name} config error: {key}={value} must be boolean")
                elif constraint_type == "illegal":
                    if value in constraint_value:
                        raise CodegenError(f"Target {self.name} config error: {key}={value} is an illegal value")
        self.__config_verified__ = True
        # Each target can override this method to add more complex verification if needed

class MPIOpenMP(Target):
    name = "mpi_openmp"
    suffix = "seq"
    kernel_translation = False
    config = {
        "grouped" : False, 
        "device" : 1
        }

class F2CMPIOpenMP(Target):
    name = "f2c_mpi_openmp"
    suffix = "f2c"
    kernel_translation = False
    config = {
        "grouped" : False,
        "device" : 2
        }

class Cuda(Target):
    name = "cuda"
    suffix = "cuda"
    kernel_translation = True
    config = {
        "grouped" : True,
        "device" : 3,
        "atomics": True,
        "color2": False
        }

class F2CCuda(Target):
    name = "f2c_cuda"
    suffix = "f2c"
    kernel_translation = True
    config = {
        "grouped" : True,
        "device" : 4,
        "atomics": True,
        "color2": False
        }

class Hip(Target):
    name = "hip"
    kernel_translation = True
    config = {
        "grouped" : True,
        "device" : 5,
        "atomics": True,
        "color2": False
        }

class F2CHip(Target):
    name = "f2c_hip"
    suffix = "f2c"
    kernel_translation = True
    config = {
        "grouped" : True,
        "device" : 6,
        "atomics": True,
        "color2": False
        }

class OpenMPOffload(Target):
    name = "openmp_offload"
    kernel_translation = True
    suffix = "ompoffload"
    config = {
        "grouped" : True,
        "device" : 7,
        "atomics": True,
        "color2": False
        }

#class OpenACC(Target):
#    name = "openacc"
#    kernel_translation = True
#    config = {
#        "grouped" : True,
#        "device" : 5,
#        "atomics": True,
#        "color2": False
#        }

class Sycl(Target):
    name = "sycl"
    kernel_translation = True
    config = {
        "grouped" : True,
        "device" : 8,
        "atomics": True,
        "color2": False
        }

class FpgaDatamoverMode(Enum):
    DATAMOVER_LOOPBACK = 1
    DATAMOVER_DATACOPY = 2
    DATAMOVER_HYBRID = 3
class F2CSycl(Target):
    name = "f2c_sycl"
    suffix = "f2c"
    kernel_translation = True
    config = {
        "grouped" : True,
        "device" : 9,
        "atomics": True,
        "color2": False
        }

class HLS(Target):
    name = "hls"
    kernel_translation = True
    config = {
        "grouped" : False,
        "SLR_count" : 1,
        "max_SLR_count" : 3,
        "device_id" : 0,
        "vector_factor" : 8,
        "mem_vector_factor": 16,
        "iter_par_factor": 20,
        "stencil_type" : "float",
        "data_width" : 32,
        "mem_data_width" : 32,
        "maxi_depth" : 4096,
        "maxi_read_burst_length" : 64,
        "maxi_write_burst_length" : 64,
        "num_read_outstanding" : 4,
        "num_write_outstanding" : 4,
        "maxi_offset" : "slave",
        "ops_max_dim" : 3,
        "axis_interconnect_buff_size" : 2048,
        "hls_interconnect_buff_size" : 10,
        "datamover_mode" : 1,
        "profile" : False,
        "platform" : "",
        "platform_is_multi_slr" : True,
        "platform_is_sb_selectable" : True,
        "platform_is_ib_selectable" : False,
        "supported_internal_storage" : []
        }
    platforms = {
        "u280" : {
            "SLR_count" : 3,
            "max_SLR_count" : 3,
            "platform_is_multi_slr" : True,
            "platform_is_sb_selectable" : True,
            "platform_is_ib_selectable" : True,
            "supported_internal_storage" : ["URAM",  "BRAM"]
        },
        "u55c" : {
            "SLR_count" : 3,
            "max_SLR_count" : 3,
            "platform_is_multi_slr" : True,
            "platform_is_sb_selectable" : True,
            "platform_is_ib_selectable" : True,
            "supported_internal_storage" : ["URAM",  "BRAM"]
        },
        "vck5000" : {
            "SLR_count" : 1,
            "max_SLR_count" : 1,
            "platform_is_multi_slr" : False,
            "platform_is_sb_selectable" : False
        }
    }
    __config_contrains__ = {
        "max_SLR_count" : ("numeric", (1, 3)),
        "SLR_count" : ("numeric", (1, config["max_SLR_count"])),
        "vector_factor" : ("numeric", (1, None)),
        "mem_vector_factor": ("numeric", (1, None)),
        "iter_par_factor": ("numeric", (1, None)),
        "data_width" : ("select", {8,16,32,64}),
        "mem_data_width" : ("select", {8,16,32,64}),
        "maxi_depth" : ("numeric", (1, None)),
        "maxi_read_burst_length" : ("numeric", (1, 256)),
        "maxi_write_burst_length" : ("numeric", (1, 256)),
        "num_read_outstanding" : ("numeric", (1, 16)),
        "num_write_outstanding" : ("numeric", (1, 16)),
        "maxi_offset" : ("select", {"master","slave"}),
        "ops_max_dim" : ("numeric", {None,3}),
        "datamover_mode" : ("select", {1,2,3}),
        "profile" : ("bool", (True, False))
    }
    def verify_config(self, app: Application) -> None:
        super().verify_config(app)
        # if App is tiled then datamover mode cannot be loopback
        is_tiled = any(program.tiling for program in app.programs)
        if is_tiled and self.config["datamover_mode"] != FpgaDatamoverMode.DATAMOVER_DATACOPY.value:
            CodeGenWarning(f"Target {self.name} config warning: Application is tiled, changing datamover_mode to DATAMOVER_DATACOPY")
            self.config["datamover_mode"] = FpgaDatamoverMode.DATAMOVER_DATACOPY.value
        # Check platform specific constraints
        
        platform = self.config.get("platform", "")
        if platform in self.platforms:
            platform_info = self.platforms[platform]
            # Check SLR_count
            max_slr = platform_info.get("max_SLR_count", self.config["max_SLR_count"])
            if self.config["SLR_count"] > max_slr:
                raise CodegenError(f"Target {self.name} config error: SLR_count={self.config['SLR_count']} exceeds platform {platform} max_SLR_count={max_slr}")
            # Check internal storage
            supported_storage = platform_info.get("supported_internal_storage", [])
            internal_storage = self.config.get("internal_storage", None)
            if internal_storage and internal_storage not in supported_storage:
                raise CodegenError(f"Target {self.name} config error: internal_storage={internal_storage} not supported on platform {platform}. Supported: {supported_storage}") 
            

            

Target.register(MPIOpenMP)
Target.register(F2CMPIOpenMP)
Target.register(Cuda)
Target.register(F2CCuda)
Target.register(Hip)
Target.register(F2CHip)
Target.register(OpenMPOffload)
#Target.register(OpenACC)
Target.register(Sycl)
Target.register(F2CSycl)
Target.register(HLS)
