#ifndef MULTI_CHANNEL_FXLMS_H
#define MULTI_CHANNEL_FXLMS_H

#include "control_filter.h"
#include <vector>
using namespace std;

#define R_NUM 2   // Number of reference microphones
#define CUNIT 1   // Number of speakers
#define LS 4      // Length of secondary path filter
#define LEN 16    // Length of control filter
#define E_NUM 1   // Number of error microphones

class MultichannelFxLMS {
public:
    vector<vector<ControlFilter>> filters; // [CUNIT][R_NUM]

    MultichannelFxLMS(const vector<vector<vector<float>>>& SecMatrix, float mu);

    float update(const vector<float>& reference, vector<float>& Er);
};

#endif