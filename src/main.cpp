#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include "time_benchmarking/timeBenchmarkSelect.h"
#include "time_benchmarking/timeBenchmarkHelpers.h"
#include "data_generation/dataGenerators.h"
#include "utils/dataHelpers.h"
#include "library/select.h"
#include "library/groupBy.h"
#include "library/papi.h"
#include "perf_counters/selectCounterBenchmark.h"
#include "perf_counters/groupByCounterBenchmark.h"
#include "data_generation/dataFiles.h"
#include "utils/papiHelpers.h"


void dataDistributionTest(const DataFile& dataFile) {
    LoadedData::getInstance(dataFile);
    std::cout << dataFile.getNumElements() << " elements" << std::endl;
    displayDistribution(dataFile);
}

void selectFunctionalityTest(const DataFile& dataFile, Select selectImplementation) {

    for (auto i = 0; i <= 100; i += 10) {
        auto inputData = new int[dataFile.getNumElements()];
        auto inputFilter = new int[dataFile.getNumElements()];
        auto selection = new int[dataFile.getNumElements()];
        copyArray(LoadedData::getInstance(dataFile).getData(), inputData, dataFile.getNumElements());
        copyArray(LoadedData::getInstance(dataFile).getData(), inputFilter, dataFile.getNumElements());

        auto selected = runSelectFunction(selectImplementation,
                                         dataFile.getNumElements(), inputData, inputFilter, selection, i);
        std::cout << i << "%: " << static_cast<float>(selected) / static_cast<float>(dataFile.getNumElements()) << std::endl;

        delete[] inputData;
        delete[] inputFilter;
        delete[] selection;
    }
}

void runSelectTimeBenchmark(const DataFile& dataFile, Select selectImplementation, int selectivityStride) {
    selectTimeBenchmark(dataFile, selectImplementation, selectivityStride);
    runTimeBenchmark(0, nullptr);
}

void runSelectTimeBenchmarkSetIterations(const DataFile& dataFile, Select selectImplementation, int selectivityStride, int iterations) {
    selectTimeBenchmarkSetIterations(dataFile, selectImplementation, selectivityStride, iterations);
    runTimeBenchmark(0, nullptr);
}

void selectBenchmarkWithExtraCountersConfigurations(const DataFile &dataFile, Select selectImplementation, int iterations) {
    // HPC set 1
    std::vector<std::string> benchmarkCounters = {"PERF_COUNT_HW_CPU_CYCLES",
                                                  "INSTRUCTION_RETIRED",
                                                  "LLC_REFERENCES",
                                                  "LLC_MISSES",
                                                  "MISPREDICTED_BRANCH_RETIRED",
                                                  "PERF_COUNT_HW_CACHE_REFERENCES",
                                                  "PERF_COUNT_HW_CACHE_MISSES",
                                                  "PERF_COUNT_HW_BRANCH_MISSES",
                                                  "PERF_COUNT_HW_CACHE_L1D"};
//    // HPC set 2
//    std::vector<std::string> benchmarkCounters = {"PERF_COUNT_HW_CPU_CYCLES",
//                                                  "INSTRUCTION_RETIRED",
//                                                  "PERF_COUNT_HW_CACHE_L1D",
//                                                  "L1-DCACHE-LOADS",
//                                                  "L1-DCACHE-LOAD-MISSES",
//                                                  "L1-DCACHE-STORES"};

    std::vector<float> inputThresholdDistribution;
    generateLinearDistribution(2, 0., 1, inputThresholdDistribution);

    // Update min & max to match dataFile
//    generateLinearDistribution(10, 0, 100, inputThresholdDistribution);
    // Update min & max to match dataFile
//    generateLogDistribution(30, 1, 10*1000, inputThresholdDistribution);

    selectBenchmarkWithExtraCounters(dataFile,
                                     selectImplementation,
                                     inputThresholdDistribution,
                                     iterations,
                                     benchmarkCounters, "");
}

