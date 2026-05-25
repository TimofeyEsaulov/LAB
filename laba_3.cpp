#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <chrono>
#include <algorithm>

using namespace std;

// Константы и структуры
const int INF = 1000000000; // Явно целое число

struct Cell {
    int r, c, h;
    bool operator>(const Cell& other) const { return h > other.h; }
};

// --- А) РЕАЛИЗАЦИЯ ЧЕРЕЗ МАССИВ (Неупорядоченный) ---
class ArrayPQ {
    vector<Cell> data;
public:
    void push(Cell c) { data.push_back(c); }
    Cell pop() {
        int minIdx = 0;
        for (int i = 1; i < (int)data.size(); ++i) {
            if (data[i].h < data[minIdx].h) minIdx = i;
        }
        Cell res = data[minIdx];
        data.erase(data.begin() + minIdx);
        return res;
    }
    bool empty() const { return data.empty(); }
};

// --- Б) РЕАЛИЗАЦИЯ ЧЕРЕЗ СВЯЗАННЫЙ СПИСОК (Упорядоченный) ---
class ListPQ {
    list<Cell> data;
public:
    void push(Cell c) {
        auto it = data.begin();
        while (it != data.end() && it->h < c.h) ++it;
        data.insert(it, c);
    }
    Cell pop() {
        Cell res = data.front();
        data.pop_front();
        return res;
    }
    bool empty() const { return data.empty(); }
};

// --- В) РЕАЛИЗАЦИЯ ЧЕРЕЗ STL (Используем композицию) ---
class STLPQ {
    priority_queue<Cell, vector<Cell>, greater<Cell>> pq;
public:
    void push(Cell c) { pq.push(c); }
    Cell pop() {
        Cell t = pq.top();
        pq.pop();
        return t;
    }
    bool empty() { return pq.empty(); }
};

// Расчет матрицы предельных уровней (используется в задаче Б)
vector<vector<int>> getWaterLevels(int M, int N, const vector<vector<int>>& A) {
    vector<vector<int>> water(M, vector<int>(N, INF));
    priority_queue<Cell, vector<Cell>, greater<Cell>> pq;

    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i == 0 || i == M - 1 || j == 0 || j == N - 1) {
                water[i][j] = A[i][j];
                pq.push({i, j, A[i][j]});
            }
        }
    }

    int dr[] = {0, 0, 1, -1}, dc[] = {1, -1, 0, 0};
    while (!pq.empty()) {
        Cell curr = pq.top(); pq.pop();
        for (int i = 0; i < 4; ++i) {
            int nr = curr.r + dr[i], nc = curr.c + dc[i];
            if (nr >= 0 && nr < M && nc >= 0 && nc < N && water[nr][nc] == INF) {
                water[nr][nc] = max(A[nr][nc], water[curr.r][curr.c]);
                pq.push({nr, nc, water[nr][nc]});
            }
        }
    }
    return water;
}

// Алгоритм расчета объема для задачи А
template <typename T>
long long calculateTotalVolume(int M, int N, const vector<vector<int>>& A) {
    if (M < 3 || N < 3) return 0;
    vector<vector<bool>> visited(M, vector<bool>(N, false));
    T pq;

    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i == 0 || i == M - 1 || j == 0 || j == N - 1) {
                pq.push({i, j, A[i][j]});
                visited[i][j] = true;
            }
        }
    }

    long long volume = 0;
    int dr[] = {0, 0, 1, -1}, dc[] = {1, -1, 0, 0};
    while (!pq.empty()) {
        Cell curr = pq.pop();
        for (int i = 0; i < 4; ++i) {
            int nr = curr.r + dr[i], nc = curr.c + dc[i];
            if (nr >= 0 && nr < M && nc >= 0 && nc < N && !visited[nr][nc]) {
                visited[nr][nc] = true;
                volume += max(0, curr.h - A[nr][nc]);
                pq.push({nr, nc, max(curr.h, A[nr][nc])});
            }
        }
    }
    return volume;
}

int main() {
    setlocale(LC_ALL, "Russian");
    cout << "Есаулов Тимофей Александрович, группа 090304-РПИа-25" << endl << endl;

    int M, N;
    cout << "Введите M и N: ";
    if (!(cin >> M >> N) || M <= 0 || N <= 0) return 1;

    vector<vector<int>> A(M, vector<int>(N));
    cout << "Введите высоты матрицы:" << endl;
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < N; ++j)
            cin >> A[i][j];

    cout << "\n--- 1. СРАВНЕНИЕ ПРОИЗВОДИТЕЛЬНОСТИ (Задача А) ---" << endl;
    const int ITER = 50; // Количество итераций для среднего времени

    auto benchmark = [&](auto type_tag, string name) {
        using T = decltype(type_tag);
        long long last_vol = 0;
        auto start = chrono::high_resolution_clock::now();
        for(int i = 0; i < ITER; ++i) {
            last_vol = calculateTotalVolume<T>(M, N, A);
        }
        auto end = chrono::high_resolution_clock::now();
        double avg_time = chrono::duration<double, micro>(end - start).count() / ITER;
        cout << name << ": Объем = " << last_vol << ", Среднее время = " << avg_time << " us" << endl;
    };

    benchmark(ArrayPQ(), "А) Массив");
    benchmark(ListPQ(), "Б) Список");
    benchmark(STLPQ(), "В) STL   ");

    cout << "\n--- 2. ВЫПОЛНЕНИЕ ЗАДАЧИ Б ---" << endl;
    int i0, j0;
    double V;
    cout << "Введите координаты (i0, j0): ";
    cin >> i0 >> j0;
    cout << "Введите объем воды V: ";
    cin >> V;

    // Валидация ввода
    if (i0 < 0 || i0 >= M || j0 < 0 || j0 >= N) {
        cout << "Ошибка: координаты вне диапазона." << endl;
    } else if (i0 == 0 || i0 == M - 1 || j0 == 0 || j0 >= N - 1) {
        cout << "Результат задачи (b): 0 (край формы)" << endl;
    } else {
        auto levels = getWaterLevels(M, N, A);
        int targetLevel = levels[i0][j0];
        long long depressionCap = 0; // Целочисленный объем

        if (targetLevel > A[i0][j0]) {
            queue<pair<int, int>> q;
            vector<vector<bool>> vis(M, vector<bool>(N, false));
            q.push({i0, j0});
            vis[i0][j0] = true;

            while (!q.empty()) {
                auto [r, c] = q.front(); q.pop();
                depressionCap += (levels[r][c] - A[r][c]);

                int dr[] = {0, 0, 1, -1}, dc[] = {1, -1, 0, 0};
                for (int i = 0; i < 4; i++) {
                    int nr = r + dr[i], nc = c + dc[i];
                    // Строгое условие == targetLevel для корректного выделения впадины
                    if (nr > 0 && nr < M - 1 && nc > 0 && nc < N - 1 && 
                        !vis[nr][nc] && levels[nr][nc] == targetLevel && levels[nr][nc] > A[nr][nc]) {
                        vis[nr][nc] = true;
                        q.push({nr, nc});
                    }
                }
            }
        }
        cout << "Результат задачи (b): " << min((long long)V, depressionCap) << endl;
    }

    return 0;
}