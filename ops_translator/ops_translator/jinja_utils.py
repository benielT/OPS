import os
from math import ceil, log2, floor

from jinja2 import Environment, FileSystemLoader
import re
import ops
import logging
from typing import Union, List

env = Environment(
    loader=FileSystemLoader(os.path.join(os.path.dirname(__file__), "../resources/templates")),
    lstrip_blocks=True,
    trim_blocks=True
)

# env.tests["soa"] = lambda dat, loop=None: dat.soa
# env.tests["opt"]

env.tests["ops_dat"]    = lambda arg, loop=None: isinstance(arg, ops.ArgDat)
env.tests["ops_gbl"]    = lambda arg, loop=None: isinstance(arg, ops.ArgGbl)
env.tests["ops_reduce"] = lambda arg, loop=None: isinstance(arg, ops.ArgReduce)
env.tests["ops_idx"]    = lambda arg, loop=None: isinstance(arg, ops.ArgIdx)

env.tests["ops_read"]  = lambda arg, loop=None: hasattr(arg, "access_type") and arg.access_type == ops.AccessType.OPS_READ
env.tests["ops_write"] = lambda arg, loop=None: hasattr(arg, "access_type") and arg.access_type == ops.AccessType.OPS_WRITE
env.tests["ops_rw"]    = lambda arg, loop=None: hasattr(arg, "access_type") and arg.access_type == ops.AccessType.OPS_RW
env.tests["read_only_rw"] = lambda arg, loop=None: hasattr(arg, "access_type") and arg.access_type == ops.AccessType.OPS_RW \
    and hasattr(arg, "is_read_only") and arg.is_read_only
env.tests["ops_read_or_rw"]  = lambda arg, loop=None: hasattr(arg, "access_type") and \
    (arg.access_type == ops.AccessType.OPS_READ or arg.access_type == ops.AccessType.OPS_RW)
env.tests["ops_write_or_rw"]  = lambda arg, loop=None: hasattr(arg, "access_type") and \
    (arg.access_type == ops.AccessType.OPS_WRITE or arg.access_type == ops.AccessType.OPS_RW)
env.tests["ops_not_idx"] = lambda arg, loop=None: not isinstance(arg, ops.ArgIdx)
env.tests["ops_inc"] = lambda arg, loop=None: hasattr(arg, "access_type") and arg.access_type == ops.AccessType.OPS_INC
env.tests["ops_min"] = lambda arg, loop=None: hasattr(arg, "access_type") and arg.access_type == ops.AccessType.OPS_MIN
env.tests["ops_max"] = lambda arg, loop=None: hasattr(arg, "access_type") and arg.access_type == ops.AccessType.OPS_MAX

def isArgUseStencil(arg: ops.ArgDat, stencil_ptr: str) -> bool:
    if not hasattr(arg, "stencil_ptr"):
        return False
    if arg.stencil_ptr == stencil_ptr:
        return True
    return False
env.tests["ops_arg_use_stencil"] = isArgUseStencil

env.tests["point"] = lambda point, loop=None: isinstance(point, ops.Point)
env.tests["window_buffer"] = lambda buff, loop=None: isinstance(buff, ops.WindowBuffer)

env.tests["read_or_write"] = lambda arg, loop=None: hasattr(arg, "access_type") and arg.access_type in [
    ops.AccessType.OPS_READ,
    ops.AccessType.OPS_WRITE,
    ops.AccessType.OPS_RW
]

env.tests["reduction"] = lambda arg, loop=None: hasattr(arg, "access_type") and arg.access_type in [
    ops.AccessType.OPS_INC,
    ops.AccessType.OPS_MIN,
    ops.AccessType.OPS_MAX
]
    
env.tests["fortran_real_type"]  = lambda arg, loop=None: hasattr(arg, "typ") and ("real" in str(arg.typ).lower())
env.tests["fortran_integer_type"]  = lambda arg, loop=None: hasattr(arg, "typ") and ("integer" in str(arg.typ).lower())

def read_in(dat: ops.Dat, loop: ops.Loop) -> bool:
    isReadin = False
    for arg in loop.args:
        if not isinstance(arg, ops.ArgDat):
            continue

        if arg.dat_id == dat.id and arg.access_type in [ops.AccessType.OPS_READ, ops.AccessType.OPS_RW]:
            isReadin = True
    
    if isReadin:
        return True

    return False

env.tests["read_in"] = read_in
env.tests["instance"] = lambda x, c: isinstance(x, c)
env.tests["isnumaric"] = lambda arg, loop=None: isinstance(arg, str) and arg.isnumeric()

def isArgSwap(arg: ops.ArgDat, iterloop: ops.IterLoop) -> bool:
    pair = iterloop.getDatSwapPair(iterloop.dats[arg.dat_id][0].ptr)
    if pair[0] != pair[1]:
        return True
    else:
        return False
    
env.tests["is_arg_swap"] = isArgSwap

env.globals.update(shift_bits = lambda widen, base_size: int(log2(widen+1) - log2(base_size)))

def getReadArgFromDat(dat: ops.Dat, loop: ops.Loop) -> ops.ArgDat:
    # print(f"dat:  {dat}, dat.name: {dat.ptr}, dat_id: {dat.id}, loop: {loop}\n")
    for arg in loop.args:
        if not isinstance(arg, ops.ArgDat):
            continue
        if arg.dat_id  == dat.id and arg.access_type in [ops.AccessType.OPS_READ, ops.AccessType.OPS_RW]:
            # print(f"Arg found from dat {dat.ptr}: {arg}")
            return arg
    logging.warning(f"Couldn't find dat:{dat} in loop: {loop}")
    return None

