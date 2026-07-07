#include <errno.h>
#include <error.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TEST_CONDITION(func, config, do_check) test_condition(func, &config, do_check, #func)
#define TEST_CONDITION_WITH_DATA(func, data, do_check) test_condition_with_data(func, data, do_check, #func)
const char *PRINT_CONFIG_INDENT = "    ";

typedef struct {
    int *data;
    size_t size;
} Data;

typedef enum {
    RANDOM,
    PARTIAL_RANDOM,
    REVERSE,
} Mode;

const char *mode_name[] = {
    "RANDOM",
    "PARTIAL_RANDOM",
    "REVERSE",
};

typedef struct {
    Mode mode;
    unsigned int seed;
    size_t data_size;
    double ration;
} Config;

Data alloc_data(size_t size) {
    Data data;
    data.size = size;
    data.data = (int *)malloc(sizeof(int) * size);

    if (data.data == NULL) {
        fprintf(stderr, "error: in alloc_data(), failed to allocate %zu bytes.\n", size);
        fprintf(stderr, "malloc error(%s)", strerror(errno));

        exit(1);
    }

    return data;
}

void free_data(Data data) {
    free(data.data);
}

// 配列用データを生成する関数
Data gen_data(const Config *config) {
    Data data;

    srand(config->seed);
    data = alloc_data(config->data_size);

    switch (config->mode) {
    case RANDOM:
        for (size_t i = 0; i < data.size; i++) {
            data.data[i] = rand();
        }
        break;

    case PARTIAL_RANDOM:
        if (config->ration < 0 || config->ration > 1) {
            fprintf(stderr, "error: in gen_data(), 'config->ration' should be set between 0 and 1.\n");
            exit(1);
        }
        for (size_t i = 0; i < data.size; i++) {
            data.data[i] = i < (size_t)(data.size * config->ration) ? (int)i : rand();
        }
        break;

    case REVERSE:
        for (size_t i = 0; i < data.size; i++) {
            data.data[i] = data.size - i;
        }
        break;

    default:
        fprintf(stderr, "error: in gen_data(), 'config->mode' should be set defined in 'Mode'.\n");
        exit(1);
    }

    return data;
}

double get_elapsed_time() {
    static clock_t time_point = 0;
    double elapsed_time = (double)(clock() - time_point) / CLOCKS_PER_SEC;
    time_point = clock();
    return elapsed_time;
}

void print_config(const Config *config) {
    printf("%smode: %s\n", PRINT_CONFIG_INDENT, mode_name[config->mode]);
    printf("%sseed: %u\n", PRINT_CONFIG_INDENT, config->seed);
    printf("%sdata_size: %zu\n", PRINT_CONFIG_INDENT, config->data_size);
    if (config->mode == PARTIAL_RANDOM) {
        printf("%sration: %lf\n", PRINT_CONFIG_INDENT, config->ration);
    }
}

static inline void swap(int *lhs, int *rhs) {
    int temp = *lhs;
    *lhs = *rhs;
    *rhs = temp;
}

void print_data(Data data) {
    if (data.size == 0) {
        return;
    }

    for (size_t i = 0; i < data.size - 1; i++) {
        printf("%d ", data.data[i]);
    }
    printf("%d\n", data.data[data.size - 1]);
}

void new_line() {
    putchar('\n');
}

typedef void (*SortAlgorithm)(Data);

void test_condition(SortAlgorithm func, const Config *config, bool do_check, const char *algorithm_name) {
    puts("generating test case...");
    Data data = gen_data(config);
    puts("test case is generated");
    print_config(config);
    puts("");

    printf("start sorting with: %s\n", algorithm_name);
    puts("sorting...");
    get_elapsed_time();
    func(data);
    double time = get_elapsed_time();
    puts("finished sorting!");
    printf("\ttime: %.8lf\n", time);
    puts("");

    if (do_check) {
        puts("checking...");
        bool flag = true;
        for (size_t i = 1; i < data.size; i++) {
            if (data.data[i - 1] > data.data[i]) {
                flag = false;
                break;
            }
        }

        if (flag) {
            puts("succeed!");
        }
        else {
            puts("failed to sort");
        }
    }

    free_data(data);
}

int test_condition_with_data(SortAlgorithm func, Data data, bool do_check, const char *algorithm_name) {
    printf("start sorting with: %s\n", algorithm_name);
    puts("sorting...");
    get_elapsed_time();
    func(data);
    double time = get_elapsed_time();
    puts("finished sorting!");
    printf("\ttime(ms): %d\n", (int)(time * 1000));
    puts("");

    if (do_check) {
        puts("checking...");
        bool flag = true;
        for (size_t i = 1; i < data.size; i++) {
            if (data.data[i - 1] > data.data[i]) {
                flag = false;
                break;
            }
        }

        if (flag) {
            puts("succeed!");
        }
        else {
            puts("failed to sort");
        }
    }

    puts("");

    return (int)(time * 1000);
}

void selection_sort(Data data) {
    int min;
    for (size_t i = 0; i < data.size; i++) {
        min = i;
        for (size_t j = i + 1; j < data.size; j++) {
            if (data.data[min] > data.data[j]) {
                min = j;
            }
        }
        swap(data.data + i, data.data + min);
    }
}

void insertion_sort(Data data) {
    for (size_t manipulating_pos = 1; manipulating_pos < data.size; manipulating_pos++) {
        size_t i;
        for (i = 0; i < manipulating_pos; i++) {
            if (data.data[i] > data.data[manipulating_pos]) {
                break;
            }
        }

        for (; i < manipulating_pos; i++) {
            swap(data.data + i, data.data + manipulating_pos);
        }
    }
}

void bubble_sort(Data data) {
    for (size_t i = 1; i < data.size; i++) {
        for (size_t j = 0; j < data.size - i; j++) {
            if (data.data[j] > data.data[j + 1]) {
                swap(data.data + j, data.data + j + 1);
            }
        }
    }
}

void quick_sort(Data data) {
    size_t left_accessor = 0;
    size_t right_accessor = data.size - 1;

    if (data.size == 2) {
        if (data.data[0] > data.data[1]) {
            swap(data.data, data.data + 1);
        }
        return;
    }
    else if (data.size <= 1) {
        return;
    }

    int pivot = data.data[data.size / 2];
    while (true) {
        while (right_accessor > 0 && data.data[right_accessor] > pivot) {
            right_accessor--;
        }
        while (left_accessor < data.size && data.data[left_accessor] < pivot) {
            left_accessor++;
        }

        if (left_accessor >= right_accessor) {
            Data left_data;
            left_data.data = data.data;
            left_data.size = right_accessor + 1;

            Data right_data;
            right_data.data = data.data + right_accessor + 1;
            right_data.size = data.size - right_accessor - 1;

            quick_sort(left_data);
            quick_sort(right_data);

            break;
        }

        swap(data.data + left_accessor, data.data + right_accessor);
        left_accessor++;
        right_accessor--;
    }
}

void merge_sort(Data data) {
}

int main() {
    Config config;
    config.data_size = 1e8;
    config.seed = 1;
    config.mode = RANDOM;

    puts("generating test case...");
    Data data = gen_data(&config);
    puts("test case is generated");
    print_config(&config);
    new_line();

    // TEST_CONDITION_WITH_DATA(selection_sort, data, true);
    // TEST_CONDITION_WITH_DATA(insertion_sort, data, true);
    // TEST_CONDITION_WITH_DATA(bubble_sort, data, true);
    TEST_CONDITION_WITH_DATA(quick_sort, data, true);

    free_data(data);

    return 0;
}
