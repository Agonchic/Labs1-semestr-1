#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

typedef enum {
    SUCCESS = 0,
    ERROR_OVERFLOW,
    ERROR_INVALID_ARGUMENTS
} status;

static status parse_base(const char *str, long *out_val)
{
    if (str == NULL || out_val == NULL || *str == '\0') {
        return ERROR_INVALID_ARGUMENTS;
    }

    if (isspace((unsigned char)str[0])) {
        return ERROR_INVALID_ARGUMENTS;
    }

    errno = 0;
    char *endptr = NULL;
    long val = strtol(str, &endptr, 10);

    if (errno == ERANGE) {
        return ERROR_OVERFLOW;
    }
    if (str == endptr || *endptr != '\0') {
        return ERROR_INVALID_ARGUMENTS;
    }
    if (val < 2 || val > 36) {
        return ERROR_INVALID_ARGUMENTS;
    }

    *out_val = val;
    return SUCCESS;
}

static status char_to_digit(const char ch, const long base, long long *out_val)
{
    long long digit;

    if (isdigit((unsigned char)ch)) {
        digit = ch - '0';
    } else if (isupper((unsigned char)ch)) {
        digit = ch - 'A' + 10;
    } else {
        return ERROR_INVALID_ARGUMENTS;
    }

    if (digit >= base) {
        return ERROR_INVALID_ARGUMENTS;
    }

    *out_val = digit;
    return SUCCESS;
}

static status str_to_num(const char *str, const long base, long long *out_val)
{
    if (str == NULL || out_val == NULL || *str == '\0') {
        return ERROR_INVALID_ARGUMENTS;
    }

    const char *pch = str;
    long long res = 0;
    int sign = 1;

    if (*pch == '-') {
        sign = -1;
        ++pch;
    } else if (*pch == '+') {
        ++pch;
    }

    if (*pch == '-' || *pch == '+' || *pch == '\0') {
        return ERROR_INVALID_ARGUMENTS;
    }

    while (*pch != '\0') {
        long long digit;
        status err = char_to_digit(*pch, base, &digit);
        if (err != SUCCESS) {
            return err;
        }

        if (res < (LLONG_MIN + digit) / base) {
            return ERROR_OVERFLOW;
        }
        res = res * base - digit;

        ++pch;
    }

    if (sign == 1) {
        if (res == LLONG_MIN) {
            return ERROR_OVERFLOW;
        }
        res = -res;
    }

    *out_val = res;
    return SUCCESS;
}

static status num_to_str(const long long value, const long base, char *buf, const int buf_size)
{
    if (buf == NULL || buf_size < 2 || base < 2 || base > 36) {
        return ERROR_INVALID_ARGUMENTS;
    }

    int idx = buf_size - 1;
    buf[idx] = '\0';

    const int is_negative = (value < 0);
    long long temp = is_negative ? value : -value;

    do {
        if (idx <= (is_negative ? 1 : 0)) {
            return ERROR_OVERFLOW;
        }

        const int remainder = -(int)(temp % base);
        buf[--idx] = (remainder < 10)
                     ? (char)('0' + remainder)
                     : (char)('A' + (remainder - 10));
        temp /= base;
    } while (temp != 0);

    if (is_negative) {
        buf[--idx] = '-';
    }

    int i = 0;
    while (buf[idx] != '\0') {
        buf[i++] = buf[idx++];
    }
    buf[i] = '\0';

    return SUCCESS;
}

static void print_error(const status err)
{
    if (err == ERROR_INVALID_ARGUMENTS) {
        printf("Error: invalid argument value or count\n");
    } else if (err == ERROR_OVERFLOW) {
        printf("Error: value overflow\n");
    } else if (err != SUCCESS) {
        printf("Error: unknown status code (%d)\n", (int)err);
    }
}

#define SIZE_BUF 256
#define STR_BUF  64

int main(int argc, char *argv[])
{
    if (argc != 2) {
        print_error(ERROR_INVALID_ARGUMENTS);
        return ERROR_INVALID_ARGUMENTS;
    }

    long base;
    status err = parse_base(argv[1], &base);
    if (err != SUCCESS) {
        print_error(err);
        return err;
    }

    char str[STR_BUF];
    long long sum = 0;
    long long max = 0;
    int count = 0;

    while (1) {
        if (scanf("%63s", str) != 1) {
            print_error(ERROR_INVALID_ARGUMENTS);
            return ERROR_INVALID_ARGUMENTS;
        }

        if (strcmp(str, "Stop") == 0) {
            break;
        }

        if (strlen(str) >= STR_BUF - 1) {
            print_error(ERROR_INVALID_ARGUMENTS);
            return ERROR_INVALID_ARGUMENTS;
        }

        long long num;
        err = str_to_num(str, base, &num);
        if (err != SUCCESS) {
            print_error(err);
            return err;
        }

        if (num > 0 && sum > 0) {
            if (num > LLONG_MAX - sum) {
                print_error(ERROR_OVERFLOW);
                return ERROR_OVERFLOW;
            }
        } else if (num < 0 && sum < 0) {
            if (num < LLONG_MIN - sum) {
                print_error(ERROR_OVERFLOW);
                return ERROR_OVERFLOW;
            }
        }
        sum += num;

        if (count == 0) {
            max = num;
        } else {
            const long long neg_num = (num > 0) ? -num : num;
            const long long neg_max = (max > 0) ? -max : max;
            if (neg_num < neg_max) {
                max = num;
            }
        }

        ++count;
    }

    if (count == 0) {
        print_error(ERROR_INVALID_ARGUMENTS);
        return ERROR_INVALID_ARGUMENTS;
    }

    printf("%lld %lld\n", sum, max);

    char buf[SIZE_BUF];
    for (int b = 9; b <= 36; b += 9) {
        err = num_to_str(sum, b, buf, SIZE_BUF);
        if (err != SUCCESS) {
            print_error(err);
            return err;
        }
        printf("%s ", buf);

        err = num_to_str(max, b, buf, SIZE_BUF);
        if (err != SUCCESS) {
            print_error(err);
            return err;
        }
        printf("%s\n", buf);
    }

    return 0;
}
