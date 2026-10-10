#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_NULL,
    STATUS_ERR_ALLOC,
    STATUS_ERR_INVALID_ARGS,
    STATUS_ERR_RANGE,
    STATUS_ERR_OVERFLOW
} StatusCode;

#define FIXED_ARRAY_SIZE 20
#define DYN_ARRAY_MIN_SIZE 10
#define DYN_ARRAY_MAX_SIZE 10000
#define DYN_VAL_MIN -1000
#define DYN_VAL_MAX 1000
#define PRINT_LIMIT 35

static StatusCode parse_int(const char * const str, int * const result) {
    if (str == NULL || result == NULL) {
        return STATUS_ERR_NULL;
    }

    long long val = 0;
    int sign = 1;
    const char *p = str;

    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    if (*p == '\0') {
        return STATUS_ERR_INVALID_ARGS;
    }

    while (*p != '\0') {
        if (*p < '0' || *p > '9') {
            return STATUS_ERR_INVALID_ARGS;
        }

        int digit = *p - '0';

        if (sign == 1) {
            if (val > (INT_MAX - digit) / 10) return STATUS_ERR_OVERFLOW;
            val = val * 10 + digit;
        } else {
            if (val < (INT_MIN + digit) / 10) return STATUS_ERR_OVERFLOW;
            val = val * 10 - digit;
        }
        p++;
    }

    *result = (int)val;
    return STATUS_OK;
}

static StatusCode generate_random_array(int * const arr, const size_t size, const int min_val, const int max_val) {
    if (arr == NULL) {
        return STATUS_ERR_NULL;
    }
    if (min_val > max_val) {
        return STATUS_ERR_RANGE;
    }

    const long long range = (long long)max_val - (long long)min_val + 1;

    for (size_t i = 0; i < size; i++) {
        arr[i] = (int)(rand() % range) + min_val;
    }

    return STATUS_OK;
}

static StatusCode swap_min_max(int * const arr, const size_t size) {
    if (arr == NULL) {
        return STATUS_ERR_NULL;
    }
    if (size == 0) {
        return STATUS_OK;
    }

    size_t min_idx = 0;
    size_t max_idx = 0;

    for (size_t i = 1; i < size; i++) {
        if (arr[i] < arr[min_idx]) {
            min_idx = i;
        }
        if (arr[i] > arr[max_idx]) {
            max_idx = i;
        }
    }

    const int temp = arr[min_idx];
    arr[min_idx] = arr[max_idx];
    arr[max_idx] = temp;

    return STATUS_OK;
}

static StatusCode find_closest(const int * const B, const size_t size_b,
                           const int target, int * const result) {
    if (B == NULL || result == NULL) {
        return STATUS_ERR_NULL;
    }
    if (size_b == 0) {
        return STATUS_ERR_INVALID_ARGS;
    }

    size_t best_idx = 0;
    long long best_diff = (long long)B[0] - target;
    if (best_diff < 0) best_diff = -best_diff;

    for (size_t i = 1; i < size_b; i++) {
        long long d = (long long)B[i] - target;
        if (d < 0) d = -d;
        if (d < best_diff) {
            best_diff = d;
            best_idx = i;
        }
    }

    *result = B[best_idx];
    return STATUS_OK;
}

static StatusCode build_array_c(const int * const A, const size_t size_a, const int * const B, const size_t size_b, int ** const C) {
    if (A == NULL || B == NULL || C == NULL) {
        return STATUS_ERR_NULL;
    }
    if (size_a == 0 || size_b == 0) {
        return STATUS_ERR_INVALID_ARGS;
    }

    *C = (int *)malloc(size_a * sizeof(int));
    if (*C == NULL) {
        return STATUS_ERR_ALLOC;
    }

    for (size_t i = 0; i < size_a; i++) {
        int closest = 0;
        StatusCode s = find_closest(B, size_b, A[i], &closest);
        if (s != STATUS_OK) {
            free(*C);
            *C = NULL;
            return s;
        }
        (*C)[i] = A[i] + closest;
    }

    return STATUS_OK;
}

