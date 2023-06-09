#include <iostream>
#include <random>
#include <set>
#include <cassert>

#include "dataGenerators.h"


void generateVaryingSelectivityInMemory(int *data, int n, int minimum, int numberOfDiscreteSections) {
    std::cout << "Generating data in memory... ";
    std::cout.flush();

    int elementsPerSection = n / (numberOfDiscreteSections * minimum);

    unsigned int seed = 1;
    std::mt19937 gen(seed);

    bool increasing = true;
    auto lowerBound = 1;
    auto upperBound = 100;

    auto index = 0;
    for (auto i = 0; i < numberOfDiscreteSections; ++i) {
        for (auto j = 0; j < minimum; ++j) {
            if (j > 0) {
                if (increasing) {
                    lowerBound += 1;
                } else {
                    lowerBound -= 1;
                }
            }

            std::uniform_int_distribution<int> distribution(lowerBound, upperBound);
            for (auto k = 0; k < elementsPerSection; ++k) {
                data[index++] = distribution(gen);
            }
        }
        increasing = !increasing;
    }

    std::cout << "Complete" << std::endl;
}

void generateUpperStepSelectivityInMemory(int *data, int n, int step, int numberOfDiscreteSections) {
    std::cout << "Generating data in memory... ";
    std::cout.flush();

    int elementsPerSection = n / numberOfDiscreteSections;

    unsigned int seed = 1;
    std::mt19937 gen(seed);

    bool onStep = false;
    auto lowerBound = 1;
    int upperBound;

    auto index = 0;
    for (auto i = 0; i < numberOfDiscreteSections; ++i) {
        if (onStep) {
            upperBound = step;
        } else {
            upperBound = 100;
        }

        std::uniform_int_distribution<int> distribution(lowerBound, upperBound);
        for (auto k = 0; k < elementsPerSection; ++k) {
            data[index++] = distribution(gen);
        }

        onStep = !onStep;
    }

    std::cout << "Complete" << std::endl;
}


void generateLowerStepSelectivityInMemory(int *data, int n, int step, int numberOfDiscreteSections) {
    std::cout << "Generating data in memory... ";
    std::cout.flush();

    int elementsPerSection = n / numberOfDiscreteSections;

    unsigned int seed = 1;
    std::mt19937 gen(seed);

    bool onStep = false;
    int lowerBound;
    auto upperBound = 100;

    auto index = 0;
    for (auto i = 0; i < numberOfDiscreteSections; ++i) {
        if (onStep) {
            lowerBound = step;
        } else {
            lowerBound = 1;
        }

        std::uniform_int_distribution<int> distribution(lowerBound, upperBound);
        for (auto k = 0; k < elementsPerSection; ++k) {
            data[index++] = distribution(gen);
        }

        onStep = !onStep;
    }

    std::cout << "Complete" << std::endl;
}


void generateUnequalLowerStepSelectivityInMemory(int *data, int n, int step, int numberOfDiscreteSections, int sectionRatio) {
    std::cout << "Generating data in memory... ";
    std::cout.flush();

    int elementsPerSection = n / numberOfDiscreteSections;

    auto elementsPerStepSection = elementsPerSection * sectionRatio / (1 + sectionRatio);
    auto elementsPerNoStepSection = elementsPerSection - elementsPerStepSection;

    unsigned int seed = 1;
    std::mt19937 gen(seed);

    bool onStep = false;
    int lowerBound;
    auto upperBound = 100;

    auto index = 0;
    for (auto i = 0; i < (2 * numberOfDiscreteSections); ++i) {
        if (onStep) {
            lowerBound = step;
        } else {
            lowerBound = 1;
        }

        std::uniform_int_distribution<int> distribution(lowerBound, upperBound);
        if (onStep) {
            for (auto k = 0; k < elementsPerStepSection; ++k) {
                data[index++] = distribution(gen);
            }
        } else {
            for (auto k = 0; k < elementsPerNoStepSection; ++k) {
                data[index++] = distribution(gen);
            }

        }

        onStep = !onStep;
    }

    std::cout << "Complete" << std::endl;
}

