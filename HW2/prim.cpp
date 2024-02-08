#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <chrono>

using namespace std;
using namespace chrono;

class Compare { //if plows are same, lower index should be first
    public:
        bool operator() (pair<long long, long long> &a, pair<long long, long long> &b) {
        if (a.first == b.first) {
            return a.second > b.second;
        }
        else {
            return a.first < b.first;
        }
    }
};

void createGraph(vector<long long> &pounts, vector<vector<pair<long long, long long>>> &adjMatrix, float th) { //fill the matrix
    for (uint i = 0; i < pounts.size(); i++) {
        for (uint j = i + 1; j < pounts.size(); j++) {
            long long plow = abs(pounts[i] - pounts[j]); //plow is absolute difference of pount numbers
            if (plow <= th * ((pounts[i] + pounts[j])/2) && plow != 0) { //if plow less than or equal to th times mean of pounts, and is not zero, build road
                adjMatrix[i].push_back(make_pair(plow, j)); //add plows and city indexes to graph
                adjMatrix[j].push_back(make_pair(plow, i));
            }
        }
    }
}

int bQueueEmpty(vector<priority_queue<pair<long long, long long>, vector<pair<long long, long long>>, Compare>> bQueue) { //if there are no way of spreading anymore, i.e. all bQueue[i]s are empty
    for (uint i = 0; i < bQueue.size(); i++) {
        if (!bQueue[i].empty()) {
            return 0;
        }
    }
    return 1;
}

void prim(vector<priority_queue<pair<long long, long long>, vector<pair<long long, long long>>, Compare>> &bQueue, vector<vector<pair<long long, long long>>> &adjMatrix, vector<vector<long long>> &branches) {
    long long b = bQueue.size(); //number of bakeries
    long long c = adjMatrix.size(); //number of cities
    vector<int> visited(c, 0); //if a city is "visited" i.e. has bakery
    long long turn = 0; //bakeries spread turn by turn
    while (!bQueueEmpty(bQueue)) { //if there are still cities in any of the bakery`s priority queue
        while (!bQueue[turn].empty()) { //the bakery's priority queue isn't empty
            long long i = bQueue[turn].top().second; //first city's index in priority queue 
            bQueue[turn].pop(); //remove from priority queue
            if (visited[i]) { //if city is already taken, try again with other cities
                continue;
            }
            branches[turn].push_back(i); //add the city as a branch of current bakery
            visited[i] = 1; //city is "visited", so other cities won't be able to spread here
            for (auto e : adjMatrix[i]) { //check neighbours of newly added city
                long long plow = e.first;
                long long city = e.second;
                if (!visited[city]) { //if the neighbour is not "visited"
                    bQueue[turn].push({plow, city}); //add plow & city to priority queue for next iteration 
                }
            }
            break;
        }
        turn = (turn + 1) % b; //next bakery
    }
}

int main(int argc, char* argv[]) {
    try {
		if (argc < 2) { //if no file in the command line
            throw(0);
        }
		long long c, b, p, bs; //number of cities, number of bakeries, pount number, bakery source index 
        float th; //threshold value
        long long _; //unused data
        auto start = high_resolution_clock::now(); //start timer
        fstream infile(argv[1]); //opening the inputted file
        if (infile.is_open()) {
            infile >> c >> b >> th;
            vector<long long> pounts, bSources; //pounts of cities and bakery sources

            for (long long i = 0; i < b; i++) { //save bakery sources data
                infile >> bs;
                bSources.push_back(bs);
            }
            for (long long i = 0; i < c; i++) { //save pounts data
                infile >> _ >> _ >> p;
                pounts.push_back(p);
            }

            vector<vector<pair<long long, long long>>> adjMatrix(c, vector<pair<long long, long long>>(0)); //adjacency matrix for graph initialized to 0
            vector<priority_queue<pair<long long, long long>, vector<pair<long long, long long>>, Compare>> bQueue(b); //priority queue of bakeries, storing plow & city num
            vector<vector<long long>> branches(b, vector<long long>(0)); //branch city index of bakeries.

            createGraph(pounts, adjMatrix, th);

            for (uint i = 0; i < bSources.size(); i++) { //add the city itself to its priority queue with plow 0
                bQueue[i].push({0, bSources[i]});
            }

            prim(bQueue, adjMatrix, branches); //prim algorithm

            // to create output file and write the results into it
            fstream outfile ("prim.txt", ios::out);
            for (int i = 0; i < b; i++) {
                string id = "k" + to_string(i);
                outfile << id << " " << branches[i].size() << endl;
                // writing the results to prim file output
                for (uint j = 0; j < branches[i].size(); j++) { //for each branch of current bakery
                    if (j == branches[i].size()-1) {
                        outfile << branches[i][j];
                        break;
                    }
                    outfile << branches[i][j] << "->";
                }
                outfile << endl;
            }
            outfile.close();
            auto stop = high_resolution_clock::now(); //stop timer
            auto duration = duration_cast<milliseconds>(stop - start); //read timer
            cout << duration.count() << "ms\n";
        }
        else {
            throw(1); //inputted file could not be opened
        }
    }
    catch (int i) {
        if (i == 0) {
            cout << "No input file found\n";
        }
        else if (i == 1) {
            cout << "Could not open file\n";
        }
    }
	exit(0);
}