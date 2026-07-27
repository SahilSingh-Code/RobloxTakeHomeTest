//------------------------------------------------------------------------------
// This contains the implementations of the algorithms needed for the Roblox
// Take Home Test. See the header file for a description of what each is trying
// to do.
//
// Author: Sundeep Singh
// Date: July 26, 2026
//------------------------------------------------------------------------------

#include "Algorithms.h"

#include <algorithm>  // for std::nth_element
#include <limits>     // for std::numeric_limits
#include <stdexcept>  // for std::invalid_argument, std::overflow_error
#include <vector>     // for std::vector

namespace Roblox {

uint64_t naiveNumberGeneration(uint64_t n) {
  if (n == 0) {
    throw std::invalid_argument("n must be at least 1");
  }

  // A counter to keep track of how many numbers have been found in the sequence
  // so far
  uint64_t count = 0;
  uint64_t currentNumber = 1;

  while (count < n) {
    // Check if the current number is valid
    uint64_t temp = currentNumber;

    // Repeatedly divide by 2, 3, and 5 until we can't anymore. If we end up
    // with 1, then the number is valid.
    // Integer division here should be fine as we are explicitly checking that
    // there is no remainder before dividing.
    while ((temp % 2) == 0) {
      temp /= 2;
    }

    while ((temp % 3) == 0) {
      temp /= 3;
    }

    while ((temp % 5) == 0) {
      temp /= 5;
    }

    if (temp == 1) {
      // The number is valid, so increment the count
      count++;

      // Once we reach the desired count, we can return the current number
      if (count == n) {
        return currentNumber;
      }
    }

    // Not that we would ever use this algorithm for such a large number, but we
    // should check for overflow, and throw an exception if we reach the maximum
    // value of uint64_t
    if (currentNumber == std::numeric_limits<uint64_t>::max()) {
      throw std::overflow_error(
          "Result cannot be found using uint64_t enumeration");
    }

    currentNumber++;
  };

  // We shouldn't reach here, but just in case, return 0 to indicate an error
  return 0;

}  // end of naiveNumberGeneration

Triplet optimizedNumberGeneration(uint64_t n) {
  if (n == 0) {
    throw std::invalid_argument("n must be at least 1");
  }

  uint64_t alphaMax = 0;
  uint64_t betaMax = 0;
  uint64_t gammaMax = 0;
  std::vector<Triplet> triplets;
  triplets.emplace_back(
      0, 0,
      0);  // This initialization is necessary as we will skip
           // the 0,0,0 triplet in the main analysis loop below

  //------------------------------------------------------------------------------
  // Converge criteria lambda
  // We check and see if we have generated enough numbers, and if the smallest
  // possible missed value is larger than the value we currently have in the nth
  // position. The nth position number must be in the correct sorted location
  //------------------------------------------------------------------------------
  auto isConverged = [&]() {
    // We must have at least n entries to find the nth number, so if we don't
    // have enough, we are not converged.
    if (triplets.size() < n) {
      return false;
    }

    // Check if the current triplet is greater than or equal to the nth triplet
    const Triplet& nthTriplet = triplets[n - 1];
    const Triplet smallestMissed =
        std::min({Triplet(alphaMax + 1, 0, 0), Triplet(0, betaMax + 1, 0),
                  Triplet(0, 0, gammaMax + 1)});

    return nthTriplet < smallestMissed;
  };

  //------------------------------------------------------------------------------
  // Lambdas to vary only two of the three values, keeping the third at its max
  // value
  //------------------------------------------------------------------------------
  auto fixAlphaMaxAndVaryOthers = [&](uint64_t a) {
    for (uint64_t j = 0; j <= betaMax; j++) {
      for (uint64_t k = 0; k <= gammaMax; k++) {
        triplets.emplace_back(a, j, k);
      }
    }
  };

  auto fixBetaMaxAndVaryOthers = [&](uint64_t b) {
    for (uint64_t i = 0; i <= alphaMax; i++) {
      for (uint64_t k = 0; k <= gammaMax; k++) {
        triplets.emplace_back(i, b, k);
      }
    }
  };

  auto fixGammaMaxAndVaryOthers = [&](uint64_t c) {
    for (uint64_t i = 0; i <= alphaMax; i++) {
      for (uint64_t j = 0; j <= betaMax; j++) {
        triplets.emplace_back(i, j, c);
      }
    }
  };

  // Incremently generate triplets and add them to the vector until we have
  // enough to guarantee that the nth number is correct.
  while (true) {
    if (isConverged()) {
      return triplets[n - 1];
    }

    // Determine incrementing which of the three maxes will generate the most
    // granular step, then increment it and generate the varied addition to the
    // dataset
    Triplet alphaVariation = Triplet(alphaMax + 1, 0, 0);
    Triplet betaVariation = Triplet(0, betaMax + 1, 0);
    Triplet gammaVariation = Triplet(0, 0, gammaMax + 1);

    if ((alphaVariation < betaVariation) && (alphaVariation < gammaVariation)) {
      alphaMax++;
      fixAlphaMaxAndVaryOthers(alphaMax);
    } else if (betaVariation < gammaVariation) {
      betaMax++;
      fixBetaMaxAndVaryOthers(betaMax);
    } else {
      gammaMax++;
      fixGammaMaxAndVaryOthers(gammaMax);
    }

    // Sort the nth entry into the correct position
    if (triplets.size() >= n) {
      std::nth_element(triplets.begin(), triplets.begin() + n - 1,
                       triplets.end());
    }
  }

  // We shouldn't be able to get here, so return something to signal error
  return Triplet(0, 0, 0);

}  // end of optimizedNumberGeneration

Triplet standardNumberGeneration(uint64_t n) {
  if (n == 0) {
    throw std::invalid_argument("n must be at least 1");
  }

  std::vector<Triplet> uglyNumbers;
  uglyNumbers.reserve(n);
  uglyNumbers.emplace_back(0, 0,
                           0);  // initializing 1 is necessary as this serves as
                                // the base of the number generation

  // Each pointer tracks the first unused entry in one multiplication stream.
  uint64_t index2 = 0;
  uint64_t index3 = 0;
  uint64_t index5 = 0;

  // Generate the ugly numbers
  // This loop only needs to go exactly to n as the numbers being generated are
  // guaranteed to be sorted and complete
  while (uglyNumbers.size() < n) {
    // Generate the next number in each multiplication stream.
    const Triplet& base2 = uglyNumbers[index2];
    const Triplet& base3 = uglyNumbers[index3];
    const Triplet& base5 = uglyNumbers[index5];

    const Triplet nextFrom2(base2.alpha + 1, base2.beta, base2.gamma);
    const Triplet nextFrom3(base3.alpha, base3.beta + 1, base3.gamma);
    const Triplet nextFrom5(base5.alpha, base5.beta, base5.gamma + 1);

    // Select the smallest current stream value.
    const Triplet nextNumber = std::min({nextFrom2, nextFrom3, nextFrom5});
    uglyNumbers.push_back(nextNumber);

    // Advance every stream that generated the chosen number.
    // Duplicates are possible here, so we must check all three streams.
    if (nextFrom2 == nextNumber) {
      ++index2;
    }

    if (nextFrom3 == nextNumber) {
      ++index3;
    }

    if (nextFrom5 == nextNumber) {
      ++index5;
    }
  }

  return uglyNumbers[n - 1];
}  // end of standardNumberGeneration

Triplet predictiveNumberGeneration(uint64_t n) {
  if (n == 0) {
    throw std::invalid_argument("n must be at least 1");
  }

  // Pr-define these as constants so that calculations don't default to regular
  // double precision. Use long double where available to reduce floating-point
  // error in the logarithmic boundary calculations.
  const long double log2 = std::log(2.0L);
  const long double log3 = std::log(3.0L);
  const long double log5 = std::log(5.0L);

  // Count all triplets satisfying alpha*log(2) + beta*log(3) + gamma*log(5) <=
  // limit
  auto countAtOrBelow = [&](long double limit) {
    uint64_t count = 0;

    // Iterative over every combination of allowable powers of 2, 3, and 5, in
    // descending order (see dev log for explanation why). This is a brute-force
    // approach, but the number of powers here should be small enough. Besides,
    // the alternative is to generate the entire sequence up to the nth value.
    for (uint64_t gamma = 0;; ++gamma) {
      const long double remainingAfterFive =
          limit - static_cast<long double>(gamma) * log5;

      // If we took too many values off, then we are done with this loop and all
      // subsequent loops, no more powers of 5 allowed
      if (remainingAfterFive < 0.0L) {
        break;
      }

      // Given the current remaining value in the log, iterate over beta and see
      // how many powers of 3 we can subtract off
      for (uint64_t beta = 0;; ++beta) {
        const long double remaining =
            remainingAfterFive - static_cast<long double>(beta) * log3;

        // If we counted too many powers of 3, then no more 3s are allowed, and
        // we can break out of this loop and go to the next power of 5
        if (remaining < 0.0L) {
          break;
        }

        // Once we reach just one number, counting how many exponents is easy
        // For example, if we want to know how many values of 2^\alpha are less
        // than or equal to 100, we can take the log base 2 of 100 and round
        // down to get the maximum alpha value. Then, all values of alpha from 0
        // to that maximum are valid.
        const auto alphaMax =
            static_cast<uint64_t>(std::floor(remaining / log2));

        // Valid alpha values are 0 through alphaMax.
        count += alphaMax + 1;
      }
    }

    return count;
  };

  // Estimate the logarithmic magnitude of the nth value using the
  // volume of the exponent-space tetrahedron.
  const long double estimate =
      std::cbrt(6.0L * static_cast<long double>(n) * log2 * log3 * log5) -
      0.5L * (log2 + log3 + log5);

  // Set some arbitrary boundaries around the estimate
  // It's ok if this isn't accurate, and if the actual nth value is outside of
  // this range, as we will expand the boundaries until we find the correct
  // range
  long double lower = estimate - 1.0L;
  long double upper = estimate + 1.0L;

  uint64_t lowerCount = countAtOrBelow(lower);
  uint64_t upperCount = countAtOrBelow(upper);

  // Slide the boundary down and expand it by powers of 2 until we find a lower
  // boundary that has fewer than n values below it
  long double step = 1.0L;
  while (lowerCount >= n) {
    upper = lower;
    upperCount = lowerCount;

    step *= 2.0L;
    lower -= step;
    lowerCount = countAtOrBelow(lower);
  }

  // Slide the boundary up and expand it by powers of 2 until we find an upper
  // boundary that has more than n values below it
  step = 1.0L;
  while (upperCount < n) {
    lower = upper;
    lowerCount = upperCount;

    step *= 2.0L;
    upper += step;
    upperCount = countAtOrBelow(upper);
  }

  // Narrow the interval until only a manageable number of sequence
  // entries lie between the two boundaries.
  // This shell size is arbitrary, but it should be small enough to avoid
  // excessive memory usage, and large enough to avoid excessive iterations of
  // the loop. This could be optimized in a hyperparameter search style manner,
  // but for now, this is a reasonable value.
  const uint64_t maximumShellSize = 100000;

  // Iteratively narrow down the shell size and zero in on the actual value of
  // the nth number in the sequence.
  while (upperCount - lowerCount > maximumShellSize) {
    // Count how many values are below the midpoint of the current boundaries
    const long double middle = lower + 0.5L * (upper - lower);
    const uint64_t middleCount = countAtOrBelow(middle);

    // Close the boundary in towards the midpoint , depending on whether the
    // midpoint has too many or too few values below it
    if (middleCount < n) {
      lower = middle;
      lowerCount = middleCount;
    } else {
      upper = middle;
      upperCount = middleCount;
    }
  }

  // Generate values in the thin shell between the two boundaries, and store
  // them in a vector. This is a brute-force approach similar to what we did
  // earlier for counting, but this time we will store the values in a vector so
  // that we can sort them and find the nth value.
  std::vector<Triplet> shell;
  shell.reserve(static_cast<std::size_t>(upperCount - lowerCount));

  for (uint64_t gamma = 0;; ++gamma) {
    const long double gammaLog = static_cast<long double>(gamma) * log5;

    // If this value of gamma is too big, no more powers of 5 are allowed
    if (gammaLog > upper) {
      break;
    }

    for (uint64_t beta = 0;; ++beta) {
      // Calculate the log value of the base number for this combination of beta
      // and gamma. This is the log value of 3^beta * 5^gamma, which is the base
      // for all alpha values. We will then calculate how many alpha values are
      // allowed for this base, and add them to the shell.
      const long double baseLog =
          gammaLog + static_cast<long double>(beta) * log3;

      // If this power of 3 makes the total product too big, then no more powers
      // of 3 are allowed for this value of gamma
      if (baseLog > upper) {
        break;
      }

      // For the inequality lower < alpha*log(2) + baseLog <= upper, we can
      // solve for first and last alpha values that satisfy this inequality
      // Unlike the powers of 3 and 5, we aren't iterating over the values.
      // This means that we might be overestimating how many powers of 2. If
      // this happens, it will manifest as a negative value for firstAlpha,
      // which we will correct to 0. We also add a +1 to firstAlpha to ensure
      // that we are strictly greater than the lower bound
      int64_t firstAlpha =
          static_cast<int64_t>(std::floor((lower - baseLog) / log2)) + 1;
      firstAlpha = std::max<int64_t>(firstAlpha, 0);

      // The last alpha value is the largest integer that satisfies the
      // inequality.
      // We do not need to do a negative check as we already compared baseLog >
      // upper earlier. We also do not need to add 1 as this is already less
      // than or equal to the upper bound.
      const int64_t lastAlpha =
          static_cast<int64_t>(std::floor((upper - baseLog) / log2));

      // Place all the possible values into the shell, unsorted
      for (std::int64_t alpha = firstAlpha; alpha <= lastAlpha; ++alpha) {
        shell.emplace_back(static_cast<std::uint64_t>(alpha), beta, gamma);
      }
    }
  }

  // The shell should contain all the values between the two boundaries, and we
  // want to index it as such. If we don't convert to a local index, we would be
  // indexing into the shell with a value that is larger than the size of the
  // shell
  const std::size_t localIndex = static_cast<std::size_t>(n - lowerCount - 1);

  if (localIndex >= shell.size()) {
    throw std::runtime_error(
        "Floating-point boundary calculation was inconsistent");
  }

  // Sort only the nth position value into the correct position, don't use a
  // full sort
  auto nthPosition = shell.begin() + localIndex;
  std::nth_element(shell.begin(), nthPosition, shell.end());
  return *nthPosition;

}  // end of predictiveNumberGeneration

}  // end of namespace Roblox
