#include <errno.h>
#include <error.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TEST_CONDITION(func, config, do_check, show_data) test_condition(func, &config, do_check, show_data, #func)
#define TEST_CONDITION_WITH_DATA(func, data, do_check, show_data) test_condition_with_data(func, data, do_check, show_data, #func)

#define MIN(lhs, rhs) ((lhs) < (rhs) ? (lhs) : (rhs))
const char *PRINT_CONFIG_INDENT = "    ";
#define NEW_LINE_COUNT 10
#define ALGORITHM_COUNT 2
#define AVERAGE_COUNT 10

typedef struct {
    int *data;
    size_t size;
} Data;

typedef enum {
    RANDOM,
    PARTIAL_RANDOM,
    REVERSE,
    INPUT,
} Mode;

const char *mode_name[] = {
    "RANDOM",
    "PARTIAL_RANDOM",
    "REVERSE",
    "INPUT",
};

typedef struct {
    Mode mode;
    unsigned int seed;
    size_t data_size;
    double ration;
} Config;

Data alloc_data(const size_t size) {
    Data data;
    data.size = size;
    data.data = (int *)malloc(sizeof(int) * size);

    if (data.data == NULL) {
        fprintf(stderr, "error: in alloc_data(), failed to allocate %zu bytes.\n", sizeof(int) * size);
        fprintf(stderr, "malloc error(%s)", strerror(errno));

        exit(1);
    }

    return data;
}

