#include "gusev_d_double_sort_even_odd_batcher_tbb/tbb/include/ops_tbb.hpp"

#include <tbb/tbb.h>

#include <functional>

namespace gusev_d_double_sort_even_odd_batcher_tbb_task_threads {
namespace {

void SortWithTbb(const InType &input, OutType &output) {
  output = input;
  if (output.size() < 2) {
    return;
  }

  tbb::parallel_sort(output.begin(), output.end(), std::less<ValueType>());
}

}  // namespace

DoubleSortEvenOddBatcherTBB::DoubleSortEvenOddBatcherTBB(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = {};
}

bool DoubleSortEvenOddBatcherTBB::ValidationImpl() {
  return GetOutput().empty();
}

bool DoubleSortEvenOddBatcherTBB::PreProcessingImpl() {
  input_data_ = GetInput();
  result_data_.clear();
  return true;
}

bool DoubleSortEvenOddBatcherTBB::RunImpl() {
  SortWithTbb(input_data_, result_data_);
  return true;
}

bool DoubleSortEvenOddBatcherTBB::PostProcessingImpl() {
  GetOutput() = result_data_;
  return true;
}

}  // namespace gusev_d_double_sort_even_odd_batcher_tbb_task_threads
