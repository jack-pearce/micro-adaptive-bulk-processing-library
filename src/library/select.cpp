#include <algorithm>
#include <iostream>
#include <immintrin.h>

#include "select.h"
#include "papi.h"


int selectIndexesBranch(int n, const int *inputData, int *selection, int threshold) {
    int k = 0;
    for (int i = 0; i < n; ++i) {
        if (inputData[i] <= threshold) {
            selection[k++] = i;
        }
    }
    return k;
}

int selectIndexesPredication(int n, const int *inputData, int *selection, int threshold) {
    int k = 0;
    for (int i = 0; i < n; ++i) {
        selection[k] = i;
        k += (inputData[i] <= threshold);
    }
    return k;
}

inline int runSelectIndexesChunk(int (*&selectFunctionPtr)(int, const int *, int *, int),
                                 int tuplesToProcess,
                                 int &n,
                                 const int *&inputData,
                                 int *&selection,
                                 int threshold,
                                 int &k,
                                 int &consecutivePredications) {
    Counters::getInstance().readEventSet();
    int selected = selectFunctionPtr(tuplesToProcess, inputData, selection, threshold);
    Counters::getInstance().readEventSet();
    n -= tuplesToProcess;
    inputData += tuplesToProcess;
    selection += selected;
    k += selected;
    consecutivePredications += (selectFunctionPtr == selectIndexesPredication);
    return selected;
}

inline void performSelectIndexesAdaption(int (*&selectFunctionPtr)(int, const int *, int *, int),
                                         const long_long *counterValues,
                                         float lowerCrossoverSelectivity,
                                         float upperCrossoverSelectivity,
                                         float lowerBranchCrossoverBranchMisses,
                                         float m,
                                         float selectivity,
                                         int &consecutivePredications) {

    if (__builtin_expect(static_cast<float>(counterValues[0]) >
                         (((selectivity - lowerCrossoverSelectivity) * m) + lowerBranchCrossoverBranchMisses)
                         && selectFunctionPtr == selectIndexesBranch, false)) {
//        std::cout << "Switched to select predication" << std::endl;
        selectFunctionPtr = selectIndexesPredication;
    }

    if (__builtin_expect((selectivity < lowerCrossoverSelectivity
                         || selectivity > upperCrossoverSelectivity)
                         && selectFunctionPtr == selectIndexesPredication, false)) {
//        std::cout << "Switched to select branch" << std::endl;
        selectFunctionPtr = selectIndexesBranch;
        consecutivePredications = 0;
    }
}

int selectIndexesAdaptive(int n, const int *inputData, int *selection, int threshold) {
    int tuplesPerAdaption = 50000;
    int maxConsecutivePredications = 10;
    int tuplesInBranchBurst = 1000;

    float lowerCrossoverSelectivity = 0.03; // Could use a tuning function to identify these cross-over points
    float upperCrossoverSelectivity = 0.98; // Could use a tuning function to identify these cross-over points

    // Equations below are only valid at the extreme ends of selectivity
    // Y intercept of number of branch misses (at lower cross-over selectivity)
    float lowerBranchCrossoverBranchMisses = lowerCrossoverSelectivity * static_cast<float>(tuplesPerAdaption);
    float upperBranchCrossoverBranchMisses = (1 - upperCrossoverSelectivity) * static_cast<float>(tuplesPerAdaption);

    // Gradient of number of branch misses between lower cross-over selectivity and upper cross-over selectivity
    float m = (upperBranchCrossoverBranchMisses - lowerBranchCrossoverBranchMisses) /
            (upperCrossoverSelectivity - lowerCrossoverSelectivity);

    // Modified values for short branch burst chunks
    float lowerBranchCrossoverBranchMisses_BranchBurst = lowerCrossoverSelectivity * static_cast<float>(tuplesInBranchBurst);
    float upperBranchCrossoverBranchMisses_BranchBurst = (1 - upperCrossoverSelectivity) * static_cast<float>(tuplesInBranchBurst);
    float m_BranchBurst = (upperBranchCrossoverBranchMisses_BranchBurst - lowerBranchCrossoverBranchMisses_BranchBurst) /
              (upperCrossoverSelectivity - lowerCrossoverSelectivity);

    int k = 0;
    int consecutivePredications = 0;
    int tuplesToProcess;
    int selected;
    SelectFunctionPtr selectFunctionPtr = selectIndexesPredication;

    std::vector<std::string> counters = {"PERF_COUNT_HW_BRANCH_MISSES"};
    long_long *counterValues = Counters::getInstance().getEvents(counters);

    while (n > 0) {
        if (__builtin_expect(consecutivePredications == maxConsecutivePredications, false)) {
//            std::cout << "Running branch burst" << std::endl;
            selectFunctionPtr = selectIndexesBranch;
            consecutivePredications = 0;

            selected = runSelectIndexesChunk(selectFunctionPtr, tuplesInBranchBurst, n,
                                             inputData, selection, threshold, k, consecutivePredications);
            performSelectIndexesAdaption(selectFunctionPtr, counterValues, lowerCrossoverSelectivity,
                                         upperCrossoverSelectivity,
                                         lowerBranchCrossoverBranchMisses_BranchBurst,
                                         m_BranchBurst,
                                         static_cast<float>(selected) / static_cast<float>(tuplesInBranchBurst),
                                         consecutivePredications);
        } else {
            tuplesToProcess = std::min(n, tuplesPerAdaption);
            selected = runSelectIndexesChunk(selectFunctionPtr, tuplesToProcess, n, inputData, selection,
                                             threshold, k, consecutivePredications);
            performSelectIndexesAdaption(selectFunctionPtr, counterValues, lowerCrossoverSelectivity,
                                         upperCrossoverSelectivity,
                                         lowerBranchCrossoverBranchMisses, m,
                                         static_cast<float>(selected) / static_cast<float>(tuplesPerAdaption),
                                         consecutivePredications);
        }
    }

    return k;
}

