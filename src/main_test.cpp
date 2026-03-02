#include <iostream>
#include <iomanip> // for setw, setprecision
#include <cmath>
#include "control_filter.h"

#define NUM_SAMPLES 500

using namespace std;

int main() { 
    // ===== 1. Define secondary path (speaker -> error mic) =====
    float sec_path[SEC_LEN] = {0.2f, 0.5f, 0.2f, 0.1f};

    // Create FxLMS controller
    Control_filter ctrl;
    for (int i = 0; i < SEC_LEN; i++)
        ctrl.Sn[i] = sec_path[i];

    // ===== 2. Generate reference signal (the noise we want to cancel) =====
    float reference[NUM_SAMPLES];
    for (int n = 0; n < NUM_SAMPLES; n++)
        reference[n] = sin(2.0f * 3.14159f * n / 20.0f) * 10.0f; // make bigger for testing

    // ===== 3. Print table header =====
    cout << left; // left-align headers
    cout << setw(12) << "Sample n"
        << right << setw(20) << "Anti-Noise"
         << setw(20) << "Reference Noise" 
         << setw(20) << "Error" 
         << endl;

    cout << string(12 + 20 + 20 + 20, '-') << endl;

    // ===== Main FxLMS Loop =====
    for (int n = 0; n < NUM_SAMPLES; n++) {
        // --- Update the reference delay line x(n) ---
        for (int i = FILTER_LEN - 1; i > 0; i--)
            ctrl.Xn[i] = ctrl.Xn[i - 1];
        ctrl.Xn[0] = reference[n]; // newest sample at index 0

        // --- Set desired signal at error mic ---
        ctrl.Dn = reference[n];

        // --- Run one FxLMS step ---
        ctrl.step();

        // --- Print row ---
        cout << setw(12) << n
             << right << setw(20) << fixed << setprecision(4) << ctrl.Yn
             << setw(20) << fixed << setprecision(4) << ctrl.Dn
             << setw(20) << fixed << setprecision(4) << ctrl.En
             << endl;
        cout << left; // reset left-align for next row if needed
    }

    return 0;
}