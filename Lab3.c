#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <float.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR_INVALID_PARAMETER = 1,
    STATUS_ERROR_OVERFLOW = 2,
    STATUS_ERROR_NO_REAL_ROOTS = 3,
    STATUS_ERROR_INFINITE_SOLUTIONS = 4,
    STATUS_ERROR_UNDERFLOW = 5
} status;



status parse_int(const char *const str, int *const out) {
    if (str == NULL || out == NULL || *str == '\0') {
        return STATUS_ERROR_INVALID_PARAMETER;
    }

    int sign = 1;
    const char *p = str;
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    if (*p == '\0') {
        return STATUS_ERROR_INVALID_PARAMETER;
    }



    long long value = 0;
    while (*p != '\0') {
        if (*p < '0' || *p > '9') {
            return STATUS_ERROR_INVALID_PARAMETER;
        }
        int digit = *p - '0';
        if (value > (LLONG_MAX - digit) / 10) {
            return STATUS_ERROR_OVERFLOW;
        }
        value = value * 10 + digit;
        p++;
    }


    long long signed_val = sign * value;
    if (signed_val < INT_MIN || signed_val > INT_MAX) {
        return STATUS_ERROR_OVERFLOW;
    }
    *out = (int)signed_val;
    return STATUS_OK;
}

status parse_double(const char *const str, double *const out) {
    if (str == NULL || out == NULL || *str == '\0') {
        return STATUS_ERROR_INVALID_PARAMETER;
    }
    char *end = NULL;
    double val = strtod(str, &end);

    if (end == str || *end != '\0') {
        return STATUS_ERROR_INVALID_PARAMETER;
    }
    if (!isfinite(val)) {
        return STATUS_ERROR_OVERFLOW;
    }
    *out = val;
    return STATUS_OK;
}

status m_key(const int num1, const int num2, int *const is_multiple) {

    if (is_multiple == NULL || num1 == 0 || num2 == 0) {
        return STATUS_ERROR_INVALID_PARAMETER;
    }
    if (num1 == INT_MIN && num2 == -1) {
        *is_multiple = 1;
        return STATUS_OK;
    }

    *is_multiple = (num1 % num2 == 0) ? 1 : 0;
    return STATUS_OK;
}

status t_key(const double s1, const double s2, const double s3,
                 const double eps, int *const is_right) {
    if (is_right == NULL || !isfinite(eps) || eps <= 0.0 ||
        !isfinite(s1) || !isfinite(s2) || !isfinite(s3) ||
        s1 <= 0.0 || s2 <= 0.0 || s3 <= 0.0) {
        return STATUS_ERROR_INVALID_PARAMETER;
    }

    const double max_safe = sqrt(DBL_MAX);
    const double min_safe = sqrt(DBL_MIN);

    if (s1 > max_safe || s2 > max_safe || s3 > max_safe) {
        return STATUS_ERROR_OVERFLOW;
    }
    if (s1 < min_safe || s2 < min_safe || s3 < min_safe) {
        return STATUS_ERROR_UNDERFLOW;
    }

    double sum12 = s1 + s2;
    double sum13 = s1 + s3;
    double sum23 = s2 + s3;
    double rhs1 = s1 + eps;
    double rhs2 = s2 + eps;
    double rhs3 = s3 + eps;

    if (!isfinite(sum12) || !isfinite(sum13) || !isfinite(sum23) ||
        !isfinite(rhs1) || !isfinite(rhs2) || !isfinite(rhs3)) {
        return STATUS_ERROR_OVERFLOW;
    }

    if ((sum12 <= rhs3) || (sum13 <= rhs2) || (sum23 <= rhs1)) {
        *is_right = 0;
        return STATUS_OK;
    }

    double a = s1, b = s2, c = s3;
    if (a > b) { double t = a; a = b; b = t; }
    if (b > c) { double t = b; b = c; c = t; }
    if (a > b) { double t = a; a = b; b = t; }

    double cathetus_sq = a * a + b * b;
    double hypotenuse_sq = c * c;

    if (!isfinite(cathetus_sq) || !isfinite(hypotenuse_sq)) {
        return STATUS_ERROR_OVERFLOW;
    }

    *is_right = (fabs(cathetus_sq - hypotenuse_sq) < eps) ? 1 : 0;
    return STATUS_OK;
}

