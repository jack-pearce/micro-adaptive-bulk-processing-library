#include <iostream>
#include <functional>
#include <vector>
#include <string>
#include <cassert>

#include "../utils/dataHelpers.h"
#include "../utils/papiHelpers.h"
#include "counterBenchmark.h"


void selectSingleRunNoCounters(const DataFile &dataFile, SelectImplementation selectImplementation, int threshold,
                               int iterations) {
    SelectFunctionPtr selectFunctionPtr;
    setSelectFuncPtr(selectFunctionPtr, selectImplementation);

    for (int j = 0; j < iterations; ++j) {
        int* inputData = new int[dataFile.getNumElements()];
        int* selection = new int[dataFile.getNumElements()];
        copyArray(LoadedData::getInstance(dataFile).getData(), inputData, dataFile.getNumElements());

        std::cout << "Running threshold " << threshold << ", iteration " << j + 1 << "... ";

        selectFunctionPtr(dataFile.getNumElements(), inputData, selection, threshold);

        delete[] inputData;
        delete[] selection;

        std::cout << "Completed" << std::endl;
    }
}

void selectCpuCyclesSingleInputBenchmark(const DataFile &dataFile, const std::vector<SelectImplementation> &selectImplementations,
                                    int threshold, int iterations) {
    int numTests = static_cast<int>(selectImplementations.size());

    long_long cycles;
    std::vector<std::vector<long_long>> results(iterations, std::vector<long_long>(numTests + 1, 0));

    SelectFunctionPtr selectFunctionPtr;

    for (int i = 0; i < iterations; ++i) {
        results[i][0] = static_cast<long_long>(threshold);

        for (int j = 0; j < numTests; ++j) {
            setSelectFuncPtr(selectFunctionPtr, selectImplementations[j]);

            int *inputData = new int[dataFile.getNumElements()];
            int *selection = new int[dataFile.getNumElements()];
            copyArray(LoadedData::getInstance(dataFile).getData(), inputData, dataFile.getNumElements());

            std::cout << "Running " << getName(selectImplementations[j]) << ", iteration " << i + 1 << "... ";

            cycles = *Counters::getInstance().readEventSet();

            selectFunctionPtr(dataFile.getNumElements(), inputData, selection, threshold);

            results[i][1 + j] = *Counters::getInstance().readEventSet() - cycles;

            delete[] inputData;
            delete[] selection;

            std::cout << "Completed" << std::endl;
        }
    }

    std::vector<std::string> headers(1 + numTests);
    headers [0] = "Input";
    for (int i = 0; i < numTests; ++i) {
        headers[1 + i] = getName(selectImplementations[i]);
    }

    std::string fileName =
            "SingleCyclesBM_" +
            dataFile.getFileName() +
            "_threshold_" +
            std::to_string(threshold);
    std::string fullFilePath = outputFilePath + selectCyclesFolder + fileName + ".csv";
    writeHeadersAndTableToCSV(headers, results, fullFilePath);
}

void selectCpuCyclesMultipleInputBenchmark(const DataFile& dataFile,
                                           const std::vector<SelectImplementation>& selectImplementations,
                                           int selectivityStride,
                                           int iterations) {
    int numTests = 1 + (100 / selectivityStride);
    int implementations = static_cast<int>(selectImplementations.size());
    int dataCols = implementations * iterations;

    long_long cycles;
    std::vector<std::vector<long_long>> results(numTests, std::vector<long_long>(dataCols + 1, 0));

    SelectFunctionPtr selectFunctionPtr;
    for (int i = 0; i < implementations; ++i) {
        setSelectFuncPtr(selectFunctionPtr, selectImplementations[i]);
        int count = 0;

        for (int j = 0; j <= 100; j += selectivityStride) {
            results[count][0] = static_cast<long_long>(j);

            for (int k = 0; k < iterations; ++k) {
                int *inputData = new int[dataFile.getNumElements()];
                int *selection = new int[dataFile.getNumElements()];
                copyArray(LoadedData::getInstance(dataFile).getData(), inputData, dataFile.getNumElements());

                std::cout << "Running " << getName(selectImplementations[i]) << ", threshold ";
                std::cout << j << ", iteration " << k + 1 << "... ";

                cycles = *Counters::getInstance().readEventSet();

                selectFunctionPtr(dataFile.getNumElements(), inputData, selection, j);

                results[count][1 + (i * iterations) + k] = *Counters::getInstance().readEventSet() - cycles;

                delete[] inputData;
                delete[] selection;

                std::cout << "Completed" << std::endl;
            }

            count++;
        }
    }

    std::vector<std::string> headers(1 + dataCols);
    headers[0] = "Threshold";
    for (int i = 0; i < dataCols; ++i) {
        headers[1 + i] = getName(selectImplementations[i / iterations]) + " iteration_" + std::to_string(i % iterations);
    }

    std::string fileName =
            "MultiplesCyclesBM_" +
            dataFile.getFileName() +
            "_selectivityStride_" +
            std::to_string(selectivityStride);
    std::string fullFilePath = outputFilePath + selectCyclesFolder + fileName + ".csv";
    writeHeadersAndTableToCSV(headers, results, fullFilePath);
}

