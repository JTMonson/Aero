#include "multi_channel_FxLMS.h"
using namespace std;

MultichannelFxLMS::MultichannelFxLMS(const vector<vector<vector<float>>>& SecMatrix, float mu) {
    filters.resize(CUNIT);
    for (int i = 0; i < CUNIT; ++i) {
        filters[i].resize(R_NUM);
        for (int j = 0; j < R_NUM; ++j)
            filters[i][j] = ControlFilter(SecMatrix[i][j], LEN, LS, E_NUM, mu);
    }
}

float MultichannelFxLMS::update(const vector<float>& reference, vector<float>& Er) {
    float total_output = 0.0f;
    for (int i = 0; i < CUNIT; ++i)
        for (int j = 0; j < R_NUM; ++j)
            total_output += filters[i][j].generator_antinoise(reference[j], Er);
    return total_output;
}