



Left-1cm
$J=G21G91X-1F500

Right 1cm
$J=G21G91X1F500

Up .1cm
$J=G21G91Z0.1F500

Down .1cm
$J=G21G91Z-0.1F500


Up 1cm
$J=G21G91Z1F500

Down 1cm
$J=G21G91Z-1F500

Forward 1cm
$J=G21G91Y11F500

Back 1cm
$J=G21G91X-1F500

RETURN TO ZERO
F300.00
G01 Z 22
G01 X 40
G01 Y 20
G01 X 0
G01 Y 0
F120.00
G01 Z 0

RESET Z`
G10 P0 L20 Z0

RESET X 
G10 P0 L20 X0

RESET Y 
G10 P0 L20 Y0

RESET Z 
G10 P0 L20 Y0

Plate Prepare Autolevel
F300.00
G01 Z 22
G01 Y 10
G01 X 40
G01 Y -40

Spindle On
m3 s3000

Spidnel OFF
m3 s0



CutVertLine

m3 s3000
F80.00

G01 Z -.3
G01 Y 88
G01 Z -.7
G01 Y 0

G01 Z -1.0
G01 Y 88
G01 Z -1.4
G01 Y 0

m3 s0


Set Manualy Cordinates
X 
 G10 P0 L20 X26
 
Y 
G10 P0 L20 Y5

Z 
G10 P0 L20 Z0