void allSelectIndexesTests() {
    // Graph 1: Selectivity range on uniform data
    std::vector<float> inputThresholdDistribution;
    generateLogDistribution(30, 1, 10*1000, inputThresholdDistribution);
    selectCpuCyclesInputSweepBenchmark(DataFiles::uniformIntDistribution250mValuesMax10000,
                                       {Select::ImplementationIndexesBranch,
                                        Select::ImplementationIndexesPredication,
                                        Select::ImplementationIndexesAdaptive},
                                       inputThresholdDistribution,
                                       5, "Indexes");

    // Graph 2: Randomness range on sorted data
    selectCpuCyclesSweepBenchmark(DataSweeps::logSortedIntDistribution250mValuesRandomnessSweep,
                                  {Select::ImplementationIndexesBranch,
                                   Select::ImplementationIndexesPredication,
                                   Select::ImplementationIndexesAdaptive},
                                   50, 5, "Indexes");

    // Graph 3: Period range on linearly varying selectivity
    selectCpuCyclesSweepBenchmark(DataSweeps::varyingIntDistribution250mValuesSweep,
                                  {Select::ImplementationIndexesBranch,
                                   Select::ImplementationIndexesPredication,
                                   Select::ImplementationIndexesAdaptive},
                                   50, 5, "Indexes");

    // Graph 4: Period range on step varying selectivity
    selectCpuCyclesSweepBenchmark(DataSweeps::lowerStep50IntDistribution250mValuesSweep,
                                  {Select::ImplementationIndexesBranch,
                                   Select::ImplementationIndexesPredication,
                                   Select::ImplementationIndexesAdaptive},
                                   50, 5, "Indexes");

    // Graph 5: Best case - tuned unequal step varying selectivity
    selectCpuCyclesSingleInputBenchmark(DataFiles::bestCaseIndexesTunedUnequalLowerStep50IntDistribution250mValues,
                                        {Select::ImplementationIndexesBranch,
                                         Select::ImplementationIndexesPredication,
                                         Select::ImplementationIndexesAdaptive},
                                        50,
                                        5, "Indexes");

    // Graph 6: Worst case - tuned period range on step varying selectivity
    selectCpuCyclesSingleInputBenchmark(DataFiles::worstCaseIndexesTunedUpperStep50IntDistribution250mValues,
                                        {Select::ImplementationIndexesBranch,
                                         Select::ImplementationIndexesPredication,
                                         Select::ImplementationIndexesAdaptive},
                                        50,
                                        5, "Indexes");
}

void allSelectValuesTests() {
    // Graph 1: Selectivity range on uniform data
    std::vector<float> inputThresholdDistribution;
    generateLogDistribution(30, 1, 10 * 1000, inputThresholdDistribution);
    selectCpuCyclesInputSweepBenchmark(DataFiles::uniformIntDistribution250mValuesMax10000,
                                       {Select::ImplementationValuesBranch,
                                        Select::ImplementationValuesVectorized,
                                        Select::ImplementationValuesPredication,
                                        Select::ImplementationValuesAdaptive},
                                       inputThresholdDistribution,
                                       5, "Values");

    // Graph 2: Randomness range on sorted data
    selectCpuCyclesSweepBenchmark(DataSweeps::logSortedIntDistribution250mValuesRandomnessSweep,
                                  {Select::ImplementationValuesBranch,
                                   Select::ImplementationValuesVectorized,
                                   Select::ImplementationValuesAdaptive},
                                   50, 5, "Values");


    // Graph 3: Period range on linearly varying selectivity
    selectCpuCyclesSweepBenchmark(DataSweeps::varyingIntDistribution250mValuesSweep,
                                  {Select::ImplementationValuesBranch,
                                   Select::ImplementationValuesVectorized,
                                   Select::ImplementationValuesAdaptive},
                                   50, 5, "Values");

    // Graph 4: Period range on step varying selectivity
    selectCpuCyclesSweepBenchmark(DataSweeps::lowerStep50IntDistribution250mValuesSweep,
                                  {Select::ImplementationValuesBranch,
                                   Select::ImplementationValuesVectorized,
                                   Select::ImplementationValuesAdaptive},
                                   50, 5, "Values");

    // Graph 5: Best case - tuned unequal step varying selectivity
    selectCpuCyclesSingleInputBenchmark(DataFiles::bestCaseValuesTunedUnequalLowerStep50IntDistribution250mValues,
                                        {Select::ImplementationValuesBranch,
                                         Select::ImplementationValuesVectorized,
                                         Select::ImplementationValuesAdaptive},
                                        50,
                                        5, "Values");

    // Graph 6: Worst case - tuned period range on step varying selectivity
    selectCpuCyclesSingleInputBenchmark(DataFiles::worstCaseValuesTunedLowerStep50IntDistribution250mValues,
                                        {Select::ImplementationValuesBranch,
                                         Select::ImplementationValuesVectorized,
                                         Select::ImplementationValuesAdaptive},
                                        50,
                                        5, "Values");

}

