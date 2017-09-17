#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>

#define MAX_INT 9

using namespace std;
using namespace std::chrono;

// returns time in milliseconds
long get_time()
{
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

// returns a 2D vector of random integers
vector<vector<int>> randomize_matrix(int n)
{
  random_device rd;
  mt19937 rng(rd());
  uniform_int_distribution<int> uni(0, MAX_INT);

  vector<vector<int>> matrix;

  for (int row = 0; row < n; ++row)
  {
    vector<int> matrix_row;
    for (int col = 0; col < n; ++col)
    {
      matrix_row.push_back(uni(rng));
    }
    matrix.push_back(matrix_row);
  }

  return matrix;
}

// returns a vector of matrices derived from distributing the main matrix
// into t parts
vector<vector<vector<int>>> distribute_matrix(vector<vector<int>> matrix, int n, int t)
{
  int cols = n / t;
  vector<vector<vector<int>>> submatrices;
  for (int thread = 0; thread < t; ++thread)
  {
    vector<vector<int>> submatrix;
    for (int row = 0; row < n; ++row)
    {
      vector<int> submatrix_row;
      for (int col = 0; col < cols; ++col)
      {
        submatrix_row.push_back(matrix[row][(thread * cols) + col]);
      }
      submatrix.push_back(submatrix_row);
    }
    submatrices.push_back(submatrix);
  }

  return submatrices;
}

vector<int> solve_column_sum(vector<vector<int>> matrix)
{
  vector<int> vector_sum;

  for (auto col = 0; col < matrix[0].size(); ++col)
  {
    vector_sum.push_back(0);
    for (auto row = 0; row < matrix.size(); ++row)
    {
      vector_sum[col] += matrix[row][col];
    }
  }

  return vector_sum;
}

// https://stackoverflow.com/questions/1407786/how-to-set-cpu-affinity-of-a-particular-pthread
int assign_to_cpu(pthread_t thread, int core_id)
{
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);

  return pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
}

int main()
{
  unsigned NUM_CORES = thread::hardware_concurrency();
  int n, t;

  cout << "N: ";
  cin >> n;
  cout << "T: ";
  cin >> t;

  auto time_start = get_time();
  cout << "Randomzing Matrix...";
  auto matrix = randomize_matrix(n);
  cout << "\t" << get_time() - time_start << "ms" << endl;

  time_start = get_time();
  cout << "Dividing Matrix...";
  auto submatrices = distribute_matrix(matrix, n, t);
  cout << "\t" << get_time() - time_start << "ms" << endl;

  time_start = get_time();
  cout << "Solving Column Sums...";

  vector<thread> threads;
  for (int i = 0; i < t; ++i)
  {
    threads.push_back(thread(solve_column_sum, submatrices[i]));
    assign_to_cpu(threads[i].native_handle(), i % NUM_CORES);
  }
  for (auto &th : threads)
    th.join();

  cout << "\t" << get_time() - time_start << "ms" << endl;

  return 0;
}