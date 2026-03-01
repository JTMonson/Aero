#include "control_filter.h"
using namespace std;

Control_filter::Control_filter() {
    for(int i = 0; i < FILTER_LEN; i++) {
        Wn[i] = 0.0f;
        Xn[i] = 0.0f;
        Xfn[i] = 0.0f;
    }
    for(int i = 0; i < SEC_LEN; i++)
        Sn[i] = 0.0f;

    En = 0.0f;
    Dn = 0.0f;
    Yn = 0.0f;
}

// Compute filtered reference Xfn = Xn * Sn (convolution over recent taps)   
void Control_filter::update_Xfn() {
    for(int i = 0; i < FILTER_LEN; i++) {
        Xfn[i] = 0.0f;
        for(int j = 0; j < SEC_LEN; j++) {
            if(i - j >= 0)
                Xfn[i] += Xn[i - j] * Sn[j];
        }
    }
}


// Compute anti-noise output y(n) = sum( w(n) * x(n) )
void Control_filter::compute_Yn() {
    Yn = 0.0f;
    for(int i = 0; i < FILTER_LEN; i++)
        Yn += Wn[i] * Xn[i];
}

// Update filter coefficients w(n+1) = w(n) + mu * e(n) * Xfn(n)
void Control_filter::compute_error() {
    En = Dn - Yn;
}

// Update filter coefficients w(n+1) = w(n) + mu * e(n) * Xfn(n)
void Control_filter::update_Wn() {
    for(int i = 0; i < FILTER_LEN; i++)
        Wn[i] += MU * En * Xfn[i];
}

// Run one step of the FxLMS algorithm
void Control_filter::step() {
    update_Xfn();       // Filter reference through secondary path
    compute_Yn();       // Compute current anti-noise output
    compute_error();    // Compute error at error mic
    update_Wn();        // Update filter coefficients
}