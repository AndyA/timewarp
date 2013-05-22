
set terminal png size 10000,400 enhanced font "Helvetica,20"
set output 'output.png'
plot "plotpipe.dat" using 2 title 'Y' with lines,  \
     "plotpipe.dat" using 3 title 'Cb' with lines, \
     "plotpipe.dat" using 4 title 'Cr' with lines
