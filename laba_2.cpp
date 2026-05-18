#include <iostream>
#include <vector>
#include <complex>
#include <chrono>
#include <random>
#include <cmath>
#include <iomanip>
#include <windows.h> 
#include <algorithm>
#include <mkl.h>

using namespace std;
using namespace std::chrono;

// Используем float для комплексных чисел
typedef complex<float> ComplexD;

const size_t N = 2048;

static void print_stats(const string& variant_name, double time_s) {
    double c = 2.0 * pow((double)N, 3);
    double p = (c / time_s) * 1e-6;

    cout << "--------------------------------------------------\n";
    cout << variant_name << "\n";
    cout << "Время выполнения: " << fixed << setprecision(4) << time_s << " секунд\n";
    cout << "Производительность: " << fixed << setprecision(2) << p << " MFlops\n";
    cout << "--------------------------------------------------\n\n";
}

static void print_matrix_corner(const vector<ComplexD>& mat, const string& name, int print_size = 4) {
    int actual_size = min((int)N, print_size);
    cout << "Часть матрицы результата " << name << " (левый верхний угол " << actual_size << "x" << actual_size << "):\n";
    for (int i = 0; i < actual_size; ++i) {
        for (int j = 0; j < actual_size; ++j) {
            cout << fixed << setprecision(3) << "[" << mat[i * N + j].real();
            if (mat[i * N + j].imag() >= 0) cout << "+";
            cout << mat[i * N + j].imag() << "i]  ";
        }
        cout << "\n";
    }
    cout << "\n";
}

static void multiply_naive(const vector<ComplexD>& A, const vector<ComplexD>& B, vector<ComplexD>& C) {
    fill(C.begin(), C.end(), ComplexD(0, 0));
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            ComplexD sum(0.0f, 0.0f);
            for (size_t k = 0; k < N; ++k) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
}

static void multiply_blas(const vector<ComplexD>& A, const vector<ComplexD>& B, vector<ComplexD>& C) {
    // Для cblas_cgemm параметры alpha и beta передаются как указатели на float[2] или MKL_Complex8
    float alpha[2] = { 1.0f, 0.0f };
    float beta[2] = { 0.0f, 0.0f };

    cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
        (int)N, (int)N, (int)N,
        alpha, (float*)A.data(), (int)N,
        (float*)B.data(), (int)N,
        beta, (float*)C.data(), (int)N);
}

static void multiply_optimized(const vector<ComplexD>& A, const vector<ComplexD>& B, vector<ComplexD>& C) {
    // 1. Подготовка SoA (Structure of Arrays) на float
    vector<float> Ar(N * N), Ai(N * N);
    vector<float> Br(N * N), Bi(N * N);
    vector<float> Cr(N * N, 0.0f), Ci(N * N, 0.0f);

#pragma omp parallel for schedule(static)
    for (int i = 0; i < (int)(N * N); ++i) {
        Ar[i] = A[i].real(); Ai[i] = A[i].imag();
        Br[i] = B[i].real(); Bi[i] = B[i].imag();
    }

    const int blockSize = 64;

    // 2. Вычисления (IKJ + SoA + Register Blocking 4x1)
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < (int)N; i += blockSize) {
        for (int k = 0; k < (int)N; k += blockSize) {

            int i_end = min(i + blockSize, (int)N);
            int k_end = min(k + blockSize, (int)N);

            for (int ii = i; ii < i_end; ii += 4) {
                float* c0r = &Cr[ii * N];     float* c0i = &Ci[ii * N];
                float* c1r = &Cr[(ii + 1) * N]; float* c1i = &Ci[(ii + 1) * N];
                float* c2r = &Cr[(ii + 2) * N]; float* c2i = &Ci[(ii + 2) * N];
                float* c3r = &Cr[(ii + 3) * N]; float* c3i = &Ci[(ii + 3) * N];

                for (int kk = k; kk < k_end; ++kk) {
                    float a0r = Ar[ii * N + kk];     float a0i = Ai[ii * N + kk];
                    float a1r = Ar[(ii + 1) * N + kk]; float a1i = Ai[(ii + 1) * N + kk];
                    float a2r = Ar[(ii + 2) * N + kk]; float a2i = Ai[(ii + 2) * N + kk];
                    float a3r = Ar[(ii + 3) * N + kk]; float a3i = Ai[(ii + 3) * N + kk];

                    const float* __restrict br_row = &Br[kk * N];
                    const float* __restrict bi_row = &Bi[kk * N];

#pragma omp simd
                    for (int j = 0; j < (int)N; ++j) {
                        float br = br_row[j];
                        float bi = bi_row[j];

                        // Комплексное умножение: (ar+ai*i)*(br+bi*i) = (ar*br - ai*bi) + i*(ar*bi + ai*br)
                        c0r[j] += a0r * br - a0i * bi;
                        c0i[j] += a0r * bi + a0i * br;

                        c1r[j] += a1r * br - a1i * bi;
                        c1i[j] += a1r * bi + a1i * br;

                        c2r[j] += a2r * br - a2i * bi;
                        c2i[j] += a2r * bi + a2i * br;

                        c3r[j] += a3r * br - a3i * bi;
                        c3i[j] += a3r * bi + a3i * br;
                    }
                }
            }
        }
    }

    // 3. Сборка обратно в vector<ComplexD>
