;
;   This program checks very simple commands
;
set ra 3            ; ra = 3          -> 3
set rb 6            ; rb = 6          -> 6
add ra ra rb        ; ra = ra + rb    -> 9
add ra ra '0'       ; ra -> ASCII     -> '9'
out ra              ; print ra        -> '9' (on screen)
out '\n'            ; newline
halt                ; end of program   