void free_data(Data data) {
    free(data.data);
    data.size = 0;
    data.data = NULL;
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

    case INPUT:
        printf("input data\n>> ");
        for (size_t i = 0; i < config->data_size; i++) {
            scanf("%d", data.data + i);
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
    if (config->mode != INPUT) {
        printf("%sseed: %u\n", PRINT_CONFIG_INDENT, config->seed);
    }
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

int test_condition_with_data(SortAlgorithm func, Data data, bool do_check, bool show_data, const char *algorithm_name) {
    if (show_data) {
        puts("data before sorted");
        print_data(data);
    }
    puts("copying data...");
    Data data_copy = alloc_data(data.size);
    memcpy(data_copy.data, data.data, sizeof(data.data[0]) * data.size);
    puts("copied!");
    printf("start sorting with: %s\n", algorithm_name);
    puts("sorting...");
    get_elapsed_time();
    func(data_copy);
    double time = get_elapsed_time();
    puts("finished sorting!");
    printf("\ttime(ms): %d\n", (int)(time * 1000));
    new_line();

    if (show_data) {
        puts("data after sorted");
        print_data(data_copy);
    }

    if (do_check) {
        puts("checking...");
        bool flag = true;
        for (size_t i = 1; i < data_copy.size; i++) {
            if (data_copy.data[i - 1] > data_copy.data[i]) {
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

    new_line();

    free_data(data_copy);

    return (int)(time * 1000);
}

int test_condition(SortAlgorithm func, const Config *config, bool do_check, bool show_data, const char *algorithm_name) {
    puts("generating test case...");
    Data data = gen_data(config);
    puts("test case is generated");
    print_config(config);
    new_line();

    int value = test_condition_with_data(func, data, do_check, show_data, algorithm_name);
    free_data(data);
    return value;
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
    if (data.size == 1) {
        return;
    }
    else if (data.size == 2) {
        if (data.data[0] > data.data[1]) {
            swap(data.data, data.data + 1);
        }
        return;
    }

    size_t mid = data.size / 2;

    Data left_data;
    left_data.data = data.data;
    left_data.size = mid;

    Data right_data;
    right_data.data = data.data + mid;
    right_data.size = data.size - mid;

    merge_sort(left_data);
    merge_sort(right_data);

    size_t left_accessor = 0;
    size_t right_accessor = mid;

    Data work_space = alloc_data(data.size);
    size_t work_space_accessor = 0;

    while (true) {
        bool left_fin = left_accessor == mid;
        bool right_fin = right_accessor == data.size;

        if (left_fin && right_fin) {
            break;
        }
        else if (left_fin) {
            while (right_accessor != data.size) {
                work_space.data[work_space_accessor++] = data.data[right_accessor++];
            }
            break;
        }
        else if (right_fin) {
            while (left_accessor != mid) {
                work_space.data[work_space_accessor++] = data.data[left_accessor++];
            }
            break;
        }
        else {
            work_space.data[work_space_accessor++] =
                data.data[left_accessor] < data.data[right_accessor] ? data.data[left_accessor++] : data.data[right_accessor++];
        }
    }

    memcpy(data.data, work_space.data, data.size * sizeof(int));
    free_data(work_space);
}

void write_csv(const char *file_name, size_t time_data_count, size_t *size_data, Data time_data) {
    FILE *fp = fopen(file_name, "w");
    if (fp == NULL) {
        fprintf(stderr, "error: in write_csv(), failed to open file(file: %s).\n", file_name);
        fprintf(stderr, "fopen error(%s)", strerror(errno));

        exit(1);
    }

    for (size_t i = 0; i < time_data_count; i++) {
        fprintf(fp, "%zu,%d\n", size_data[i], time_data.data[i]);
    }

    fclose(fp);
}

void input_config(Config *config) {
    printf("input data size: ");
    scanf("%zu", &config->data_size);

    int mode_input;
    do {
        printf("input mode(0: RANDOM, 1: PARTIAL_RANDOM, 2: REVERSE, 3: INPUT): ");
        scanf("%d", &mode_input);
    } while (mode_input < 0 || 3 < mode_input);
    config->mode = (Mode)mode_input;

    if (config->mode != INPUT) {
        printf("input seed: ");
        scanf("%u", &config->seed);
    }
}

void average_tester() {
    Data data[AVERAGE_COUNT];
    Config config;

    input_config(&config);

    for (size_t i = 0; i < AVERAGE_COUNT; i++) {
        config.seed++;

        puts("generating test case...");
        data[i] = gen_data(&config);
        puts("test case is generated");
        print_config(&config);
        new_line();
    }

    Data time_data[ALGORITHM_COUNT];

    size_t time_data_count = 0;
    for (size_t size = 1; size < config.data_size; size *= 2) {
        time_data_count++;
    }

    for (size_t i = 0; i < ALGORITHM_COUNT; i++) {
        time_data[i] = alloc_data(time_data_count);
    }

    size_t *size_data = malloc(time_data_count * sizeof(size_t));
    if (size_data == NULL) {
        fprintf(stderr, "error: in main(), failed to allocate %zu bytes.\n", sizeof(size_t) * time_data_count);
        fprintf(stderr, "malloc error(%s)", strerror(errno));

        exit(1);
    }

    size_t time_data_counter = 0;
    long long time_data_accessor = 0;
    for (size_t size = 1; size < config.data_size; size *= 2) {

        size_data[time_data_counter] = size;
        for (size_t i = 0; i < AVERAGE_COUNT; i++) {
            time_data_accessor = 0;
            data[i].size = size;

            // time_data[time_data_accessor++].data[time_data_counter] +=
            //     TEST_CONDITION_WITH_DATA(selection_sort, *(data + i), false, false);
            // time_data[time_data_accessor++].data[time_data_counter] +=
            //     TEST_CONDITION_WITH_DATA(insertion_sort, *(data + i), false, false);
            // time_data[time_data_accessor++].data[time_data_counter] +=
            //     TEST_CONDITION_WITH_DATA(bubble_sort, *(data + i), false, false);
            time_data[time_data_accessor++].data[time_data_counter] +=
                TEST_CONDITION_WITH_DATA(quick_sort, *(data + i), false, false);
            time_data[time_data_accessor++].data[time_data_counter] +=
                TEST_CONDITION_WITH_DATA(merge_sort, *(data + i), false, false);
        }

        time_data_accessor--;
        for (; time_data_accessor >= 0; time_data_accessor--) {
            time_data[time_data_accessor].data[time_data_counter] /= AVERAGE_COUNT;
        }

        time_data_counter++;
        printf("finished sorting %f%%...\n", (double)time_data_counter / time_data_count * 100);
    }

    time_data_accessor = 0;
    // write_csv("selection_sort.csv", time_data_count, size_data, time_data[time_data_accessor++]);
    // write_csv("insertion_sort.csv", time_data_count, size_data, time_data[time_data_accessor++]);
    // write_csv("bubble_sort.csv", time_data_count, size_data, time_data[time_data_accessor++]);
    write_csv("quick_sort.csv", time_data_count, size_data, time_data[time_data_accessor++]);
    write_csv("merge_sort.csv", time_data_count, size_data, time_data[time_data_accessor++]);

    puts("--------------------------------------------------");
    puts("finished all sorting!!!!");

    print_config(&config);

    for (size_t i = 0; i < AVERAGE_COUNT; i++) {
        free_data(data[i]);
    }
    for (size_t i = 0; i < ALGORITHM_COUNT; i++) {
        free_data(time_data[i]);
    }
    free(size_data);
}

void normal_mode() {
    Data data;
    Config config;

    input_config(&config);

    puts("generating test case...");
    data = gen_data(&config);
    puts("test case is generated");
    print_config(&config);
    new_line();

    TEST_CONDITION_WITH_DATA(insertion_sort, data, true, true);
    TEST_CONDITION_WITH_DATA(bubble_sort, data, true, true);
}

int main() {
    average_tester();

    // Data data;
    // Config config;
    // config.data_size = 1e5;
    // config.seed = 1;
    // config.mode = RANDOM;

    // printf("input data size: ");
    // scanf("%zu", &config.data_size);

    // int mode_input;
    // do {
    //     printf("input mode(0: RANDOM, 1: PARTIAL_RANDOM, 2: REVERSE, 3: INPUT): ");
    //     scanf("%d", &mode_input);
    // } while (mode_input < 0 || 3 < mode_input);
    // config.mode = (Mode)mode_input;

    // if (config.mode != INPUT) {
    //     printf("input seed: ");
    //     scanf("%u", &config.seed);
    // }
    // puts("generating test case...");
    // data = gen_data(&config);
    // puts("test case is generated");
    // print_config(&config);
    // new_line();

    // Data time_data[ALGORITHM_COUNT];

    // size_t time_data_count = 0;
    // for (size_t size = 1; size < config.data_size; size *= 2) {
    //     time_data_count++;
    // }

    // for (size_t i = 0; i < ALGORITHM_COUNT; i++) {
    //     time_data[i] = alloc_data(time_data_count);
    // }

    // size_t *size_data = malloc(time_data_count * sizeof(size_t));
    // if (size_data == NULL) {
    //     fprintf(stderr, "error: in main(), failed to allocate %zu bytes.\n", sizeof(size_t) * time_data_count);
    //     fprintf(stderr, "malloc error(%s)", strerror(errno));

    //     exit(1);
    // }

    // time_data_count = 0;
    // for (size_t size = 1; size < config.data_size; size *= 2) {
    //     size_t time_data_accessor = 0;
    //     data.size = size;

    //     size_data[time_data_count] = size;
    //     time_data[time_data_accessor++].data[time_data_count] = TEST_CONDITION_WITH_DATA(selection_sort, data, false, false);
    //     time_data[time_data_accessor++].data[time_data_count] = TEST_CONDITION_WITH_DATA(insertion_sort, data, false, false);
    //     time_data[time_data_accessor++].data[time_data_count] = TEST_CONDITION_WITH_DATA(bubble_sort, data, false, false);
    //     time_data[time_data_accessor++].data[time_data_count] = TEST_CONDITION_WITH_DATA(quick_sort, data, false, false);
    //     time_data[time_data_accessor++].data[time_data_count] = TEST_CONDITION_WITH_DATA(merge_sort, data, false, false);

    //     time_data_count++;
    // }

    // write_csv("selection_sort.csv", time_data_count, size_data, time_data[0]);
    // write_csv("insertion_sort.csv", time_data_count, size_data, time_data[1]);
    // write_csv("bubble_sort.csv", time_data_count, size_data, time_data[2]);
    // write_csv("quick_sort.csv", time_data_count, size_data, time_data[3]);
    // write_csv("merge_sort.csv", time_data_count, size_data, time_data[4]);

    // puts("--------------------------------------------------");
    // puts("finished all sorting!!!!");

    // print_config(&config);

    // free_data(data);
    // for (size_t i = 0; i < ALGORITHM_COUNT; i++) {
    //     free_data(time_data[i]);
    // }
    // free(size_data);

    // TEST_CONDITION_WITH_DATA(selection_sort, data, true, true);
    // TEST_CONDITION_WITH_DATA(insertion_sort, data, true, true);
    // TEST_CONDITION_WITH_DATA(bubble_sort, data, true, true);
    // TEST_CONDITION_WITH_DATA(quick_sort, data, true, true);
    // TEST_CONDITION_WITH_DATA(merge_sort, data, true, true);

    return 0;
}