void selectBenchmarkWithExtraCounters(const DataFile& dataFile,
                                      SelectImplementation selectImplementation,
                                      int selectivityStride,
                                      int iterations,
                                      std::vector<std::string>& benchmarkCounters) {
    if (selectImplementation == SelectImplementation::IndexesAdaptive)
        std::cout << "Cannot benchmark adaptive select using counters as adaptive select is already using these counters" << std::endl;

    int numTests = 1 + (100 / selectivityStride);

    long_long benchmarkCounterValues[benchmarkCounters.size()];
    int benchmarkEventSet = initialisePAPIandCreateEventSet(benchmarkCounters);

    std::vector<std::vector<long_long>> results(numTests, std::vector<long_long>((iterations * benchmarkCounters.size()) + 1, 0));
    int count = 0;

    SelectFunctionPtr selectFunctionPtr;
    setSelectFuncPtr(selectFunctionPtr, selectImplementation);

    for (int i = 0; i <= 100; i += selectivityStride) {
        results[count][0] = static_cast<long_long>(i);

        for (int j = 0; j < iterations; ++j) {
            int* inputData = new int[dataFile.getNumElements()];
            int* selection = new int[dataFile.getNumElements()];
            copyArray(LoadedData::getInstance(dataFile).getData(), inputData, dataFile.getNumElements());

            std::cout << "Running threshold " << i << ", iteration " << j + 1 << "... ";

            if (PAPI_reset(benchmarkEventSet) != PAPI_OK)
                exit(1);

            selectFunctionPtr(dataFile.getNumElements(), inputData, selection, i);

            if (PAPI_read(benchmarkEventSet, benchmarkCounterValues) != PAPI_OK)
                exit(1);

            delete[] inputData;
            delete[] selection;

            for (int k = 0; k < static_cast<int>(benchmarkCounters.size()); ++k) {
                results[count][1 + (j * benchmarkCounters.size()) + k] = benchmarkCounterValues[k];
            }

            std::cout << "Completed" << std::endl;
        }

        count++;
    }

    std::vector<std::string> headers(benchmarkCounters.size() * iterations);
    for (int i = 0; i < iterations; ++i) {
        std::copy(benchmarkCounters.begin(), benchmarkCounters.end(), headers.begin() + i * benchmarkCounters.size());
    }
    headers.insert(headers.begin(), "Selectivity");

    std::string fileName =
            "ExtraCountersCyclesBM_" +
            getName(selectImplementation) +
            dataFile.getFileName() +
            "_selectivityStride_" +
            std::to_string(selectivityStride);
    std::string fullFilePath = outputFilePath + selectCyclesFolder + fileName + ".csv";
    writeHeadersAndTableToCSV(headers, results, fullFilePath);

    shutdownPAPI(benchmarkEventSet, benchmarkCounterValues);
}

void selectCpuCyclesSweepBenchmark(DataSweep &dataSweep, const std::vector<SelectImplementation> &selectImplementations,
                                   int threshold, int iterations) {
    assert(!selectImplementations.empty());

    int dataCols = iterations * static_cast<int>(selectImplementations.size());
    long_long cycles;
    std::vector<std::vector<double>> results(dataSweep.getTotalRuns(),
                                             std::vector<double>(dataCols + 1, 0));

    SelectFunctionPtr selectFunctionPtr;
    for (int i = 0; i < iterations; ++i) {
        for (int j = 0; j < static_cast<int>(selectImplementations.size()); ++j) {
            for (int k = 0; k < dataSweep.getTotalRuns(); ++k) {
                results[k][0] = static_cast<double>(dataSweep.getRunInput());
                setSelectFuncPtr(selectFunctionPtr, selectImplementations[j]);
                std::cout << "Running " << getName(selectImplementations[j]) << " for input ";
                std::cout << dataSweep.getRunInput() << "... ";

                int *inputData = new int[dataSweep.getNumElements()];
                int *selection = new int[dataSweep.getNumElements()];
                dataSweep.loadNextDataSetIntoMemory(inputData);

                cycles = *Counters::getInstance().readEventSet();

                selectFunctionPtr(dataSweep.getNumElements(), inputData, selection, threshold);

                results[k][1 + (i * selectImplementations.size()) + j] =
                        static_cast<double>(*Counters::getInstance().readEventSet() - cycles);

                delete[] inputData;
                delete[] selection;

                std::cout << "Completed" << std::endl;
            }
            dataSweep.restartSweep();
        }
    }

    std::vector<std::string> headers(1 + dataCols);
    headers [0] = "Input";
    for (int i = 0; i < dataCols; ++i) {
        headers[1 + i] = getName(selectImplementations[i % selectImplementations.size()]);
    }

    std::string fileName = "SweepCyclesBM_" + dataSweep.getSweepName() + "_threshold_" + std::to_string(threshold);
    std::string fullFilePath = outputFilePath + selectCyclesFolder + fileName + ".csv";
    writeHeadersAndTableToCSV(headers, results, fullFilePath);
}




