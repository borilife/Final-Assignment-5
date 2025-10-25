#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sstream>
using namespace std;

// =====================================================
// FUNCTION DECLARATIONS
// =====================================================
void selection_sort(int* values, int size);
bool binary_search_recursive(int* values, int key, int start, int end);
bool binary_search(int* values, int key, int size);
void createBinaryFile(const string& filename);
void writeBinary(const string& filename, int* values, int length);

// =====================================================
// CLASS DECLARATIONS
// =====================================================
class BinaryReader {
private:
    int* values;
    int size;

public:
    BinaryReader(const string& filename) {
        values = NULL;
        size = 0;
        readValues(filename);
    }

    ~BinaryReader() {
        delete[] values;
    }

    int* getValues() { return values; }
    int getSize() { return size; }

    void readValues(const string& filename) {
        ifstream fin(filename.c_str(), ios::binary);
        if (!fin) {
            cout << "Binary file not found, creating new file..." << endl;
            createBinaryFile(filename);
            fin.open(filename.c_str(), ios::binary);
        }

        fin.read((char*)&size, sizeof(size));
        if (size <= 0) {
            cout << "Invalid file data.
";
            return;
        }

        values = new int[size];
        fin.read((char*)values, sizeof(int) * size);
        fin.close();
    }
};

// =====================================================
// BASE CLASS: Analyzer
// =====================================================
class Analyzer {
protected:
    int* values;
    int size;

public:
    Analyzer(int* v, int s) {
        size = s;
        values = new int[size];
        for (int i = 0; i < size; i++)
            values[i] = v[i];
    }

    virtual ~Analyzer() { delete[] values; }

    int* cloneValues(int* src, int n) {
        int* copy = new int[n];
        for (int i = 0; i < n; i++) copy[i] = src[i];
        return copy;
    }

    virtual string analyze() = 0;
};

// =====================================================
// DuplicatesAnalyzer CLASS
// =====================================================
class DuplicatesAnalyser : public Analyzer {
public:
    DuplicatesAnalyser(int* v, int s) : Analyzer(v, s) {}

    string analyze() override {
        selection_sort(values, size);
        int dupCount = 0;
        for (int i = 1; i < size; i++)
            if (values[i] == values[i - 1]) dupCount++;
        return "Duplicate count: " + to_string(dupCount);
    }
};

// =====================================================
// MissingAnalyzer CLASS
// =====================================================
class MissingAnalyser : public Analyzer {
public:
    MissingAnalyser(int* v, int s) : Analyzer(v, s) {}

    string analyze() override {
        selection_sort(values, size);
        int missing = 0;
        for (int i = 1; i < size; i++) {
            if (values[i] - values[i - 1] > 1)
                missing += (values[i] - values[i - 1] - 1);
        }
        return "Missing numbers between min and max: " + to_string(missing);
    }
};

// =====================================================
// SearchAnalyzer CLASS
// =====================================================
class SearchAnalyser : public Analyzer {
public:
    SearchAnalyser(int* v, int s) : Analyzer(v, s) {
        selection_sort(values, size);
    }

    string analyze() override {
        srand((unsigned)time(0));
        int foundCount = 0;
        for (int i = 0; i < 100; i++) {
            int randomValue = rand() % 1000;
            if (binary_search(values, randomValue, size))
                foundCount++;
        }
        return "Random search: " + to_string(foundCount) + " values found in dataset.";
    }
};

// =====================================================
// StatisticsAnalyzer CLASS
// =====================================================
class StatisticsAnalyser : public Analyzer {
public:
    StatisticsAnalyser(int* v, int s) : Analyzer(v, s) {}

    string analyze() override {
        selection_sort(values, size);

        int minVal = values[0];
        int maxVal = values[size - 1];

        // Compute mean
        long long sum = 0;
        for (int i = 0; i < size; i++) sum += values[i];
        double mean = (double)sum / size;

        // Compute median
        double median = 0.0;
        if (size % 2 == 0)
            median = (values[size / 2 - 1] + values[size / 2]) / 2.0;
        else
            median = values[size / 2];

        // Compute mode
        int mode = values[0];
        int modeCount = 1, currentCount = 1;
        for (int i = 1; i < size; i++) {
            if (values[i] == values[i - 1])
                currentCount++;
            else
                currentCount = 1;

            if (currentCount > modeCount) {
                modeCount = currentCount;
                mode = values[i];
            }
        }

        ostringstream out;
        out << "Min: " << minVal
            << ", Max: " << maxVal
            << ", Mean: " << mean
            << ", Median: " << median
            << ", Mode: " << mode
            << " (count=" << modeCount << ")";
        return out.str();
    }
};

// =====================================================
// FUNCTION DEFINITIONS
// =====================================================
void selection_sort(int* values, int size) {
    for (int i = 0; i < size - 1; i++) {
        int minIndex = i;
        for (int j = i + 1; j < size; j++) {
            if (values[j] < values[minIndex])
                minIndex = j;
        }
        int temp = values[i];
        values[i] = values[minIndex];
        values[minIndex] = temp;
    }
}

bool binary_search_recursive(int* values, int key, int start, int end) {
    if (start > end) return false;
    int mid = (start + end) / 2;
    if (values[mid] == key) return true;
    else if (key < values[mid])
        return binary_search_recursive(values, key, start, mid - 1);
    else
        return binary_search_recursive(values, key, mid + 1, end);
}

bool binary_search(int* values, int key, int size) {
    return binary_search_recursive(values, key, 0, size - 1);
}

void createBinaryFile(const string& filename) {
    srand((unsigned)time(0));
    int size = 200;
    int* data = new int[size];
    for (int i = 0; i < size; i++)
        data[i] = rand() % 1000;
    writeBinary(filename, data, size);
    delete[] data;
}

void writeBinary(const string& filename, int* values, int length) {
    ofstream fout(filename.c_str(), ios::binary);
    fout.write((char*)&length, sizeof(length));
    fout.write((char*)values, sizeof(int) * length);
    fout.close();
}

// =====================================================
// MAIN FUNCTION
// =====================================================
int main() {
    string filename = "data.bin";
    BinaryReader reader(filename);
    int* data = reader.getValues();
    int size = reader.getSize();

    if (size <= 0) {
        cout << "No valid data to analyze.
";
        return 1;
    }

    cout << "=== Signature Assignment Final Output ===\n\n";

    StatisticsAnalyser stats(data, size);
    cout << stats.analyze() << endl << endl;

    DuplicatesAnalyser dup(data, size);
    cout << dup.analyze() << endl;

    MissingAnalyser miss(data, size);
    cout << miss.analyze() << endl;

    SearchAnalyser search(data, size);
    cout << search.analyze() << endl;

    cout << "\nProgram completed successfully.\n";
    return 0;
}
