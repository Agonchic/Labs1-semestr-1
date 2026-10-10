#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_NULL,
    STATUS_ERR_NOT_POSITIVE,
    STATUS_ERR_INVALID_X,
    STATUS_ERR_OVERFLOW,
    STATUS_ERR_NO_CONVERGENCE
} StatusCode;

#define SUM_MAX_ITER 1000000
#define INT_MAX_N    1048576
#define INT_START_N  4
#define INT_SING_EPS 1e-12

StatusCode sum_a(const double x, const double eps, double * const result, int * const iters) {
    if (result == NULL || iters == NULL) return STATUS_ERR_NULL;
    if (eps <= 0.0) return STATUS_ERR_NOT_POSITIVE;

    double sum = 1.0;
    double term = 1.0;
    int n = 0;

    while (fabs(term) >= eps) {
        if (n >= SUM_MAX_ITER) return STATUS_ERR_NO_CONVERGENCE;
        n++;
        term = term * x / (double)n;
        sum += term;
        if (!isfinite(term) || !isfinite(sum)) return STATUS_ERR_OVERFLOW;
    }
    *result = sum;
    *iters = n;
    return STATUS_OK;
}

StatusCode sum_b(const double x, const double eps, double * const result, int * const iters) {
    if (result == NULL || iters == NULL) return STATUS_ERR_NULL;
    if (eps <= 0.0) return STATUS_ERR_NOT_POSITIVE;

    double sum = 1.0;
    double term = 1.0;
    int n = 0;

    while (fabs(term) >= eps) {
        if (n >= SUM_MAX_ITER) return STATUS_ERR_NO_CONVERGENCE;
        n++;
        const double denom = (double)(2 * n - 1) * (double)(2 * n);
        term = term * (-x * x) / denom;
        sum += term;
        if (!isfinite(term) || !isfinite(sum)) return STATUS_ERR_OVERFLOW;
    }
    *result = sum;
    *iters = n;
    return STATUS_OK;
}

StatusCode sum_c(const double x, const double eps, double * const result, int * const iters) {
    if (result == NULL || iters == NULL) return STATUS_ERR_NULL;
    if (eps <= 0.0) return STATUS_ERR_NOT_POSITIVE;

    double sum = 1.0;
    double term = 1.0;
    int n = 0;

    while (fabs(term) >= eps) {
        if (n >= SUM_MAX_ITER) return STATUS_ERR_NO_CONVERGENCE;
        n++;
        const double n3 = (double)n * (double)n * (double)n;
        const double denom = (double)(3 * n - 2) * (double)(3 * n - 1) * (double)(3 * n);
        term = term * 27.0 * n3 * x * x / denom;
        sum += term;
        if (!isfinite(term) || !isfinite(sum)) return STATUS_ERR_OVERFLOW;
    }
    *result = sum;
    *iters = n;
    return STATUS_OK;
}

StatusCode sum_d(const double x, const double eps, double * const result, int * const iters) {
    if (result == NULL || iters == NULL) return STATUS_ERR_NULL;
    if (eps <= 0.0) return STATUS_ERR_NOT_POSITIVE;
    if (fabs(x) >= 1.0) return STATUS_ERR_INVALID_X;

    double sum = 0.0;
    double term = -(x * x) / 2.0;
    int n = 1;

    while (fabs(term) >= eps) {
        sum += term;
        if (n >= SUM_MAX_ITER) return STATUS_ERR_NO_CONVERGENCE;
        n++;
        const double num = (double)(2 * n - 1) * x * x;
        const double den = (double)(2 * n);
        term = term * (-num) / den;
        if (!isfinite(term) || !isfinite(sum)) return STATUS_ERR_OVERFLOW;
    }
    *result = sum;
    *iters = n - 1;
    return STATUS_OK;
}

static double f_a(const double x, const double eps) {
    if (fabs(x) < eps) return 1.0;
    return log(1.0 + x) / x;
}

static double f_b(const double x, const double eps) {
    (void)eps;
    return exp(-x * x / 2.0);
}

static double f_c(const double x, const double eps) {
    const double d = 1.0 - x;
    if (fabs(d) < eps) return 0.0;
    return log(1.0 / d);
}