bool comparePairs(const std::pair<int, int>& pair1, const std::pair<int, int>& pair2) {
    return pair1.first < pair2.first;
}

void groupByFunctionalityTest(const DataFile& dataFile, GroupBy groupByImplementation, aggFuncPtr aggregator) {
    auto inputGroupBy = new int[dataFile.getNumElements()];
    auto inputAggregate = new int[dataFile.getNumElements()];
    copyArray(LoadedData::getInstance(dataFile).getData(), inputGroupBy, dataFile.getNumElements());
    generateUniformDistributionInMemory(inputAggregate, dataFile.getNumElements(), 10);

    vectorOfPairs resultHash = groupByHash(dataFile.getNumElements(), inputGroupBy, inputAggregate, aggregator);
    std::sort(resultHash.begin(), resultHash.end(), comparePairs);

    vectorOfPairs resultInput = runGroupByFunction(groupByImplementation, dataFile.getNumElements(), inputGroupBy,
                                                   inputAggregate, aggregator);
    std::sort(resultInput.begin(), resultInput.end(), comparePairs);

    if (resultHash.size() != resultInput.size()) {
        std::cout << "Size of result is different from base hash implementation" << std::endl;
    }

    for (auto i = 0; i < static_cast<int> (resultHash.size()); ++i) {
        if ((resultHash[i].first != resultInput[i].first) || (resultHash[i].second != resultInput[i].second)) {
            std::cout << "Different result found" << std::endl;
        }
    }

    delete []inputGroupBy;
    delete []inputAggregate;
}


void groupByBenchmarkWithExtraCountersConfigurations(DataSweep &dataSweep,
                                                     GroupBy groupByImplementation,
                                                     aggFuncPtr aggregator,
                                                     int iterations,
                                                     const std::string &fileNamePrefix) {
    // HPC set 1
    std::vector<std::string> benchmarkCounters = {"PERF_COUNT_HW_CPU_CYCLES",
                                                  "INSTRUCTION_RETIRED",
                                                  "LLC_REFERENCES",
                                                  "PERF_COUNT_HW_CACHE_REFERENCES",
                                                  "PERF_COUNT_HW_CACHE_MISSES",
                                                  "PERF_COUNT_HW_CACHE_L1D",
                                                  "L1-DCACHE-LOADS",
                                                  "L1-DCACHE-LOAD-MISSES",
                                                  "L1-DCACHE-STORES"};

    groupByBenchmarkWithExtraCounters(dataSweep, groupByImplementation, aggregator, iterations, benchmarkCounters, fileNamePrefix);
}

