//-------------------------------------------------------------------------------
// This file serves as a simple test and timing harness for the algorithms
// implemented in Algorithms.cpp.
//
// It performs:
// 1. Correctness tests for the first 20 sequence entries.
// 2. Correctness tests for the 100,000th entry.
// 3. Consistency tests over 50 positions from 100 to 100,000.
// 4. Timing measurements written to algorithm_timings.csv
//
// If I had time, and we were allowed to use external libraries, I would have
// used a unit testing framework like Catch2 to make the tests better
// structured. For now, I just wrote a simple test struct to keep track of the
// number of tests passed and failed, and print out the results at the end.
//
// The test cases here and what is being tested are my choice, but AI was used
// to help expedite the coding.
//
// Author: Sundeep Singh
// Date: July 26, 2026
//-------------------------------------------------------------------------------

#include <array>      // for std::array
#include <chrono>     // for std::chrono
#include <cmath>      // for std::log, std::exp, std::floor, std::ceil
#include <cstdint>    // for uint64_t
#include <fstream>    // for std::ofstream
#include <iomanip>    // for std::setw, std::setprecision
#include <iostream>   // for std::cout, std::endl
#include <limits>     // for std::numeric_limits
#include <stdexcept>  // for std::invalid_argument, std::overflow_error
#include <string>     // for std::string
#include <vector>     // for std::vector

#include "Algorithms.h"

namespace {

//-------------------------------------------------------------------------------
// Test statistics
//-------------------------------------------------------------------------------

// A struct to contain the basic information about how many tests have passed
// and failed, and the ability to print a failed message to the console without
// stopping all the other tests
struct TestStats {
  uint64_t passed = 0;
  uint64_t total = 0;

  void check(bool condition, const std::string& description) {
    ++total;

    if (condition) {
      ++passed;
    } else {
      std::cout << "FAILED: " << description << std::endl;
    }
  }
};

//-------------------------------------------------------------------------------
// Timed result
//-------------------------------------------------------------------------------

// Store the results and timing
// This is templated so that the algorithms that return triplets and a uint64_t
// can both be stored in this
template <typename Result>
struct TimedResult {
  Result result;
  uint64_t timeNanoseconds;
};

// A utility function to time any block of code , and return the result and the
// time taken in nanoseconds
template <typename Function>
auto timeFunction(Function function) {
  const auto startTime = std::chrono::steady_clock::now();
  auto result = function();
  const auto endTime = std::chrono::steady_clock::now();

  const auto elapsedTime =
      std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime)
          .count();

  return TimedResult<decltype(result)>{result,
                                       static_cast<uint64_t>(elapsedTime)};
}

//-------------------------------------------------------------------------------
// Triplet helpers
//-------------------------------------------------------------------------------

// We don't want to overflow when multiplying, so let's make sure that we check
// the result of the multiplication first
uint64_t checkedMultiply(uint64_t value, uint64_t multiplier) {
  if (value > std::numeric_limits<uint64_t>::max() / multiplier) {
    throw std::overflow_error("Triplet value does not fit in uint64_t");
  }

  return value * multiplier;
}

uint64_t calculateTripletValue(const Roblox::Triplet& triplet) {
  uint64_t value = 1;

  for (uint64_t i = 0; i < triplet.alpha; ++i) {
    value = checkedMultiply(value, 2);
  }

  for (uint64_t i = 0; i < triplet.beta; ++i) {
    value = checkedMultiply(value, 3);
  }

  for (uint64_t i = 0; i < triplet.gamma; ++i) {
    value = checkedMultiply(value, 5);
  }

  return value;
}

//-------------------------------------------------------------------------------
// Generate approximately logarithmically spaced test positions.
//
// Logarithmic spacing gives us more coverage at small values while still
// reaching the full 100,000-entry test.
//
// This is a good benchmark for correctness over the normal range of 100k that
// we can actually do cross-comparisoons over, and it's good for plotting.
//-------------------------------------------------------------------------------

std::vector<uint64_t> generateTestPositions(uint64_t minimum, uint64_t maximum,
                                            uint64_t numberOfPoints) {
  std::vector<uint64_t> positions;
  positions.reserve(numberOfPoints);

  const double ratio = std::pow(static_cast<double>(maximum) / minimum,
                                1.0 / static_cast<double>(numberOfPoints - 1));

  for (uint64_t i = 0; i < numberOfPoints; ++i) {
    const uint64_t position =
        static_cast<uint64_t>(std::round(minimum * std::pow(ratio, i)));

    positions.push_back(position);
  }

  // Avoid a possible rounding error at the final point.
  positions.back() = maximum;

  return positions;
}

