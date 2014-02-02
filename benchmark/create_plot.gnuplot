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
plot [0:PLOT_NUM_ELEMENTS] for [i=2:COUNT] "results/overall.csv" using 1:i with lines

