#include "control_filter.h"
#include <algorithm>
using namespace std;

ControlFilter::ControlFilter(const vector<float>& sec, int len, int ls, int e_num, float mu)
    : Len(len), LS(ls), E_NUM(e_num), mu(mu)
{
    // --- Initialize internal buffers ---
    Wc.resize(Len, 0.0f);   // control filter coefficients
    Xd.resize(Len, 0.0f);   // reference delay line
    Xf.resize(LS, 0.0f);    // filtered reference
    Yd.resize(LS, 0.0f);    // control output delay line
    Fd.resize(Len * E_NUM, 0.0f); // filtered coefficients

    // --- Initialize secondary path matrix ---
    SecMatrix.resize(LS, vector<float>(E_NUM, 0.0f));

    // Optional: copy the input sec array into SecMatrix
    for(int i = 0; i < LS; ++i)
        for(int j = 0; j < E_NUM; ++j)
            SecMatrix[i][j] = sec[i]; // Adjust indexing if needed
}

float ControlFilter::generator_antinoise(float xin, vector<float>& Er) {
    // Shift delay lines
    rotate(Xd.rbegin(), Xd.rbegin() + 1, Xd.rend());
    Xd[0] = xin;

    rotate(Xf.rbegin(), Xf.rbegin() + 1, Xf.rend());
    Xf[0] = xin;

    // Update Wc
    for (int i = 0; i < Len; ++i) {
        float sum = 0.0f;
        for (int j = 0; j < E_NUM; ++j)
            sum += Fd[i * E_NUM + j] * Er[j];
        Wc[i] -= mu * sum;
    }

    // Control output
    float y_o = 0.0f;
    for (int i = 0; i < Len; ++i)
        y_o += Wc[i] * Xd[i];

    // Shift Yd
    rotate(Yd.rbegin(), Yd.rbegin() + 1, Yd.rend());
    Yd[0] = y_o;

    // Compute anti-noise
    float y_anti = 0.0f;
    for (int j = 0; j < E_NUM; ++j)
        for (int i = 0; i < LS; ++i)
            y_anti += SecMatrix[i][j] * Yd[i];

    // Update Fd = SecMatrix' * Xf
    for (int i = Len - 1; i > 0; --i)
        for (int j = 0; j < E_NUM; ++j)
            Fd[i * E_NUM + j] = Fd[(i - 1) * E_NUM + j];

    for (int j = 0; j < E_NUM; ++j) {
        float sum = 0.0f;
        for (int i = 0; i < LS; ++i)
            sum += SecMatrix[i][j] * Xf[i];
        Fd[j] = sum;
    }

    return y_anti;
}