//-------------------------------------------------------------------------------
// Printing helpers
//-------------------------------------------------------------------------------

void printTime(const std::string& algorithm, uint64_t timeNanoseconds) {
  const long double timeMilliseconds =
      static_cast<long double>(timeNanoseconds) / 1000000.0L;

  std::cout << std::left << std::setw(24) << algorithm << std::right
            << std::setw(14) << timeNanoseconds << " ns"
            << "  (" << std::fixed << std::setprecision(3) << timeMilliseconds
            << " ms)" << std::endl;
}

long double calculateTimeRatio(uint64_t numerator, uint64_t denominator) {
  if (denominator == 0) {
    return 0.0L;
  }

  return static_cast<long double>(numerator) /
         static_cast<long double>(denominator);
}

void writeTimingRow(std::ofstream& output, uint64_t n,
                    const std::string& algorithm, const Roblox::Triplet& result,
                    uint64_t timeNanoseconds, bool consistent) {
  output << n << ',' << algorithm << ',' << timeNanoseconds << ','
         << result.alpha << ',' << result.beta << ',' << result.gamma << ','
         << (consistent ? 1 : 0) << std::endl;
}

}  // namespace

int main() {
  TestStats testStats;

  //-----------------------------------------------------------------------------
  // Correctness and timing tests for the first 20 entries
  //-----------------------------------------------------------------------------

  {
    const std::array<uint64_t, 20> expectedValues{
        1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 15, 16, 18, 20, 24, 25, 27, 30, 32, 36};

    uint64_t naiveTotalTime = 0;
    uint64_t optimizedTotalTime = 0;
    uint64_t standardTotalTime = 0;
    uint64_t predictiveTotalTime = 0;

    for (uint64_t i = 0; i < expectedValues.size(); ++i) {
      const uint64_t n = i + 1;
      const uint64_t expectedValue = expectedValues[i];

      const auto naiveResult =
          timeFunction([n]() { return Roblox::naiveNumberGeneration(n); });

      const auto optimizedResult =
          timeFunction([n]() { return Roblox::optimizedNumberGeneration(n); });

      const auto standardResult =
          timeFunction([n]() { return Roblox::standardNumberGeneration(n); });

      const auto predictiveResult =
          timeFunction([n]() { return Roblox::predictiveNumberGeneration(n); });

      naiveTotalTime += naiveResult.timeNanoseconds;
      optimizedTotalTime += optimizedResult.timeNanoseconds;
      standardTotalTime += standardResult.timeNanoseconds;
      predictiveTotalTime += predictiveResult.timeNanoseconds;

      testStats.check(naiveResult.result == expectedValue,
                      "Naive algorithm at n = " + std::to_string(n));

      testStats.check(
          calculateTripletValue(optimizedResult.result) == expectedValue,
          "Optimized algorithm at n = " + std::to_string(n));

      testStats.check(
          calculateTripletValue(standardResult.result) == expectedValue,
          "Standard algorithm at n = " + std::to_string(n));

      testStats.check(
          calculateTripletValue(predictiveResult.result) == expectedValue,
          "Predictive algorithm at n = " + std::to_string(n));
    }

    std::cout << std::endl << "--- First 20 entries ---" << std::endl;

    printTime("Naive", naiveTotalTime);
    printTime("Optimized exponent box", optimizedTotalTime);
    printTime("Standard three pointer", standardTotalTime);
    printTime("Predictive", predictiveTotalTime);

    std::cout << std::endl
              << "Timing relative to the standard algorithm:" << std::endl
              << "Naive: "
              << calculateTimeRatio(naiveTotalTime, standardTotalTime) << "x"
              << std::endl
              << "Optimized exponent box: "
              << calculateTimeRatio(optimizedTotalTime, standardTotalTime)
              << "x" << std::endl
              << "Predictive: "
              << calculateTimeRatio(predictiveTotalTime, standardTotalTime)
              << "x" << std::endl;
  }

  //-----------------------------------------------------------------------------
  // Correctness test for the 100,000th entry
  //-----------------------------------------------------------------------------

  {
    const uint64_t n = 100000;
    const Roblox::Triplet expectedResult(96, 1, 13);

    const auto optimizedResult = timeFunction(
        []() { return Roblox::optimizedNumberGeneration(100000); });

    const auto standardResult =
        timeFunction([]() { return Roblox::standardNumberGeneration(100000); });

    const auto predictiveResult = timeFunction(
        []() { return Roblox::predictiveNumberGeneration(100000); });

    testStats.check((optimizedResult.result == expectedResult),
                    "Optimized algorithm at n = 100,000");

    testStats.check((standardResult.result == expectedResult),
                    "Standard algorithm at n = 100,000");

    testStats.check((predictiveResult.result == expectedResult),
                    "Predictive algorithm at n = 100,000");

    testStats.check((optimizedResult.result == standardResult.result),
                    "Optimized and standard algorithms agree at n = 100,000");

    testStats.check((predictiveResult.result == standardResult.result),
                    "Predictive and standard algorithms agree at n = 100,000");

    std::cout << std::endl
              << "--- 100,000th entry ---" << std::endl
              << "Expected result: (" << expectedResult.alpha << ", "
              << expectedResult.beta << ", " << expectedResult.gamma << ")"
              << std::endl
              << std::endl;

    printTime("Optimized exponent box", optimizedResult.timeNanoseconds);

    printTime("Standard three pointer", standardResult.timeNanoseconds);

    printTime("Predictive", predictiveResult.timeNanoseconds);
  }

  //-----------------------------------------------------------------------------
  // Consistency and timing tests from 100 to 100,000
  //-----------------------------------------------------------------------------

  {
    const uint64_t numberOfTestPoints = 50;

    const std::vector<uint64_t> testPositions =
        generateTestPositions(100, 100000, numberOfTestPoints);

    std::ofstream outputFile("algorithm_timings.csv");

    testStats.check(outputFile.is_open(), "Open algorithm_timings.csv");

    if (outputFile.is_open()) {
      outputFile << "n,algorithm,time_ns,alpha,beta,gamma,consistent"
                 << std::endl;

      std::cout << std::endl
                << "--- Distributed consistency tests ---" << std::endl
                << "Writing results to algorithm_timings.csv" << std::endl
                << std::endl;

      for (uint64_t i = 0; i < testPositions.size(); ++i) {
        const uint64_t n = testPositions[i];

        const auto optimizedResult = timeFunction(
            [n]() { return Roblox::optimizedNumberGeneration(n); });

        const auto standardResult =
            timeFunction([n]() { return Roblox::standardNumberGeneration(n); });

        const auto predictiveResult = timeFunction(
            [n]() { return Roblox::predictiveNumberGeneration(n); });

        const bool optimizedMatches =
            (optimizedResult.result == standardResult.result);

        const bool predictiveMatches =
            (predictiveResult.result == standardResult.result);

        const bool allAlgorithmsMatch = optimizedMatches && predictiveMatches;

        testStats.check(allAlgorithmsMatch,
                        "Algorithm consistency at n = " + std::to_string(n));

        writeTimingRow(outputFile, n, "optimized", optimizedResult.result,
                       optimizedResult.timeNanoseconds, allAlgorithmsMatch);

        writeTimingRow(outputFile, n, "standard", standardResult.result,
                       standardResult.timeNanoseconds, allAlgorithmsMatch);

        writeTimingRow(outputFile, n, "predictive", predictiveResult.result,
                       predictiveResult.timeNanoseconds, allAlgorithmsMatch);

        std::cout << "Test " << std::setw(2) << i + 1 << " / "
                  << testPositions.size() << ", n = " << std::setw(6) << n
                  << ", consistent = " << (allAlgorithmsMatch ? "yes" : "no")
                  << std::endl;
      }
    }
  }

  //-----------------------------------------------------------------------------
  // Predict the 4 000 000 000th entry
  //-----------------------------------------------------------------------------

  {
    const uint64_t n = 4000000000;

    const Roblox::Triplet predictedResult =
        Roblox::predictiveNumberGeneration(n);

    std::cout << std::endl
              << "--- 4, 000, 000, 000th entry ---" << std::endl
              << "Predicted result: (" << predictedResult.alpha << ", "
              << predictedResult.beta << ", " << predictedResult.gamma << ")"
              << std::endl;
  }

  //-----------------------------------------------------------------------------
  // Final test results
  //-----------------------------------------------------------------------------

  {
    const long double percentagePassed =
        testStats.total == 0
            ? 100.0L
            : 100.0L * static_cast<long double>(testStats.passed) /
                  static_cast<long double>(testStats.total);

    std::cout << std::endl
              << "--- Test summary ---" << std::endl
              << "Passed " << testStats.passed << " of " << testStats.total
              << " tests." << std::endl
              << std::fixed << std::setprecision(2) << percentagePassed
              << "% of tests passed." << std::endl;

    return testStats.passed == testStats.total ? 0 : 1;
  }
}
