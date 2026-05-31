#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <algorithm>

using namespace std;

// константы и структуры
const int INF = 1000000000; 

struct Cell {
    int r, c, h;  // координаты и высота
    bool operator>(const Cell& other) const { return h > other.h; }
};

// А) реализация через динамический массив
class ArrayPQ {
    Cell* data;
    int size;
    int capacity;
    
    void resize() {
        int newCapacity = (capacity == 0) ? 4 : capacity * 2;
        Cell* newData = new Cell[newCapacity];
        for (int i = 0; i < size; ++i) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
        capacity = newCapacity;
    }
    
public:
    ArrayPQ() : data(nullptr), size(0), capacity(0) {}
    
    ~ArrayPQ() {
        delete[] data;
    }
    
    ArrayPQ(const ArrayPQ&) = delete;
    ArrayPQ& operator=(const ArrayPQ&) = delete;
    
    void push(Cell c) {
        if (size >= capacity) {
            resize();
        }
        data[size++] = c;
    }
    
    Cell pop() {
        if (size == 0) throw runtime_error("pop from empty ArrayPQ");
        
        int minIdx = 0;
        for (int i = 1; i < size; ++i) {
            if (data[i].h < data[minIdx].h) {
                minIdx = i;
            }
        }
        Cell res = data[minIdx];
        data[minIdx] = data[--size];
        return res;
    }
    
    bool empty() const { return size == 0; }
};

// Б) реализация через связанный список (динамическая память)
class ListPQ {
    struct Node {
        Cell data;
        Node* next;
        Node(const Cell& c) : data(c), next(nullptr) {}
    };
    
    Node* head;
    int sz;
    
public:
    ListPQ() : head(nullptr), sz(0) {}
    
    ~ListPQ() {
        while (head) {
            Node* tmp = head;
            head = head->next;
            delete tmp;
        }
    }
    
    ListPQ(const ListPQ&) = delete;
    ListPQ& operator=(const ListPQ&) = delete;
    
    void push(Cell c) {
        Node* newNode = new Node(c);
        
        if (!head || c.h < head->data.h) {
            newNode->next = head;
            head = newNode;
        } else {
            Node* curr = head;
            while (curr->next && curr->next->data.h < c.h) {
                curr = curr->next;
            }
            newNode->next = curr->next;
            curr->next = newNode;
        }
        ++sz;
    }
    
    Cell pop() {
        if (!head) throw runtime_error("pop from empty ListPQ");
        
        Node* tmp = head;
        Cell res = tmp->data;
        head = head->next;
        delete tmp;
        --sz;
        return res;
    }
    
    bool empty() const { return head == nullptr; }
};

// В) реализация через STL
class STLPQ {
    priority_queue<Cell, vector<Cell>, greater<Cell>> pq;
public:
    void push(Cell c) { pq.push(c); }
    
    Cell pop() {
        Cell t = pq.top();
        pq.pop();
        return t;
    }
    
    bool empty() const { return pq.empty(); }
};

//Расчёт матрицы предельных уровней воды (для второй задачи)
vector<vector<int>> getWaterLevels(int M, int N, const vector<vector<int>>& A) {
    vector<vector<int>> water(M, vector<int>(N, INF));
    priority_queue<Cell, vector<Cell>, greater<Cell>> pq;

    // все граничные ячейки имеют уровень равный своей высоте
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

// Расчёт общего объёма воды (ЗАДАЧА А), шаблонная функция для сравнения трёх реализаций приоритетной очереди
template <typename T>
long long calculateTotalVolume(int M, int N, const vector<vector<int>>& A) {
    if (M < 3 || N < 3) return 0;
    
    vector<vector<bool>> visited(M, vector<bool>(N, false));
    T pq;

    // добавление всех граничных ячеек
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

// main
int main() {
    setlocale(LC_ALL, "Russian");
    cout << "Есаулов Тимофей Александрович, группа 090304-РПИа-о25" << endl << endl;

    // Ввод размеров матрицы
    int M, N;
    cout << "Введите M и N: ";
    if (!(cin >> M >> N) || M <= 0 || N <= 0) {
        cout << "Ошибка: некорректные размеры матрицы." << endl;
        return 1;
    }

    //Ввод матрицы высот
    vector<vector<int>> A(M, vector<int>(N));
    cout << "Введите высоты матрицы:" << endl;
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < N; ++j)
            cin >> A[i][j];

    // ЗАДАЧА А: Сравнение производительности трёх реализаций
    cout << "\n--- 1. СРАВНЕНИЕ ПРОИЗВОДИТЕЛЬНОСТИ (Задача А) ---" << endl;
    const int ITER = 50;

    auto benchmark = [&](auto type_tag, string name) {
        using T = decltype(type_tag);
        long long last_vol = 0;
        auto start = chrono::high_resolution_clock::now();
        for (int i = 0; i < ITER; ++i) {
            last_vol = calculateTotalVolume<T>(M, N, A);
        }
        auto end = chrono::high_resolution_clock::now();
        double avg_time = chrono::duration<double, micro>(end - start).count() / ITER;
        cout << name << ": Объем = " << last_vol << ", Среднее время = " << avg_time << " us" << endl;
    };

    benchmark(ArrayPQ(), "А) Массив (ручной)");
    benchmark(ListPQ(), "Б) Список (ручной)");
    benchmark(STLPQ(), "В) STL (библиотека)");

    //Проверка: все три реализации должны дать одинаковый результат
    long long v1 = calculateTotalVolume<ArrayPQ>(M, N, A);
    long long v2 = calculateTotalVolume<ListPQ>(M, N, A);
    long long v3 = calculateTotalVolume<STLPQ>(M, N, A);
    if (v1 == v2 && v2 == v3) {
        cout << "\n Все реализации дали одинаковый результат: " << v1 << endl;
    } else {
        cout << "\n  Внимание: разные результаты! Массив=" << v1 
             << ", Список=" << v2 << ", STL=" << v3 << endl;
    }

    // ЗАДАЧА Б: Вода выливается в конкретную точку с координатами (i0, j0)
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
    } else if (i0 == 0 || i0 == M - 1 || j0 == 0 || j0 == N - 1) {
        // если точка на границе, вода сразу стекает с края
        cout << "Результат задачи (b): 0 (точка на границе — вода вытекает)" << endl;
    } else {
        // Вычисляем предельные уровни воды для всех ячеек
        auto levels = getWaterLevels(M, N, A);
        int targetLevel = levels[i0][j0];
        long long depressionCap = 0;

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
