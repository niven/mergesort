set terminal svg size 600,400 dynamic enhanced fname 'arial'  fsize 10 name "simple_1" butt solid 
set output OUTPUT_FILE
set key outside right top vertical Right noreverse enhanced autotitles box linetype -1 linewidth 1.000
set samples 50, 50
set title PLOT_TITLE 
set title  offset character 0, 0, 0 font ",20" norotate
set xlabel "Elements"
set ylabel "Ticks"
set datafile separator ","
set key autotitle columnhead
set linetype  1 lc rgb "dark-violet" lw 1
set linetype  2 lc rgb "#009e73" lw 1
set linetype  3 lc rgb "#56b4e9" lw 1
set linetype  4 lc rgb "#e69f00" lw 1
set linetype  5 lc rgb "#f0e442" lw 1
set linetype  6 lc rgb "#0072b2" lw 1
set linetype  7 lc rgb "#e51e10" lw 1
 
plot [0:PLOT_NUM_ELEMENTS] for [i=2:COUNT] "results/overall.csv" using 1:i with lines

