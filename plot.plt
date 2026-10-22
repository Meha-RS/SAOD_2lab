# Настройки терминала (сохранит в PNG)
set terminal png size 800,500 font "Times New Roman, 14"
set output "lookup_plot.png"

# Стили линий (синяя линия с точками)
set style line 1 lc rgb "blue" lt 1 lw 2 pt 7 ps 1.5

# Настройки сетки и осей
set grid
set border lw 2
set key top left

# Подписи осей (как требуют в README)
set xlabel "Количество элементов в дереве" font "Times New Roman, 16"
set ylabel "Время выполнения lookup, мкс" rotate by 90 font "Times New Roman, 16"

# Настройки делений
set xtics 10000 rotate by 45 right
set mxtics 2
set format x "%6.0f"
set format y "%.2f"

# Сама отрисовка. 
# ВАЖНО: ($2 * 1000000) берет вторую колонку из файла и умножает на миллион!
plot "plot_data.txt" using 1:($2 * 1000000) title "Radix Tree lookup" with linespoints ls 1