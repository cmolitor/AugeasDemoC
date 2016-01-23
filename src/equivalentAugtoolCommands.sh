#!/usr/bin/augtool --noautoload -f
augtool --noautoload<<-EOF

# Set lenses
set /augeas/load/Interfaces/lens "Interfaces.lns"

# Set path to config file
set /augeas/load/Interfaces/incl "/home/pi/AugeasDemoC/network/interfaces"

# load
load

print /augeas/load

# Check for errors
print /augeas//error

# Change node value; here: method = dhcp
set /files//home/pi/AugeasDemoC/network/interfaces/iface[3]/method dhcp

print /home/pi/AugeasDemoC/network/interfaces

# Save changes
save

# Check for errors
print /augeas//error

# Print saved files
print /augeas/events/saved
EOF