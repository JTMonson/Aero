// #include "multi_channel_FxLMS.h"
// #include <vector>
// #include <iostream>
// using namespace std;

// int main() {
//     // ===== 1. Initialize secondary path matrix ====
//     // CUNIT: Number of secondary sources/speakers
//     // R_NUM: Number of reference microphones
//     // LS: Length of the secondary parh filter
//     // Sec[k][j][l]: For speaker k and reference mic j, the l-th sample of path impulse response
//     // float SecMatrix[CUNIT][R_NUM][LS] = {
//     //     { // speaker 0
//     //         {1.0f, 0.0f, 0.0f, 0.0f}, // mic 0
//     //         {0.8f, 0.1f, 0.0f, 0.0f}  // mic 1
//     //     },
//     // };  

//     vector<vector<vector<float>>> SecMatrix = {
//         { // speaker 0
//             {1.0f, 0.0f, 0.0f, 0.0f}, // mic 0
//             {0.8f, 0.1f, 0.0f, 0.0f}  // mic 1
//         }
//     };
    
//     // ===== 2. Create multichannel controller =====
//     // mc: the "brain" object controlling all filters
//     // 0.01f: step size (mu) controlling how fast the filters adapt
//     // This constructor initializes one ControlFilter per speaker × mic pair
//     MultichannelFxLMS mc(SecMatrix, 0.01f); // step size = 0.01 - mu?

//     // ===== 3. Create reference and error buffers =====
//     // reference: holds the current samples from each reference microphone
//     // Er: holds the current error signals from error microphones (measure remaining noise)
//     // 0.0f: initilize all elements to 0
//     vector<float> reference(R_NUM, 0.0f); // 2 mics → vector size = 2
//     vector<float> Er(E_NUM, 0.0f);        // 1 error mic → vector size = 1

//     // ===== 4. Simulation loop =====
//     const int TOTAL_SAMPLES = 100;      // simulate 100 samples for now
//     for (int n = 0; n < TOTAL_SAMPLES; ++n) {

//         // --- 4a. Fill reference signals ---
//         // Here we just put dummy signals for simulation purposes
//         reference[0] = 0.1f;  // sample from mic 0
//         reference[1] = 0.05f; // sample from mic 1

//         // fake error signal
//         Er[0] = 0.05f;


//         // --- 4b. Update controller ---
//         //  - updates all internal ControlFilters using FxLMS algorithm
//         //  - computes anti-noise output for this sample
//         float anti_noise = mc.update(reference, Er);

//         // --- 4c. Output / store result ---
//         // For now, we just print it. Eventually this would go to a DAC or speaker
//         cout << anti_noise << endl;
//     }

//     return 0;
// }

#include "multi_channel_FxLMS.h"
#include <vector>
#include <iostream>
#include <cmath>      // for sin()
#include <iomanip>
using namespace std;

int main() {
    // ===== 1. Initialize secondary path matrix =====
    //  CUNIT: number of speakers
    //  R_NUM: number of reference microphones
    //  LS: length of secondary path impulse response
    vector<vector<vector<float>>> SecMatrix = {
        { // speaker 0
            {1.0f}, // mic0
            {1.0f}  // mic1
        }
    };

    // ===== 2. Create multichannel controller =====
    // mc: object controlling all filters
    // 0.01f: step size (mu)
    float mu = 0.0005f;
    MultichannelFxLMS mc(SecMatrix, mu);

    // ===== 3. Create reference and error buffers =====
    vector<float> reference(R_NUM, 0.0f); // current sample from each reference mic
    vector<float> Er(E_NUM, 0.0f);        // current error signal(s)

    // ===== 4. Simulation loop =====
    const int TOTAL_SAMPLES = 100;
    const float freq0 = 0.05f; // frequency for mic 0 sine wave
    const float freq1 = 0.03f; // frequency for mic 1 sine wave

    // ----- Print Table Header -----
    cout << left 
        << setw(10) << "Sample"
        << setw(15) << "Noise"
        << setw(15) << "Anti-Noise"
        << setw(15) << "Error"
        << endl;

    cout << string(55, '-') << endl;

    for (int n = 0; n < TOTAL_SAMPLES; ++n) {
        // --- 4a. Generate dummy reference signals ---
        reference[0] = sin(2.0f * 3.14159f * freq0 * n);  // mic 0 sine wave
        reference[1] = sin(2.0f * 3.14159f * freq1 * n);  // mic 1 sine wave

        // --- 4b. Compute anti-noise ---
        float anti_noise = mc.update(reference, Er);

        // --- 4c. Update dummy error mic ---
        // For now, error is the reference minus anti-noise
        // This simulates measuring remaining noise
        float noise = 0.5f * (reference[0] + reference[1]);
        Er[0] = noise - anti_noise;

        // --- 4d. Output ---
        // cout << anti_noise << endl;
        // --- Print formatted row ---
        cout << left
            << setw(10) << n
            << setw(15) << fixed << setprecision(6) << noise
            << setw(15) << anti_noise
            << setw(15) << Er[0]
            << endl;

    }

    return 0;
}