void generatePartiallySortedInMemory(int *data, int n, int numRepeats, float percentageRandom) {
    std::cout << "Generating data in memory... ";
    std::cout.flush();

    int sectionSize = 100 * numRepeats;
    int sections = n / (sectionSize);
    int elementsToShufflePerSection = static_cast<int>(
            0.5 * (percentageRandom / 100.0) * static_cast<float>(sectionSize));

//    std::cout << sectionSize << " section size" << std::endl;
//    std::cout << sections << " sections" << std::endl;
//    std::cout << elementsToShufflePerSection << " pairs to shuffle per section" << std::endl;

    unsigned int seed = 1;
    std::mt19937 gen(seed);

    bool increasing = true;
    auto lowerBound = 1;
    auto upperBound = 100;

    int value;
    auto index = 0;
    for (auto i = 0; i < sections; ++i) {
        value = increasing ? lowerBound : upperBound;
        for (auto j = 0; j < 100; ++j) {
            for (auto k = 0; k < numRepeats; ++k) {
                data[index++] = value;
            }
            value = increasing ? value + 1 : value - 1;
        }
        increasing = !increasing;

        std::set<int> selectedIndexes = {};
        int index1, index2;
        for (auto swapCount = 0; swapCount < elementsToShufflePerSection; ++swapCount) {
            std::uniform_int_distribution<int> dis(1, sectionSize);

            index1 = dis(gen);
            while (selectedIndexes.count(index1) > 0) {
                index1 = dis(gen);
            }

            index2 = dis(gen);
            while (selectedIndexes.count(index2) > 0) {
                index2 = dis(gen);
            }

            selectedIndexes.insert(index1);
            selectedIndexes.insert(index2);
            std::swap(data[index - index1], data[index - index2]);

//            std::cout << "Swapped indexes: " << index1 << ", " << index2 << std::endl;
        }

/*        for (int x = 0; x < 100; ++x) {
            for (int y = 0; y < numRepeats; ++y) {
                std::cout << data[index - sectionSize + (x * numRepeats) + y] << std::endl;
            }
        }*/
    }

    std::cout << "Complete" << std::endl;
}

void generateUniformDistributionInMemory(int *data, int n, int upperBound) {
    std::cout << "Generating data in memory... ";
    std::cout.flush();

    if (upperBound == n) {
        generateUniqueValuesRandomisedInMemory(data, n);
        return;
    }

    unsigned int seed = 1;
    std::mt19937 gen(seed);

    auto lowerBound = 1;

    std::uniform_int_distribution<int> distribution(lowerBound, upperBound);

    for (auto i = 0; i < n; ++i) {
        data[i] = distribution(gen);
    }

    std::cout << "Complete" << std::endl;
}

inline int scaleNumberLinearly(int number, int startingUpperBound, int targetUpperBound) {
    if (number == 1) {
        return 1;
    }
    return 1 + (number - 1) * (static_cast<double>((targetUpperBound - 1)) / (startingUpperBound - 1));
}

inline int scaleNumberLogarithmically(int number, int startingUpperBound, int targetUpperBound) {
    double scaledValue = log(number) / log(startingUpperBound);
    double scaledNumber = pow(targetUpperBound, scaledValue);
    return std::round(scaledNumber);
}

void generateUniqueValuesRandomisedInMemory(int *data, int n) {
    // Fisher–Yates shuffle

    for (int i = 1; i <= n; ++i) {
        data[i - 1] = i;
    }

    unsigned int seed = 1;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dis(1, 1);
    int j;

    for (int i = n - 1; i > 0; --i) {
        dis = std::uniform_int_distribution<>(1, i);
        j = dis(gen);
        std::swap(data[i], data[j]);
    }
}

void generateUniqueValuesRandomisedInMemory(int64_t *data, int n) {
    // Fisher–Yates shuffle

    for (int i = 1; i <= n; ++i) {
        data[i - 1] = i;
    }

    unsigned int seed = 1;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dis(1, 1);
    int j;

    for (int i = n - 1; i > 0; --i) {
        dis = std::uniform_int_distribution<>(1, i);
        j = dis(gen);
        std::swap(data[i], data[j]);
    }
}

void generateUniformDistributionInMemoryWithSetCardinality(int *data, int n, int upperBound, int cardinality) {
    assert(cardinality <= upperBound);
    std::cout << "Generating data in memory... ";
    std::cout.flush();

    if (cardinality == 1) {
        for (auto i = 0; i < n; ++i) {
            data[i] = upperBound;
        }
        std::cout << "Complete" << std::endl;
        return;
    }

    if (cardinality == n) {
        generateUniqueValuesRandomisedInMemory(data, n);
        return;
    }

    unsigned int seed = 1;
    std::mt19937 gen(seed);

    std::uniform_int_distribution<int> distribution(1, cardinality);

    for (auto i = 0; i < n; ++i) {
        data[i] = scaleNumberLogarithmically(distribution(gen), cardinality, upperBound);
    }

    std::cout << "Complete" << std::endl;
}

