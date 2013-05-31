
set terminal png size 800,800 enhanced font "Helvetica,20"
set output 'output.png'
plot "hist.dat" using 2 title "Y" with lines, \
     "hist.dat" using 3 title "Y'" with lines
