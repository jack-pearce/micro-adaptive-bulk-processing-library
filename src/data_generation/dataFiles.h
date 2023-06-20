#ifndef MABPL_DATAFILES_H
#define MABPL_DATAFILES_H

#include <string>
#include <vector>

extern const std::string inputFilePath;
extern const std::string outputFilePath;
extern const std::string selectCyclesFolder;
extern const std::string dataFilesFolder;


class DataFile {
public:
    DataFile(int _numElements, std::string _fileName, std::string _longDescription);

    [[nodiscard]] int getNumElements() const;
    [[nodiscard]] const std::string& getFileName() const;
    [[nodiscard]] const std::string& getLongDescription() const;
    void loadDataIntoMemory(int *data) const;

private:
    int numElements;
    std::string fileName;
    std::string longDescription;
};

class DataFiles {
public:
    static const DataFile uniformIntDistribution25kValuesMax100;
    static const DataFile uniformIntDistribution250mValuesMax100;
    static const DataFile uniformIntDistribution250mValuesMax1000000;

    static const DataFile varyingIntDistribution25kValues;
    static const DataFile varyingIntDistribution250mValues;

    static const DataFile step50IntDistribution25kValues;
    static const DataFile step50IntDistribution250mValues;
    static const DataFile worstCaseTunedStep50IntDistribution250mValues;

    static const DataFile bestCaseTunedUnequalStep50IntDistribution250mValues;

    static const DataFile fullySortedIntDistribution250mValues;
    static const DataFile veryNearlyFullySortedIntDistribution250mValues;
    static const DataFile nearlyFullySortedIntDistribution250mValues;
    static const DataFile partiallySortedIntDistribution250mValues;
    static const DataFile slightlySortedIntDistribution250mValues;
};

class DataSweep {
public:
    DataSweep(int _totalRuns, int _numElements, std::string _sweepName, std::string _longDescription);

    [[nodiscard]] int getTotalRuns() const;
    [[nodiscard]] int getNumElements() const;
    [[nodiscard]] const std::string& getSweepName() const;
    [[nodiscard]] const std::string& getLongDescription() const;
    [[nodiscard]] float getRunInput() const;
    bool loadNextDataSetIntoMemory(int *data);
    void restartSweep();

private:
    int totalRuns;
    int numElements;
    int runsCompleted;
    std::string sweepName;
    std::string longDescription;
    std::vector<float> inputs;
};

class DataSweeps {
public:
    static DataSweep logSortedIntDistribution25kValuesRandomnessSweep;
    static DataSweep logSortedIntDistribution250mValuesRandomnessSweep;

    static DataSweep varyingIntDistribution250mValuesSweep;

    static DataSweep step50IntDistribution250mValuesSweep;
};

#endif //MABPL_DATAFILES_H