void groupByBenchmarkWithExtraCountersDuringRunConfigurations(const DataFile &dataFile,
                                                              aggFuncPtr aggregator,
                                                     const std::string &fileNamePrefix) {
    // HPC set 1
    std::vector<std::string> benchmarkCounters = {"PERF_COUNT_HW_CPU_CYCLES",
                                                  "INSTRUCTION_RETIRED",
                                                  "LLC_REFERENCES",
                                                  "PERF_COUNT_HW_CACHE_REFERENCES",
                                                  "PERF_COUNT_HW_CACHE_MISSES",
                                                  "PERF_COUNT_HW_CACHE_L1D",
                                                  "L1-DCACHE-LOADS",
                                                  "L1-DCACHE-LOAD-MISSES",
                                                  "L1-DCACHE-STORES"};

    groupByBenchmarkWithExtraCountersDuringRun(dataFile, aggregator, benchmarkCounters, fileNamePrefix);
}


void allGroupByTests() {
    // Graph 1: Cardinality range on uniform data (variable max) for different hashmaps - compile with -march=native removed
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepVariableMax,
                                   {GroupBy::HashGoogleDenseHashMap,
//                                    GroupBy::HashFollyF14FastMap,   // Need to turn off -march=native for this one
                                    GroupBy::HashAbseilFlatHashMap,
                                    GroupBy::HashTessilRobinMap,
                                    GroupBy::HashTessilHopscotchMap}, countAggregation,
                                   5, "1-MapCompare");

    // Graph 2: Cardinality range (variable max) for simple radix sort - manually run with BITS_PER_PASS of 8, 10, 12
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepVariableMax,
                                   {GroupBy::SortRadix}, countAggregation,
                                   5, "2-RadixSimple");

    // Graph 3: Cardinality range (variable max) for optimised radix sort - manually run with BITS_PER_PASS of 8, 10, 12
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepVariableMax,
                                   {GroupBy::SortRadixOpt}, countAggregation,
                                   5, "3-RadixOpt");

    // Graph 5: Cardinality range on uniform data (fixed max) for different hashmaps - compile with -march=native removed
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepFixedMax,
                                   {GroupBy::HashGoogleDenseHashMap,
//                                    GroupBy::HashFollyF14FastMap,   // Need to turn off -march=native for this one
                                    GroupBy::HashAbseilFlatHashMap,
                                    GroupBy::HashTessilRobinMap,
                                    GroupBy::HashTessilHopscotchMap}, countAggregation,
                                   5, "5-MapCompare");

    // Graph 6: Cardinality range (fixed max) for simple radix sort - manually run with BITS_PER_PASS of 8, 10, 12
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepFixedMax,
                                   {GroupBy::SortRadix}, countAggregation,
                                   5, "6-RadixSimple");

    // Graph 7: Cardinality range (fixed max) for optimised radix sort - manually run with BITS_PER_PASS of 8, 10, 12
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepFixedMax,
                                   {GroupBy::SortRadixOpt}, countAggregation,
                                   5, "7-RadixOpt");

    // Graph 9: Cardinality range (fixed max) for single / double radix10 pass
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepFixedMax,
                                   {GroupBy::SingleRadixPassThenHash,
                                    GroupBy::DoubleRadixPassThenHash}, countAggregation,
                                   5, "9-SingleDoubleRadix10PassThenHash");
}


