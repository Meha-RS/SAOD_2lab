#include "experiment.h"

double wtime() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}

static void generate_file(const char *filename, int count) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    for (int i = 0; i < count; i++) {
        int len = 6 + rand() % 25; // от 6 до 30
        for (int j = 0; j < len; j++) {
            fputc('a' + rand() % 26, f);
        }
        fputc('\n', f);
    }
    fclose(f);
}

void run_experiment(const char *filename) {
    int total_words = 100000;
    
    printf("Генерация файла %s...\n", filename);
    generate_file(filename, total_words);

    printf("Чтение файла...\n");
    char **words = malloc(total_words * sizeof(char*));
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Ошибка открытия файла\n");
        return;
    }
    
    for (int i = 0; i < total_words; i++) {
        words[i] = malloc(30);
        fscanf(f, "%s", words[i]);
    }
    fclose(f);

    rtree *t = rtree_create();
    int step = 10000;
    int iterations = 1000000;

    printf("\n%-20s %-20s\n", "Количество элементов", "Время lookup (сек)");

    for (int i = 0; i < total_words; i++) {
        rtree_insert(t, words[i], i);
        
        if ((i + 1) % step == 0) {
            int n = i + 1;
            
            double t_start = wtime();
            for (int k = 0; k < iterations; k++) {
                int r = rand() % n;
                rtree_lookup(t, words[r]);
            }
            double t_end = wtime();
            
            double avg_time = (t_end - t_start) / iterations;
            printf("%-20d %.9f\n", n, avg_time);
        }
    }

    for (int i = 0; i < total_words; i++)
        free(words[i]);
    free(words);
    rtree_free(t);
}