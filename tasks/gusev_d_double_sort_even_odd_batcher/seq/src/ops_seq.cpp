#include "gusev_d_double_sort_even_odd_batcher/seq/include/ops_seq.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <vector>

namespace {

uint64_t DoubleToSortableKey(double value) {
  const uint64_t bits = std::bit_cast<uint64_t>(value);
  const uint64_t sign_mask = 0x8000000000000000ULL;
  return (bits & sign_mask) == 0 ? (bits ^ sign_mask) : (~bits);
}

void RadixSortDoubles(std::vector<double>& data) {
  if (data.size() < 2) {
    return;
  }

  std::vector<double> buffer(data.size());
  std::vector<double>* src = &data;
  std::vector<double>* dst = &buffer;

  for (int byte = 0; byte < 8; ++byte) {
    std::array<size_t, 256> count{};
    const int shift = byte * 8;

    for (double value : *src) {
      const uint8_t bucket = static_cast<uint8_t>((DoubleToSortableKey(value) >> shift) & 0xFFULL);
      count[bucket]++;
    }

    size_t prefix = 0;
    for (size_t& bucket_count : count) {
      const size_t current = bucket_count;
      bucket_count = prefix;
      prefix += current;
    }

    for (double value : *src) {
      const uint8_t bucket = static_cast<uint8_t>((DoubleToSortableKey(value) >> shift) & 0xFFULL);
      (*dst)[count[bucket]++] = value;
    }

    std::swap(src, dst);
  }

  if (src != &data) {
    data = *src;
  }
}

std::vector<double> MergeBatcherEvenOdd(const std::vector<double>& left, const std::vector<double>& right) {
  const size_t total_size = left.size() + right.size();
  std::vector<double> left_even;
  std::vector<double> left_odd;
  std::vector<double> right_even;
  std::vector<double> right_odd;
  left_even.reserve((left.size() + 1) / 2);
  left_odd.reserve(left.size() / 2);
  right_even.reserve((right.size() + 1) / 2);
  right_odd.reserve(right.size() / 2);

  for (size_t i = 0; i < left.size(); ++i) {
    if ((i % 2) == 0) {
      left_even.push_back(left[i]);
    } else {
      left_odd.push_back(left[i]);
    }
  }
  for (size_t i = 0; i < right.size(); ++i) {
    if (((left.size() + i) % 2) == 0) {
      right_even.push_back(right[i]);
    } else {
      right_odd.push_back(right[i]);
    }
  }

  std::vector<double> merged_even;
  std::vector<double> merged_odd;
  merged_even.reserve(left_even.size() + right_even.size());
  merged_odd.reserve(left_odd.size() + right_odd.size());

  std::merge(left_even.begin(), left_even.end(), right_even.begin(), right_even.end(), std::back_inserter(merged_even));
  std::merge(left_odd.begin(), left_odd.end(), right_odd.begin(), right_odd.end(), std::back_inserter(merged_odd));

  std::vector<double> result(total_size);
  size_t even_idx = 0;
  size_t odd_idx = 0;
  for (size_t i = 0; i < total_size; ++i) {
    if ((i % 2) == 0) {
      result[i] = merged_even[even_idx++];
    } else {
      result[i] = merged_odd[odd_idx++];
    }
  }

  for (size_t phase = 0; phase < total_size; ++phase) {
    const size_t start = phase % 2;
    for (size_t i = start; i + 1 < total_size; i += 2) {
      if (result[i] > result[i + 1]) {
        std::swap(result[i], result[i + 1]);
      }
    }
  }

  return result;
}

}  // namespace

namespace gusev_d_double_sort_even_odd_batcher_task_threads {

DoubleSortEvenOddBatcherSEQ::DoubleSortEvenOddBatcherSEQ(const InType& in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = {};
}

bool DoubleSortEvenOddBatcherSEQ::ValidationImpl() {
  return GetOutput().empty();
}

bool DoubleSortEvenOddBatcherSEQ::PreProcessingImpl() {
  input_data_ = GetInput();
  result_data_.clear();
  return true;
}

bool DoubleSortEvenOddBatcherSEQ::RunImpl() {
  if (input_data_.empty()) {
    result_data_.clear();
    return true;
  }

  const size_t mid = input_data_.size() / 2;
  std::vector<double> left(input_data_.begin(), input_data_.begin() + static_cast<std::ptrdiff_t>(mid));
  std::vector<double> right(input_data_.begin() + static_cast<std::ptrdiff_t>(mid), input_data_.end());

  RadixSortDoubles(left);
  RadixSortDoubles(right);
  result_data_ = MergeBatcherEvenOdd(left, right);

  return true;
}

bool DoubleSortEvenOddBatcherSEQ::PostProcessingImpl() {
  GetOutput() = result_data_;
  return true;
}

}  // namespace gusev_d_double_sort_even_odd_batcher_task_threads