status q_key(const double a, const double b, const double c,
                 const double eps,
                 double *const x1, double *const x2, int *const roots_count) {
    if (x1 == NULL || x2 == NULL || roots_count == NULL ||
        !isfinite(a) || !isfinite(b) || !isfinite(c) ||
        !isfinite(eps) || eps <= 0.0) {
        return STATUS_ERROR_INVALID_PARAMETER;
    }

    if (fabs(a) < eps) {
        if (fabs(b) < eps) {
            if (fabs(c) < eps) {
                *roots_count = -1;
                return STATUS_ERROR_INFINITE_SOLUTIONS;
            }
            *roots_count = 0;
            return STATUS_ERROR_NO_REAL_ROOTS;
        }
        double denom = b;
        if (!isfinite(denom) || denom == 0.0) {
            return STATUS_ERROR_OVERFLOW;
        }
        *x1 = -c / denom;
        if (!isfinite(*x1)) {
            return STATUS_ERROR_OVERFLOW;
        }
        *roots_count = 1;
        return STATUS_OK;
    }

    double b_sq = b * b;
    double ac4 = 4.0 * a * c;
    if (!isfinite(b_sq) || !isfinite(ac4)) {
        return STATUS_ERROR_OVERFLOW;
    }

    double d = b_sq - ac4;

    if (!isfinite(d)) {
        return STATUS_ERROR_OVERFLOW;
    }
    if (d < -eps) {
        *roots_count = 0;
        return STATUS_ERROR_NO_REAL_ROOTS;
    }

    double two_a = 2.0 * a;
    if (!isfinite(two_a) || two_a == 0.0) {
        return STATUS_ERROR_OVERFLOW;
    }

    if (fabs(d) < eps) {
        *x1 = -b / two_a;
        if (!isfinite(*x1)) {
            return STATUS_ERROR_OVERFLOW;
        }
        *roots_count = 1;
        return STATUS_OK;
    }

    double sqrt_d = sqrt(d);
    if (!isfinite(sqrt_d)) {
        return STATUS_ERROR_OVERFLOW;
    }

    *x1 = (-b + sqrt_d) / two_a;
    *x2 = (-b - sqrt_d) / two_a;

    if (!isfinite(*x1) || !isfinite(*x2)) {
        return STATUS_ERROR_OVERFLOW;
    }

    *roots_count = 2;
    return STATUS_OK;
}

static int triples_are_equal(const double a1, const double b1, const double c1, const double a2, const double b2, const double c2, const double eps) {
    double d1 = fabs(a1 - a2);
    double d2 = fabs(b1 - b2);
    double d3 = fabs(c1 - c2);
    if (!isfinite(d1) || !isfinite(d2) || !isfinite(d3)) {
        return 0;
    }
    return (d1 < eps) && (d2 < eps) && (d3 < eps);
}