int selectValuesBranch(int n, const int *inputData, int *selection, int threshold) {
    int k = 0;
    for (int i = 0; i < n; ++i) {
        if (inputData[i] <= threshold) {
            selection[k++] = inputData[i];
        }
    }
    return k;
}

int selectValuesPredication(int n, const int *inputData, const int *inputFilter, int *selection, int threshold) {
    int k = 0;
    for (int i = 0; i < n; ++i) {
        selection[k] = inputData[i];
//        selection[k] = inputData[(inputFilter[i] <= threshold) * i] * (inputFilter[i] <= threshold);
        k += (inputFilter[i] <= threshold);
    }
    return k;
}

bool isSimdAligned(const int* array) {
    const size_t simdAlignment = sizeof(__m128i);
    return reinterpret_cast<uintptr_t>(array) % simdAlignment == 0;
}

int selectValuesVectorized(int n, const int *inputData, const int *inputFilter, int *selection, int threshold) {
    int k = 0;

    // Process unaligned tuples
    int unalignedCount = 0;
    for (int i = 0; !isSimdAligned(inputData + i); ++i) {
        selection[k] = inputData[i];
        k += (inputFilter[i] <= threshold);
        ++unalignedCount;
    }

    // Vectorize the loop for aligned tuples
    int simdWidth = sizeof(__m128i) / sizeof(int);
    int simdIterations = (n - unalignedCount) / simdWidth;
    __m128i thresholdVector = _mm_set1_epi32(threshold);

    for (int i = unalignedCount; i < unalignedCount + (simdIterations * simdWidth); i += simdWidth) {
        __m128i filterVector = _mm_load_si128((__m128i *)(inputFilter + i));

        // Compare filterVector <= thresholdVector
        __mmask8 mask = _mm_cmpgt_epi32_mask(filterVector, thresholdVector);
        mask = ~mask; // Combine with line above

        if(__builtin_popcount(mask)) {   // Does this help?
            for (int j = 0; j < simdWidth; ++j) {
                if (mask & (1 << j)) {
                    selection[k++] = inputData[i + j];    // Try branch and branch free versions
                }
            }
        }
    }

    // Process the remaining elements (if any)
    for (int i = unalignedCount + simdIterations * simdWidth; i < n; ++i) {
        selection[k] = inputData[i];
        k += (inputFilter[i] <= threshold);
    }

    return k;
}

int selectValuesAdaptive(int n, const int *inputData, int *selection, int threshold) {
    int k = 0;
    // to implement
    return k;
}

void setSelectFuncPtr(SelectFunctionPtr &selectFunctionPtr, SelectImplementation selectImplementation) {
    switch(selectImplementation) {
        case SelectImplementation::IndexesBranch:
            selectFunctionPtr = selectIndexesBranch;
            break;
        case SelectImplementation::IndexesPredication:
            selectFunctionPtr = selectIndexesPredication;
            break;
        case SelectImplementation::IndexesAdaptive:
            selectFunctionPtr = selectIndexesAdaptive;
            break;
        case SelectImplementation::ValuesBranch:
            selectFunctionPtr = selectValuesBranch;
            break;
//        case SelectImplementation::ValuesPredication:
//            selectFunctionPtr = selectValuesPredication;
//            break;
//        case SelectImplementation::ValuesVectorized:
//            selectFunctionPtr = selectValuesVectorized;
//            break;
        case SelectImplementation::ValuesAdaptive:
            selectFunctionPtr = selectValuesAdaptive;
            break;
        default:
            std::cout << "Invalid selection of 'Select' implementation!" << std::endl;
            exit(1);
    }
}

std::string getName(SelectImplementation selectImplementation) {
    switch(selectImplementation) {
        case SelectImplementation::IndexesBranch:
            return "Select_Indexes_Branch";
        case SelectImplementation::IndexesPredication:
            return "Select_Indexes_Predication";
        case SelectImplementation::IndexesAdaptive:
            return "Select_Indexes_Adaptive";
        case SelectImplementation::ValuesBranch:
            return "Select_Values_Branch";
        case SelectImplementation::ValuesPredication:
            return "Select_Values_Predication";
        case SelectImplementation::ValuesVectorized:
            return "Select_Values_Vectorized";
        case SelectImplementation::ValuesAdaptive:
            return "Select_Values_Adaptive";
        default:
            std::cout << "Invalid selection of 'Select' implementation!" << std::endl;
            exit(1);
    }
}