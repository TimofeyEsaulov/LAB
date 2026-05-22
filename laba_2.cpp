#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <cmath>
#include <iomanip>
#include <windows.h> 
#include <algorithm>
#include <mkl.h>

using namespace std;
using namespace std::chrono;

const size_t N = 2048;

// Вывод статистики согласно формулам из задания
static void print_stats(const string& variant_name, double time_s) {
    double c = 2.0 * pow((double)N, 3); // Операции: 2 * n^3
    double p = (c / time_s) * 1e-6;    // MFlops: c / t * 10^-6

    cout << "--------------------------------------------------\n";
    cout << variant_name << "\n";
    cout << "Время выполнения: " << fixed << setprecision(4) << time_s << " секунд\n";
    cout << "Производительность: " << fixed << setprecision(2) << p << " MFlops\n";
    cout << "--------------------------------------------------\n\n";
}

// Проверка корректности (вывод угла матрицы)
static void print_matrix_corner(const vector<float>& mat, const string& name, int print_size = 4) {
    int actual_size = min((int)N, print_size);
    cout << "Угол матрицы " << name << " (" << actual_size << "x" << actual_size << "):\n";
    for (int i = 0; i < actual_size; ++i) {
        for (int j = 0; j < actual_size; ++j) {
            cout << setw(10) << fixed << setprecision(3) << mat[i * N + j] << " ";
        }
        cout << "\n";
    }
    cout << "\n";
}

// 1-й вариант: По формуле из линейной алгебры (IJK)
static void multiply_naive(const vector<float>& A, const vector<float>& B, vector<float>& C) {
    fill(C.begin(), C.end(), 0.0f);
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            float sum = 0.0f;
            for (size_t k = 0; k < N; ++k) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
}

// 2-й вариант: Intel MKL (cblas_sgemm)
static void multiply_blas(const vector<float>& A, const vector<float>& B, vector<float>& C) {
    // alpha = 1.0 (умножение), beta = 0.0 (запись в C без прибавления старого значения)
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
        (int)N, (int)N, (int)N,
        1.0f, A.data(), (int)N,
        B.data(), (int)N,
        0.0f, C.data(), (int)N);
}

// 3-й вариант: Оптимизированный алгоритм (IKJ + OpenMP + SIMD)
static void multiply_optimized(const vector<float>& A, const vector<float>& B, vector<float>& C) {
    fill(C.begin(), C.end(), 0.0f);

    // Порядок IKJ минимизирует промахи кэша (Cache Misses)
    #pragma omp parallel for
    for (int i = 0; i < (int)N; ++i) {
        for (int k = 0; k < (int)N; ++k) {
            float a_val = A[i * N + k];
            const float* b_row = &B[k * N];
            float* c_row = &C[i * N];

            #pragma omp simd
            for (int j = 0; j < (int)N; ++j) {
                c_row[j] += a_val * b_row[j];
            }
        }
    }
}

int main() {
    // Настройка кириллицы в консоли
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    cout << "ФИО: Есаулов Тимофей Александрович" << "\n";
    cout << "Группа: 090304-РПИа-о25" << "\n";
    cout << "Размер матриц (float): " << N << "x" << N << "\n\n";

    vector<float> A(N * N), B(N * N), C_mkl(N * N), C_opt(N * N), C_naive(N * N);

    // Генерация случайных данных
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> dist(0.0f, 1.0f);

    cout << "Генерация матриц... ";
    for (size_t i = 0; i < N * N; ++i) {
        A[i] = dist(gen);
        B[i] = dist(gen);
    }
    cout << "Готово!\n\n";

    // 1. MKL
    cout << "Запуск Intel MKL (sgemm)...\n";
    auto start = high_resolution_clock::now();
    multiply_blas(A, B, C_mkl);
    auto end = high_resolution_clock::now();
    double time_mkl = duration_cast<duration<double>>(end - start).count();
    print_stats("Вариант 2: Intel MKL", time_mkl);
    print_matrix_corner(C_mkl, "MKL");

    // 2. Optimized (IKJ)
    cout << "Запуск оптимизированного варианта (IKJ + OpenMP)...\n";
    start = high_resolution_clock::now();
    multiply_optimized(A, B, C_opt);
    end = high_resolution_clock::now();
    double time_opt = duration_cast<duration<double>>(end - start).count();
    print_stats("Вариант 3: Оптимизированный", time_opt);
    print_matrix_corner(C_opt, "Optimized");

    // 3. Naive (может быть очень медленным!)
    cout << "Запуск наивного варианта (IJK)... Пожалуйста, подождите.\n";
    start = high_resolution_clock::now();
    multiply_naive(A, B, C_naive);
    end = high_resolution_clock::now();
    double time_naive = duration_cast<duration<double>>(end - start).count();
    print_stats("Вариант 1: Наивный", time_naive);

    // Сравнение производительности
    double perf_ratio = (time_mkl / time_opt) * 100.0;
    cout << "Анализ: Оптимизированный алгоритм составляет " << fixed << setprecision(2) 
         << perf_ratio << "% от скорости MKL.\n";

    if (perf_ratio >= 30.0) 
        cout << "Требование лабы (не ниже 30%) выполнено!\n";
    else 
        cout << "Внимание: Скорость ниже 30%, проверьте флаги оптимизации компилятора.\n";

    return 0;
}
