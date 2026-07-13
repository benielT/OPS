puts "========================================================="
puts "EXECUTING TAPA CLOCK FIX HOOK..."

# Use -regexp to match the full path instead of just the pin name
set tapa_pins [get_bd_intf_pins -quiet -hierarchical -regexp {.*(kernel_outerloop|datamover_outerloop).*/arg.*axis.*}]

if {[llength $tapa_pins] == 0} {
    puts "WARNING: No TAPA stream pins found! Clock fix bypassed."
} else {
    foreach pin $tapa_pins {
        puts "Forcing 300MHz on AXI-Stream interface: $pin"
        set_property CONFIG.FREQ_HZ 300000000 $pin
    }
}

# Also ensure the main AXI control/memory interfaces aren't stuck at 100MHz
set axi_pins [get_bd_intf_pins -quiet -hierarchical -regexp {.*(kernel_outerloop|datamover_outerloop).*/[sm]_axi.*}]
foreach pin $axi_pins {
    puts "Forcing 300MHz on AXI/Control interface: $pin"
    set_property CONFIG.FREQ_HZ 300000000 $pin
}

puts "========================================================="