def getWriteArgFromDat(dat: ops.Dat, loop: ops.Loop) -> ops.ArgDat:
    for arg in loop.args:
        if not isinstance(arg, ops.ArgDat):
            continue
        if arg.dat_id  == dat.id and arg.access_type in [ops.AccessType.OPS_WRITE, ops.AccessType.OPS_RW]:
            return arg
    return None

def getArgGblName(arg: ops.ArgGbl):
    return re.sub(r'\W+', '', arg.ptr)

def getReadArgsFromStencil(stencil_ptr: str, loop: ops.Loop) :
    read_args = []
    for arg in loop.args:
        if not isinstance(arg, ops.ArgDat):
            continue
        if isArgUseStencil(arg, stencil_ptr) and arg.access_type in [ops.AccessType.OPS_READ, ops.AccessType.OPS_RW]:
            read_args.append(arg)
    return read_args

def getAdjustedLineBufferSize(buf_size: int, half_span: int, vec_fac: int, div: int = 1) -> int :
    adjusted_buf_size = int(ceil(buf_size / div))
    return ((int)((adjusted_buf_size + 2*half_span + vec_fac - 1)/vec_fac))

def getAdjustedPlaneBufferSize(x_size: int, y_size: int, half_span: int, vec_fac: int, div: int = 1) -> int:
    # print(f"x_size: {x_size}, y_size: {y_size}, half_span: {half_span}, vec_fac: {vec_fac}\n")
    adjusted_x_size = int(ceil(x_size/div))
    adj_x_size = (int)((adjusted_x_size + 2*half_span + vec_fac - 1)/vec_fac)
    adj_y_size = y_size + 2*half_span
    return (adj_x_size * adj_y_size)

# def getOverlapTileSize(n_slr: int, p_slr: int, half_span: int, mem_vec_fac: int) -> int:
#     # print(f"n_slr: {n_slr}, p_slr: {p_slr}, half_span: {half_span}, mem_vec_fac: {mem_vec_fac}")
#     if isinstance(p_slr, list):
#         return (floor(((sum(p_slr)) * half_span + mem_vec_fac - 1) / mem_vec_fac) * mem_vec_fac * 2)
#     return (floor(((n_slr * p_slr) * half_span + mem_vec_fac - 1) / mem_vec_fac) * mem_vec_fac * 2)


def getTotalPEs(n_slr: int, p_slr: Union[int, List]) -> int:
    if isinstance(p_slr, list):
        return sum(p_slr)
    return (n_slr * p_slr)

env.globals.update(get_read_arg_from_dat = lambda dat, loop: getReadArgFromDat(dat, loop))
env.globals.update(get_write_arg_from_dat = lambda dat, loop: getWriteArgFromDat(dat, loop))
env.globals.update(get_arg_gbl_name = lambda gbl: getArgGblName(gbl))
env.globals.update(get_read_args_from_stencil = lambda stencil_ptr, loop: getReadArgsFromStencil(stencil_ptr, loop))
env.globals.update(get_line_buff_size = lambda x_size, half_span, vec_fac, div = 1: getAdjustedLineBufferSize(x_size, half_span, vec_fac, div))
env.globals.update(get_plane_buff_size = lambda x_size, y_size, half_span, vec_fac, div = 1: getAdjustedPlaneBufferSize(x_size, y_size, half_span, vec_fac, div))
# env.globals.update(get_overlap_tile_size = lambda n_slr, p_slr, half_span, mem_vec_fac: getOverlapTileSize(n_slr, p_slr, half_span, mem_vec_fac))
env.globals.update(get_total_PEs = lambda n_slr, p_slr: getTotalPEs(n_slr, p_slr))
env.globals.update(log2 = lambda arg: log2(arg))
env.globals.update(max = lambda arg1, arg2: max(arg1,arg2))
env.globals.update(islist = lambda arg: isinstance(arg, list))

def unpack(tup):
    if not isinstance(tup, tuple):
        return tup
    return tup[0]

def test_to_filter(filter_, key=unpack):
    return lambda xs, loop=None: list(filter(lambda x: env.tests[filter_](key(x), loop), xs))
    
env.filters["ops_dat"] = test_to_filter("ops_dat")
env.filters["ops_gbl"] = test_to_filter("ops_gbl")
env.filters["ops_reduce"] = test_to_filter("ops_reduce")
env.filters["ops_idx"] = test_to_filter("ops_idx")
env.filters["ops_not_idx"] = test_to_filter("ops_not_idx")

env.filters["ops_read"]  = test_to_filter("ops_read")
env.filters["ops_write"] = test_to_filter("ops_write")
env.filters["ops_rw"]    = test_to_filter("ops_rw")
env.filters["read_only_rw"] = test_to_filter("read_only_rw")
env.filters["ops_read_or_rw"] = test_to_filter("ops_read_or_rw")
env.filters["ops_write_or_rw"] = test_to_filter("ops_write_or_rw")

env.filters["ops_inc"] = test_to_filter("inc")
env.filters["ops_min"] = test_to_filter("min")
env.filters["ops_max"] = test_to_filter("max")

env.filters["read_or_write"] = test_to_filter("read_or_write")
env.filters["reduction"] = test_to_filter("reduction")
env.filters["fortran_real_type"] = test_to_filter("fortran_real_type")
env.filters["fortran_integer_type"] = test_to_filter("fortran_integer_type")

env.filters["index"] = lambda xs, x: xs.index(x)

env.filters["round_up"] = lambda x, b: b * ceil(x / b)

env.filters["max_value"] = lambda values: max(values)
