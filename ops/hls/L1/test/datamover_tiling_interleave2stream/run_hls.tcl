source settings.tcl

set DUT_PROJECT "dut.prj"
set SOLUTION "solution1"

if {![info exists CLKP]} {
  set CLKP 3.30
}

# Pull in dynamically generated flags from the bash environment
set USER_CFLAGS ""
if {[info exists ::env(USER_CFLAGS)]} {
    set USER_CFLAGS $::env(USER_CFLAGS)
}

open_project -reset $DUT_PROJECT

# Inject USER_CFLAGS into the compiler
add_files "top.cpp" -cflags "-I${PROJ_ROOT}/L1/include $USER_CFLAGS"
add_files -tb "main.cpp" -cflags "-I${PROJ_ROOT}/L1/include $USER_CFLAGS"
set_top dut

open_solution -reset $SOLUTION

set_part $XPART
create_clock -period $CLKP

if {$CSIM == 1} {
  csim_design
}

if {$CSYNTH == 1} {
  csynth_design
}

if {$COSIM == 1} {
  cosim_design
}

if {$VIVADO_SYN == 1} {
  export_design -flow syn -rtl verilog
}

if {$VIVADO_IMPL == 1} {
  export_design -flow impl -rtl verilog
}

exit