static void print_array(const char * const name, const int * const arr, const size_t size) {
    printf("%s [%zu elements]: ", name, size);
    size_t limit = (size > PRINT_LIMIT) ? PRINT_LIMIT : size;

    for (size_t i = 0; i < limit; i++) {
        printf("%d ", arr[i]);
    }

    if (size > PRINT_LIMIT) {
        printf("... (truncated)");
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <a> <b>\n", argv[0]);
        return STATUS_ERR_INVALID_ARGS;
    }

    int range_a = 0;
    int range_b = 0;

    StatusCode s = parse_int(argv[1], &range_a);
    if (s != STATUS_OK) {
        fprintf(stderr, "Error: Invalid argument 'a'\n");
        return s;
    }

    s = parse_int(argv[2], &range_b);
    if (s != STATUS_OK) {
        fprintf(stderr, "Error: Invalid argument 'b'\n");
        return s;
    }

    if (range_a > range_b) {
        fprintf(stderr, "Error: a must be less than or equal to b\n");
        return STATUS_ERR_RANGE;
    }

    srand((unsigned int)time(NULL));

    int fixed_arr[FIXED_ARRAY_SIZE];

    s = generate_random_array(fixed_arr, FIXED_ARRAY_SIZE, range_a, range_b);
    if (s != STATUS_OK) {
        fprintf(stderr, "Error: failed generate fixed array (code %d)\n", s);
        return s;
    }

    print_array("Original array", fixed_arr, FIXED_ARRAY_SIZE);

    s = swap_min_max(fixed_arr, FIXED_ARRAY_SIZE);
    if (s != STATUS_OK) {
        fprintf(stderr, "Error: failed swap min and max (code %d)\n", s);
        return s;
    }

    print_array("Array after swap", fixed_arr, FIXED_ARRAY_SIZE);

    const size_t size_a = (size_t)(rand() % (DYN_ARRAY_MAX_SIZE - DYN_ARRAY_MIN_SIZE + 1) + DYN_ARRAY_MIN_SIZE);
    const size_t size_b = (size_t)(rand() % (DYN_ARRAY_MAX_SIZE - DYN_ARRAY_MIN_SIZE + 1) + DYN_ARRAY_MIN_SIZE);

    int *arr_a = (int *)malloc(size_a * sizeof(int));
    int *arr_b = (int *)malloc(size_b * sizeof(int));
    int *arr_c = NULL;

    if (arr_a == NULL || arr_b == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for arrays A or B\n");
        free(arr_a);
        free(arr_b);
        return STATUS_ERR_ALLOC;
    }

    s = generate_random_array(arr_a, size_a, DYN_VAL_MIN, DYN_VAL_MAX);
    if (s != STATUS_OK) {
        fprintf(stderr, "Error: failed generate array A (code %d)\n", s);
        free(arr_a);
        free(arr_b);
        return s;
    }

    s = generate_random_array(arr_b, size_b, DYN_VAL_MIN, DYN_VAL_MAX);
    if (s != STATUS_OK) {
        fprintf(stderr, "Error: failed generate array B (code %d)\n", s);
        free(arr_a);
        free(arr_b);
        return s;
    }

    print_array("Array A", arr_a, size_a);
    print_array("Array B", arr_b, size_b);

    s = build_array_c(arr_a, size_a, arr_b, size_b, &arr_c);

    if (s == STATUS_OK) {
        print_array("Array C (Result)", arr_c, size_a);
    } else if (s == STATUS_ERR_ALLOC) {
        fprintf(stderr, "Error: memory allocation failed for array C\n");
    } else {
        fprintf(stderr, "Error: failed build array C (code %d)\n", s);
    }

    free(arr_a);
    free(arr_b);
    free(arr_c);

    return s;
}