int main() {
    // Graph 1: Cardinality range on uniform data (variable max) for different hashmaps - compile with -march=native removed
    groupByCpuCyclesSweepBenchmark(DataSweeps::linearUniformIntDistribution200mValuesCardinalitySweepFixedMaxCrossOverPoint,
                                   {GroupBy::HashGoogleDenseHashMap,
//                                    GroupBy::HashFollyF14FastMap,   // Need to turn off -march=native for this one
                                    GroupBy::HashAbseilFlatHashMap,
                                    GroupBy::HashTessilRobinMap,
                                    GroupBy::HashTessilHopscotchMap}, countAggregation,
                                   1, "1-MapCompare");

/*    // Graph 2: Cardinality range (variable max) for simple radix sort - manually run with BITS_PER_PASS of 8, 10, 12
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepVariableMax,
                                   {GroupBy::SortRadix}, countAggregation,
                                   1, "2-RadixSimple");

    // Graph 3: Cardinality range (variable max) for optimised radix sort - manually run with BITS_PER_PASS of 8, 10, 12
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepVariableMax,
                                   {GroupBy::SortRadixOpt}, countAggregation,
                                   1, "3-RadixOpt");

    // Graph 3: Cardinality range (variable max) for optimised radix sort - manually run with BITS_PER_PASS of 8, 10, 12
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepVariableMax,
                                   {GroupBy::Adaptive}, countAggregation,
                                   1, "3-Adaptive");*/

    // Graph 5: Cardinality range on uniform data (fixed max) for different hashmaps - compile with -march=native removed
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepFixedMax,
                                   {GroupBy::HashGoogleDenseHashMap,
//                                    GroupBy::HashFollyF14FastMap,   // Need to turn off -march=native for this one
                                    GroupBy::HashAbseilFlatHashMap,
                                    GroupBy::HashTessilRobinMap,
                                    GroupBy::HashTessilHopscotchMap}, countAggregation,
                                   1, "5-MapCompare");

/*    // Graph 6: Cardinality range (fixed max) for simple radix sort - manually run with BITS_PER_PASS of 8, 10, 12
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepFixedMax,
                                   {GroupBy::SortRadix}, countAggregation,
                                   1, "6-RadixSimple");

    // Graph 7: Cardinality range (fixed max) for optimised radix sort - manually run with BITS_PER_PASS of 8, 10, 12
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepFixedMax,
                                   {GroupBy::SortRadixOpt}, countAggregation,
                                   1, "7-RadixOpt");

    // Graph 7: Cardinality range (fixed max) for optimised radix sort - manually run with BITS_PER_PASS of 8, 10, 12
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepFixedMax,
                                   {GroupBy::Adaptive}, countAggregation,
                                   1, "7-Adaptive");

    // Graph 9: Cardinality range (fixed max) for single / double radix10 pass
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepFixedMax,
                                   {GroupBy::SingleRadixPassThenHash,
                                    GroupBy::DoubleRadixPassThenHash}, countAggregation,
                                   1, "9-SingleDoubleRadix10PassThenHash");

    // Graph 7: Cardinality range (fixed max) for optimised radix sort - manually run with BITS_PER_PASS of 8, 10, 12
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepFixedMaxClustered1,
                                   {GroupBy::Hash, GroupBy::SortRadixOpt, GroupBy::Adaptive}, countAggregation,
                                   1, "10Cluster1");

    // Graph 7: Cardinality range (fixed max) for optimised radix sort - manually run with BITS_PER_PASS of 8, 10, 12
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepFixedMaxClustered1k,
                                   {GroupBy::Hash, GroupBy::SortRadixOpt, GroupBy::Adaptive}, countAggregation,
                                   1, "10Cluster1k");


    // Graph 7: Cardinality range (fixed max) for optimised radix sort - manually run with BITS_PER_PASS of 8, 10, 12
    groupByCpuCyclesSweepBenchmark(DataSweeps::logUniformIntDistribution200mValuesCardinalitySweepFixedMaxClustered100k,
                                   {GroupBy::Hash, GroupBy::SortRadixOpt, GroupBy::Adaptive}, countAggregation,
                                   1, "10Cluster100k");*/

//    groupByBenchmarkWithExtraCountersConfigurations(DataSweeps::linearUniformIntDistribution200mValuesCardinalitySweepFixedMaxCrossOverPoint,
//                                   GroupBy::Hash, countAggregation,
//                                   1, "4-DetailedCrossOver_Hash");
//
//    groupByBenchmarkWithExtraCountersConfigurations(DataSweeps::linearUniformIntDistribution200mValuesCardinalitySweepFixedMaxCrossOverPoint,
//                                                    GroupBy::SortRadixOpt, countAggregation,
//                                                    1, "4-DetailedCrossOver_SortRadixOpt");
//

    return 0;
}