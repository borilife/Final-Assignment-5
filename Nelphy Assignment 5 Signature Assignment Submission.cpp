#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <random>
#include <map>
#include <algorithm>

using namespace std;

// Selection sort implementation
void selection_sort(int* values, int size) 
{
    for (int i = 0; i < size - 1; ++i) 
    {
        int minIdx = i;
        for (int j = i + 1; j < size; ++j) 
        {
            if (values[j] < values[minIdx]) minIdx = j;
        }
        if (minIdx != i) swap(values[i], values[minIdx]);
    }
}

// Binary search recursive helper signatures
bool binary_search_recursive(int* arr, int value, int left, int right) 
{
    if (left > right) return false;
    int mid = left + (right - left) / 2;
    if (arr[mid] == value) return true;
    if (value < arr[mid]) return binary_search_recursive(arr, value, left, mid - 1);
    return binary_search_recursive(arr, value, mid + 1, right);
}

// Binary search helper that user calls: returns true if value found
bool binary_search(int* arr, int value, int size) 
{
    if (size <= 0) return false;
    return binary_search_recursive(arr, value, 0, size - 1);
}

// BinaryReader: this block read/write ints from a binary file
class BinaryReader 
{
    int* values = nullptr;
    int size = 0;
public:
    BinaryReader(const string& filename) { readValues(filename); }
    ~BinaryReader() { delete [] values; }
    int* getValues() { return values; }
    int getSize() { return size; }
    void readValues(const string& filename)
     {
        // open file in binary mode and read all ints
        ifstream in(filename, ios::binary);
        if (!in) {
            // empty dataset
            values = nullptr; size = 0; return;
        }
        // get file size
        in.seekg(0, ios::end);
        streampos fileSize = in.tellg();
        in.seekg(0, ios::beg);
        size = static_cast<int>(fileSize / sizeof(int));
        values = new int[size];
        in.read(reinterpret_cast<char*>(values), size * sizeof(int));
    }
};

// Analyzer base class
class Analyzer 
{
protected:
    int* values;
    int size;
public:
    Analyzer(int* values_, int size_): values(nullptr), size(0) 
    {
        values = cloneValues(values_, size_);
        size = size_;
    }
    virtual ~Analyzer() { delete [] values; }
    // clones the array into a newly allocated array
    int* cloneValues(int* src, int n) 
    {
        if (n <= 0) return nullptr;
        int* dest = new int[n];
        for (int i = 0; i < n; ++i) dest[i] = src[i];
        return dest;
    }
    virtual string analyze() = 0;
};

class DuplicatesAnalyser : public Analyzer {
public:
    DuplicatesAnalyser(int* values_, int size_): Analyzer(values_, size_) {}
    string analyze() override 
    {
        if (!values || size <= 0) return string("Duplicates: none\n");
        // count frequencies
        map<int,int> freq;
        for (int i = 0; i < size; ++i) ++freq[values[i]];
        vector<int> duplicates;
        for (auto &p : freq) if (p.second > 1) duplicates.push_back(p.first);
        ostringstream out;
        out << "Duplicates (value:count): ";
        if (duplicates.empty()) out << "none";
        else {
            bool first = true;
            for (int v : duplicates) 
            {
                if (!first) out << ", ";
                out << v << ":" << freq[v];
                first = false;
            }
        }
        out << '\n';
        return out.str();
    }
};

class MissingAnalyser : public Analyzer 
{
public:
    MissingAnalyser(int* values_, int size_): Analyzer(values_, size_) {}
    string analyze() override 
    {
        if (!values || size <= 0) return string("Missing: none\n");
        // find missing numbers between min and max
        int mn = values[0], mx = values[0];
        for (int i = 1; i < size; ++i) { if (values[i] < mn) mn = values[i]; if (values[i] > mx) mx = values[i]; }
        vector<char> present(mx - mn + 1);
        for (int i = 0; i < size; ++i) present[values[i] - mn] = 1;
        ostringstream out;
        out << "Missing values between " << mn << " and " << mx << ": ";
        bool any = false;
        for (int v = mn; v <= mx; ++v) 
        {
            if (!present[v - mn]) 
            {
                if (any) out << ", ";
                out << v;
                any = true;
            }
        }
        if (!any) out << "none";
        out << '\n';
        return out.str();
    }
};

class SearchAnalyzer : public Analyzer
{
public:
    SearchAnalyzer(int* values_, int size_): Analyzer(values_, size_) 
    {
        // sort internal copy so binary_search can be used
        if (values && size > 0) selection_sort(values, size);
    }
    string analyze() override 
    {
        if (!values || size <= 0) return string("Search found 0 values\n");
        // this will generate 100 random ints in 0..999 and count how many exist
        std::random_device rd; std::mt19937 gen(rd()); std::uniform_int_distribution<> dist(0,999);
        int found = 0;
        for (int i = 0; i < 100; ++i) 
        {
            int v = dist(gen);
            if (binary_search(values, v, size)) ++found;
        }
        ostringstream out; out << "Search found " << found << " values (out of 100)\n";
        return out.str();
    }
};

class StatisticsAnalyser : public Analyzer 
{
public:
    StatisticsAnalyser(int* values_, int size_): Analyzer(values_, size_) {}
    string analyze() override 
    {
        if (!values || size <= 0) return string("No data\n");
        // sort data first
        selection_sort(values, size);
        int mn = values[0];
        int mx = values[size-1];
        // mean
        long long sum = 0;
        for (int i = 0; i < size; ++i) sum += values[i];
        double mean = static_cast<double>(sum) / size;
        // median
        double median = 0.0;
        if (size % 2 == 1) median = values[size/2];
        else median = (values[size/2 - 1] + values[size/2]) / 2.0;
        // mode: most frequent value, if tie choose first encountered in sorted order
        int mode = values[0];
        int modeCount = 1;
        int cur = values[0];
        int curCount = 1;
        for (int i = 1; i < size; ++i) 
        {
            if (values[i] == cur) ++curCount;
            else 
            {
                if (curCount > modeCount) { modeCount = curCount; mode = cur; }
                cur = values[i]; curCount = 1;
            }
        }
        if (curCount > modeCount) { modeCount = curCount; mode = cur; }

        ostringstream out;
        out << "Min=" << mn << ", Max=" << mx << ", Mean=" << mean << ", Median=" << median << ", Mode=" << mode << " (count=" << modeCount << ")\n";
        return out.str();
    }
};

// Helper: write a vector<int> as binary ints to a file
void write_binary_file(const string& filename, const vector<int>& data) 
{
    ofstream out(filename, ios::binary);
    out.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(int));
}

int main() 
{
   
    const string fname = "sample_data.bin";
    const int N = 200;
    vector<int> data(N);
    random_device rd; mt19937 gen(rd()); uniform_int_distribution<> dist(0, 999);
    for (int i = 0; i < N; ++i) data[i] = dist(gen);
    write_binary_file(fname, data);

    // Read using BinaryReader
    BinaryReader reader(fname);
    int* values = reader.getValues();
    int sz = reader.getSize();
    if (sz == 0) 
    {
        cout << "No data read from file\n"; return 0;
    }

    // Run analyzers
    DuplicatesAnalyser da(values, sz);
    MissingAnalyser ma(values, sz);
    SearchAnalyzer sa(values, sz);
    StatisticsAnalyser sta(values, sz);

    cout << "Duplicates analysis:\n" << da.analyze();
    cout << "Missing analysis:\n" << ma.analyze();
    cout << "Search analysis:\n" << sa.analyze();
    cout << "Statistics analysis:\n" << sta.analyze();

    return 0;
}