void generateUniformDistributionInMemoryWithSetCardinality(int64_t *data, int n, int upperBound, int cardinality) {
    assert(cardinality <= upperBound);
    std::cout << "Generating data in memory... ";
    std::cout.flush();

    if (cardinality == 1) {
        for (auto i = 0; i < n; ++i) {
            data[i] = upperBound;
        }
        std::cout << "Complete" << std::endl;
        return;
    }

    if (cardinality == n) {
        generateUniqueValuesRandomisedInMemory(data, n);
        return;
    }

    unsigned int seed = 1;
    std::mt19937 gen(seed);

    std::uniform_int_distribution<int> distribution(1, cardinality);

    for (auto i = 0; i < n; ++i) {
        data[i] = scaleNumberLogarithmically(distribution(gen), cardinality, upperBound);
    }

    std::cout << "Complete" << std::endl;
}

void generateUniformDistributionInMemoryWithSetCardinalityClustered(int *data, int n, int upperBound,
                                                                    int cardinality, int spreadInCluster) {
    assert(cardinality <= upperBound);
    if (spreadInCluster >= cardinality) {
        generateUniformDistributionInMemoryWithSetCardinality(data, n, upperBound ,cardinality);
        return;
    }

    std::cout << "Generating data in memory... ";
    std::cout.flush();

    if (cardinality == 1) {
        for (auto i = 0; i < n; ++i) {
            data[i] = upperBound;
        }
        std::cout << "Complete" << std::endl;
        return;
    }

    unsigned int seed = 1;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> distribution(1, spreadInCluster);

    int numberOfSections = 1 + cardinality - spreadInCluster;
    int elementsPerSection = n / numberOfSections;
    int index = 0;
    for (int i = 0; i < numberOfSections - 1; i++) {
        for (int j = 0; j < elementsPerSection; j++) {
            data[index++] = scaleNumberLogarithmically(i + distribution(gen), cardinality, upperBound);
        }
    }
    while (index < n) {
        data[index++] = scaleNumberLogarithmically(cardinality - spreadInCluster + distribution(gen), cardinality, upperBound);
    }

    std::cout << "Complete" << std::endl;
}

void generateUniformDistributionInMemoryWithTwoCardinalitySections(int *data, int n, int upperBound,
                                                                   int cardinalitySectionOne, int cardinalitySectionTwo,
                                                                   float fractionSectionTwo) {
    assert(cardinalitySectionOne <= upperBound && cardinalitySectionTwo <= upperBound);
    std::cout << "Generating data in memory... ";
    std::cout.flush();

    unsigned int seed = 1;
    std::mt19937 gen(seed);

    int sizeSectionOne = n * (1.0 - fractionSectionTwo);

    if (cardinalitySectionOne == 1) {
        for (auto i = 0; i < sizeSectionOne; ++i) {
            data[i] = upperBound;
        }
    } else {
        std::uniform_int_distribution<int> distribution(1, cardinalitySectionOne);

        for (auto i = 0; i < sizeSectionOne; ++i) {
            data[i] = scaleNumberLogarithmically(distribution(gen), cardinalitySectionOne,
                                                 upperBound);
        }
    }

    if (cardinalitySectionTwo == 1) {
        for (auto i = sizeSectionOne; i < n; ++i) {
            data[i] = upperBound;
        }
    } else {
        std::uniform_int_distribution<int> distribution(1, cardinalitySectionTwo);

        for (auto i = sizeSectionOne; i < n; ++i) {
            data[i] = scaleNumberLogarithmically(distribution(gen), cardinalitySectionTwo,
                                                 upperBound);
        }
    }

    std::cout << "Complete" << std::endl;
}

void generateUniformDistributionInMemoryWithMultipleTwoCardinalitySections(int *data, int n, int upperBound,
                                                                           int cardinalitySectionOne, int cardinalitySectionTwo,
                                                                           float fractionSectionTwo, int numSections) {
    assert(n % numSections == 0);

    int tuplesPerSection = n / numSections;
    for (int i = 0; i < numSections; ++i) {
        generateUniformDistributionInMemoryWithTwoCardinalitySections(data, tuplesPerSection, upperBound, cardinalitySectionOne,
                                                                      cardinalitySectionTwo, fractionSectionTwo);
        data += tuplesPerSection;
    }
}