status get_unique_permutations(const double src[3], const double eps, double perms[6][3], int *const count) {
    if (src == NULL || perms == NULL || count == NULL ||
        !isfinite(eps) || eps <= 0.0) {
        return STATUS_ERROR_INVALID_PARAMETER;
    }

    const double all[6][3] = {
        { src[0], src[1], src[2] },
        { src[0], src[2], src[1] },
        { src[1], src[0], src[2] },
        { src[1], src[2], src[0] },
        { src[2], src[0], src[1] },
        { src[2], src[1], src[0] }
    };

    int n = 0;
    for (int i = 0; i < 6; ++i) {
        int is_duplicate = 0;
        for (int j = 0; j < n; ++j) {
            if (triples_are_equal(all[i][0], all[i][1], all[i][2], perms[j][0], perms[j][1], perms[j][2], eps)) {
                is_duplicate = 1;
                break;
            }
        }
        if (!is_duplicate) {
            perms[n][0] = all[i][0];
            perms[n][1] = all[i][1];
            perms[n][2] = all[i][2];
            n++;
        }
    }
    *count = n;
    return STATUS_OK;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "ERROR: usage: %s <flag> <params>\n", argv[0]);
        return STATUS_ERROR_INVALID_PARAMETER;
    }

    const char *flag_str = argv[1];
    if ((flag_str[0] != '-' && flag_str[0] != '/') ||
        flag_str[1] == '\0' || flag_str[2] != '\0') {
        fprintf(stderr, "ERROR: bad flag\n");
        return STATUS_ERROR_INVALID_PARAMETER;
    }
    char flag = flag_str[1];

    if (flag == 'q') {
        if (argc != 6) {
            fprintf(stderr, "ERROR: q needs 4 params\n");
            return STATUS_ERROR_INVALID_PARAMETER;
        }

        double eps = 0.0;
        status sc = parse_double(argv[2], &eps);
        if (sc != STATUS_OK || eps <= 0.0 || eps > 1.0) {
            fprintf(stderr, "ERROR: bad eps\n");
            return STATUS_ERROR_INVALID_PARAMETER;
        }

        double coef[3];
        for (int i = 0; i < 3; ++i) {
            sc = parse_double(argv[3 + i], &coef[i]);
            if (sc == STATUS_ERROR_OVERFLOW) {
                fprintf(stderr, "ERROR: coef %d overflow\n", i + 1);
                return STATUS_ERROR_OVERFLOW;
            }
            if (sc != STATUS_OK) {
                fprintf(stderr, "ERROR: bad coef %d\n", i + 1);
                return STATUS_ERROR_INVALID_PARAMETER;
            }
        }

        double perms[6][3];
        int perm_count = 0;
        status ps = get_unique_permutations(coef, eps, perms, &perm_count);
        if (ps != STATUS_OK) {
            fprintf(stderr, "ERROR: perm error %d\n", ps);
            return ps;
        }

        for (int i = 0; i < perm_count; ++i) {
            double a = perms[i][0];
            double b = perms[i][1];
            double c = perms[i][2];

            printf("(a=%.6g b=%.6g c=%.6g): ", a, b, c);

            double x1 = 0.0, x2 = 0.0;
            int roots_cnt = 0;
            status res = q_key(a, b, c, eps, &x1, &x2, &roots_cnt);

            switch (res) {
                case STATUS_OK:
                    if (roots_cnt == 1) {
                        printf("x=%.6g\n", x1);
                    } else if (roots_cnt == 2) {
                        printf("x1=%.6g x2=%.6g\n", x1, x2);
                    }
                    break;
                case STATUS_ERROR_NO_REAL_ROOTS:
                    printf("no roots\n");
                    break;
                case STATUS_ERROR_INFINITE_SOLUTIONS:
                    printf("infinite\n");
                    break;
                case STATUS_ERROR_OVERFLOW:
                    printf("overflow\n");
                    break;
                default:
                    printf("ERROR: calc %d\n", res);
                    break;
            }
        }
        return STATUS_OK;
    }

    if (flag == 'm') {
        if (argc != 4) {
            fprintf(stderr, "ERROR: m needs 2 ints\n");
            return STATUS_ERROR_INVALID_PARAMETER;
        }

        int n1 = 0, n2 = 0;
        status sc1 = parse_int(argv[2], &n1);
        status sc2 = parse_int(argv[3], &n2);

        if (sc1 == STATUS_ERROR_OVERFLOW || sc2 == STATUS_ERROR_OVERFLOW) {
            fprintf(stderr, "ERROR: int overflow\n");
            return STATUS_ERROR_OVERFLOW;
        }
        if (sc1 != STATUS_OK || sc2 != STATUS_OK || n1 == 0 || n2 == 0) {
            fprintf(stderr, "ERROR: need non-zero ints\n");
            return STATUS_ERROR_INVALID_PARAMETER;
        }

        int is_multiple = 0;
        status res = m_key(n1, n2, &is_multiple);
        if (res == STATUS_OK) {
            if (is_multiple) {
                printf("%d multiple of %d\n", n1, n2);
            } else {
                printf("%d not multiple of %d\n", n1, n2);
            }
        } else {
            fprintf(stderr, "ERROR: m error %d\n", res);
            return res;
        }
        return STATUS_OK;
    }

    if (flag == 't') {
        if (argc != 6) {
            fprintf(stderr, "ERROR: t needs 4 params\n");
            return STATUS_ERROR_INVALID_PARAMETER;
        }

        double eps = 0.0;
        status sc = parse_double(argv[2], &eps);
        if (sc != STATUS_OK || eps <= 0.0 || eps > 1.0) {
            fprintf(stderr, "ERROR: bad eps\n");
            return STATUS_ERROR_INVALID_PARAMETER;
        }

        double s[3];
        for (int i = 0; i < 3; ++i) {
            sc = parse_double(argv[3 + i], &s[i]);
            if (sc == STATUS_ERROR_OVERFLOW) {
                fprintf(stderr, "ERROR: side %d overflow\n", i + 1);
                return STATUS_ERROR_OVERFLOW;
            }
            if (sc != STATUS_OK || s[i] <= 0.0) {
                fprintf(stderr, "ERROR: bad side %d\n", i + 1);
                return STATUS_ERROR_INVALID_PARAMETER;
            }
        }

        int is_right = 0;
        status res = t_key(s[0], s[1], s[2], eps, &is_right);

        switch (res) {
            case STATUS_OK:
                if (is_right) {
                    printf("right triangle\n");
                } else {
                    printf("not right triangle\n");
                }
                break;
            case STATUS_ERROR_OVERFLOW:
                fprintf(stderr, "ERROR: overflow\n");
                return res;
            case STATUS_ERROR_UNDERFLOW:
                fprintf(stderr, "ERROR: underflow\n");
                return res;
            default:
                fprintf(stderr, "ERROR: t error %d\n", res);
                return res;
        }
        return STATUS_OK;
    }

    fprintf(stderr, "ERROR: unknown flag %c\n", flag);
    return STATUS_ERROR_INVALID_PARAMETER;
}

