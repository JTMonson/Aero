#ifndef CONTROL_FILTER_H
#define CONTROL_FILTER_H

#include <vector>
using namespace std;

class ControlFilter {
public:
    vector<float> Wc;                 // Control filter coefficients
    vector<float> Xd;                 // Reference delay line
    vector<float> Xf;                 // Filtered reference
    vector<float> Yd;                 // Control output delay line
    vector<float> Fd;                 // Filtered coefficients
    vector<vector<float>> SecMatrix;  // Secondary path [LS x E_NUM]

    int Len;      // Filter length
    int LS;       // Secondary path length
    int E_NUM;    // Number of error microphones
    float mu;     // Step size

    ControlFilter() {} // default constructor
    ControlFilter(const vector<float>& sec, int len, int ls, int e_num, float mu);

    float generator_antinoise(float xin, vector<float>& Er);
};

#endif