static double f_d(const double x, const double eps) {
    if (fabs(x) < eps) return 1.0;
    return pow(x, x);
}

static double simpson(double (*f)(const double, const double),
                      const double a, const double b,
                      const int n, const double eps) {
    const double h = (b - a) / (double)n;
    double s = f(a, eps) + f(b, eps);

    for (int i = 1; i < n; i++) {
        const double x_val = a + h * (double)i;
        if (i % 2 == 0) s += 2.0 * f(x_val, eps);
        else            s += 4.0 * f(x_val, eps);
    }
    return s * h / 3.0;
}

static StatusCode integrate(double (*f)(const double, const double),
                        const double a, const double b,
                        const double eps,
                        double * const result, int * const final_n) {
    if (result == NULL || final_n == NULL) return STATUS_ERR_NULL;
    if (eps <= 0.0) return STATUS_ERR_NOT_POSITIVE;

    int n = INT_START_N;
    double prev = simpson(f, a, b, n, eps);

    while (n <= INT_MAX_N) {
        n *= 2;
        const double cur = simpson(f, a, b, n, eps);
        if (fabs(cur - prev) / 15.0 < eps) {
            *result = cur;
            *final_n = n;
            return STATUS_OK;
        }
        prev = cur;
    }
    return STATUS_ERR_NO_CONVERGENCE;
}

static StatusCode integrate_c(const double eps, double * const result, int * const final_n) {
    return integrate(f_c, 0.0, 1.0 - INT_SING_EPS, eps, result, final_n);
}

static void print_sum(const char * const tag, const StatusCode s,
                      const double val, const int iters) {
    if (s == STATUS_OK) {
        printf("%s) sum = %.10g (iters = %d)\n", tag, val, iters);
    } else if (s == STATUS_ERR_NO_CONVERGENCE) {
        printf("%s) max iterations reached without convergence\n", tag);
    } else if (s == STATUS_ERR_INVALID_X) {
        printf("%s) x must be less than 1 by absolute value for this series\n", tag);
    } else if (s == STATUS_ERR_OVERFLOW) {
        printf("%s) numeric overflow\n", tag);
    } else {
        printf("%s) StatusCode code %d\n", tag, (int)s);
    }
}

static void print_int(const char * const tag, const StatusCode s,
                      const double val, const int n) {
    if (s == STATUS_OK) {
        printf("%s) integral = %.10g (n = %d)\n", tag, val, n);
    } else if (s == STATUS_ERR_NO_CONVERGENCE) {
        printf("%s) max subdivisions reached\n", tag);
    } else {
        printf("%s) StatusCode code %d\n", tag, (int)s);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <eps> <x>\n", argv[0]);
        return 1;
    }

    char *endptr = NULL;
    const double eps = strtod(argv[1], &endptr);
    if (*endptr != '\0' || eps <= 0.0 || eps >= 1.0) {
        fprintf(stderr, "eps must be in (0, 1)\n");
        return 1;
    }

    endptr = NULL;
    const double x = strtod(argv[2], &endptr);
    if (*endptr != '\0') {
        fprintf(stderr, "x must be a valid real number\n");
        return 1;
    }

    double val = 0.0;
    int iters = 0;
    StatusCode s;

    printf(" series (x = %g, eps = %g) \n", x, eps);

    s = sum_a(x, eps, &val, &iters);
    print_sum("a", s, val, iters);

    s = sum_b(x, eps, &val, &iters);
    print_sum("b", s, val, iters);

    s = sum_c(x, eps, &val, &iters);
    print_sum("c", s, val, iters);

    s = sum_d(x, eps, &val, &iters);
    print_sum("d", s, val, iters);

    printf("\n integrals on [0, 1] (eps = %g) \n", eps);

    int n_final = 0;

    s = integrate(f_a, 0.0, 1.0, eps, &val, &n_final);
    print_int("a", s, val, n_final);

    s = integrate(f_b, 0.0, 1.0, eps, &val, &n_final);
    print_int("b", s, val, n_final);

    s = integrate_c(eps, &val, &n_final);
    print_int("c", s, val, n_final);

    s = integrate(f_d, 0.0, 1.0, eps, &val, &n_final);
    print_int("d", s, val, n_final);

    return 0;
}