#pragma omp parallel for schedule(static)
    for (int i = 0; i < (int)(N * N); ++i) {
        C[i] = ComplexD(Cr[i], Ci[i]);
    }
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");

    cout << "ФИО: Есаулов Тимофей Александрович" << "\n";
    cout << "Номер группы: 090304-РПИа-25" << "\n";
    cout << "Размер матриц (Float Complex): " << N << "x" << N << "\n\n";

    // Инициализация векторов (выделение памяти)
    vector<ComplexD> A(N * N);
    vector<ComplexD> B(N * N);
    vector<ComplexD> C_mkl(N * N);
    vector<ComplexD> C_opt(N * N);

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> dist(-1.0f, 1.0f);

    cout << "Генерация матриц... ";
    for (size_t i = 0; i < N * N; ++i) {
        A[i] = ComplexD(dist(gen), dist(gen));
        B[i] = ComplexD(dist(gen), dist(gen));
    }
    cout << "Готово!\n\n";

    // Вариант MKL
    cout << "Запуск Intel MKL (zgemm)...\n";
    auto start = high_resolution_clock::now();
    multiply_blas(A, B, C_mkl);
    auto end = high_resolution_clock::now();
    double time_mkl = duration_cast<duration<double>>(end - start).count();
    print_stats("Вариант MKL (Float)", time_mkl);
    print_matrix_corner(C_mkl, "MKL");

    // Вариант Оптимизированный
    cout << "Запуск оптимизированного варианта (Float SoA)...\n";
    start = high_resolution_clock::now();
    multiply_optimized(A, B, C_opt);
    end = high_resolution_clock::now();
    double time_opt = duration_cast<duration<double>>(end - start).count();
    print_stats("Вариант Оптимизированный (Float)", time_opt);
    print_matrix_corner(C_opt, "Optimized");

    // Наивный вариант
    cout << "Запуск наивного варианта...\n";
    vector<ComplexD> C_naive(N * N);
    start = high_resolution_clock::now();
    multiply_naive(A, B, C_naive);
    end = high_resolution_clock::now();
    double time_naive = duration_cast<duration<double>>(end - start).count();
    print_stats("Вариант Наивный", time_naive);

    double ratio = (time_mkl / time_opt) * 100.0;
    cout << "Результат: Ваш алгоритм выдает " << fixed << setprecision(1) << ratio
        << "% от производительности MKL.\n";

    return 0;
}