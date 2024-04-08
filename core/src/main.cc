#include "cmdline.hpp"
#include "header.h"
#include "my_abc.h"


using namespace abc;
using namespace cmdline;
using namespace std;


parser CommPars(int argc, char * argv[]) {
    parser option;
    option.add<string>("initial", 'i', "path to initial circuit", false, "./input/benchmark/c880.aig");
    option.add<string>("outpPath", 'o', "path to generated circuits", false, "tmp");
    option.parse_check(argc, argv);
    return option;
}


void Generate(string& initial, string& outpPath, long& outputTime) {
    // create output path
    FixAndCreatePath(outpPath);

    // get network name
    assert(initial.find("/") != string::npos);
    assert(initial.find(".aig") != string::npos);
    string ntkName = initial.substr(initial.find_last_of("/") + 1);
    ntkName = ntkName.substr(0, ntkName.find(".aig"));
    cout << "Network name: " << ntkName << endl;

    // alias compress2rs "b -l; rs -K 6 -l; rw -l; rs -K 6 -N 2 -l; rf -l; rs -K 8 -l; b -l; rs -K 8 -N 2 -l; rw -l; rs -K 10 -l; rwz -l; rs -K 10 -N 2 -l; b -l; rs -K 12 -l; rfz -l; rs -K 12 -N 2 -l; rwz -l; b -l"
    string compress2rs[] = {
        "b -l",
        "rs -K 6 -l",
        "rw -l",
        "rs -K 6 -N 2 -l",
        "rf -l",
        "rs -K 8 -l",
        "b -l",
        "rs -K 8 -N 2 -l",
        "rw -l",
        "rs -K 10 -l",
        "rwz -l",
        "rs -K 10 -N 2 -l",
        "b -l",
        "rs -K 12 -l",
        "rfz -l",
        "rs -K 12 -N 2 -l",
        "rwz -l",
        "b -l"
    };

    // read initial circuit
    AbcMan abcMan;
    abcMan.ReadNet(initial);

    // iteratively simplify the circuit using sub-commands in compress2rs
    const int MAX_ITER = 1 << 20;
    const int CONV_ROUND = 200;
    deque<int> historySize(CONV_ROUND, INT_MAX);
    for (int iteration = 0; iteration < MAX_ITER; ++iteration) {
        int cmdId = 0;
        for (string& cmd : compress2rs) {
            // simplify
            abcMan.Comm(cmd);

            // write to file and accumulate output runtime
            auto fileName = outpPath + "/" + ntkName + "_iter" + to_string(iteration) + "_cmd" + to_string(cmdId) + ".aig";
            clock_t start = clock();
            abcMan.WriteNet(fileName);
            outputTime += clock() - start;

            int size = abcMan.GetArea();
            int depth = abcMan.GetDelay();
            cout << "Iteration " << iteration << ", Command " << cmdId << ": " << cmd << ", fileName = " << fileName << ", size = " << size << ", depth = " << depth << endl;

            // update historySize: push size to the back and pop the front
            historySize.push_back(size);
            historySize.pop_front();

            // check convergence: if size does not decrease for CONV_ROUND consecutive rounds, then stop
            bool isConverged = true;
            for (int i = 1; i < CONV_ROUND; ++i) {
                if (historySize[i] < historySize[i - 1]) {
                    isConverged = false;
                    break;
                }
            }
            if (isConverged) {
                cout << "Converged at iteration " << iteration << ", command " << cmdId << endl;
                return;
            } 

            ++cmdId;
        }
    }
}


int main(int argc, char * argv[]) {

    // start abc engine
    GlobStartAbc();

    // parse options
    parser option = CommPars(argc, argv);
    string initial = option.get<string>("initial");
    string outpPath = option.get<string>("outpPath");

    // generate circuits
    long outputTime = 0;
    auto startTime = clock();
    Generate(initial, outpPath, outputTime);
    auto totalTime = clock() - startTime;
    cout << "Total time = " << totalTime << " ms" << endl;
    cout << "Output AIG time = " << outputTime << " ms" << endl;
    cout << "Time w/o output AIG = " << totalTime - outputTime << " ms" << endl;

    // stop abc engine
    GlobStopAbc();
    
    return 0;
}