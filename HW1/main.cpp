#include <iostream>
#include <queue>
#include <stack>
#include <fstream>
#include <vector>
#include <cmath>
#include <chrono>

using namespace std;
using namespace chrono;

// method for creating graph.txt. Self-explanatory.
void graphFile(vector<vector<int>> &graph, int n) {
    fstream gOutFile ("graph.txt", ios::out);
    gOutFile << "Graph:\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            gOutFile << graph[i][j] << ' ';
        }
        gOutFile << '\n';
    }
    gOutFile.close();
    return;
}
//calculations and creation of bfs.txt
void bfs(vector<vector<int>> &graph, int n, int source, int target) {
    queue<int> q;                       //queue for the breadth, the order of the ball being sent
    vector<int> visited(n, 0);          //visited kids
    vector<int> distance(n, -1);        //distance of i-th kid from the source
    vector<int> previous(n, -1);        //sender of the kids

    q.push(source);                     
    visited[source] = 1; //visited
    distance[source] = 0; //distance from source is 0

    while (!q.empty()) {
        int current = q.front(); //the current kid
        q.pop();
        
        for (int i = 0; i < n; i++) {
            if(graph[current][i] && !visited[i]) { //if the current kid can send to i-th kid and i-th kid is not alread "visited"
                q.push(i); //put i-th kid in queue; will be sent later
                visited[i] = 1; //i-th kid is now visited (shortest path from source)
                distance[i] = distance[current] + 1; //i's distance from source is 1 more than current's distance 
                previous[i] = current; //i's sender is current kid
                if (i == target) { //if i is target, start file creation
                    fstream bfsOutFile("bfs.txt", ios::out);
                    bfsOutFile << "BFS:\n" << distance[i] << ' ' << source; //write in file
                    stack<int> passSeq; //passing sequence
                    current = target;
                    while(current != source) { //fill the stack with sequence of kids who passed
                        passSeq.push(current);
                        current = previous[current];
                    }
                    while(!passSeq.empty()){
                        bfsOutFile << "->" << passSeq.top();
                        passSeq.pop();
                    }
                    bfsOutFile.close();
                    return;
                }
            }
        }
    }
    return;
}
//recursive function nested in dfs function. Only does recursion with updates
bool nextChild(int current, vector<vector<int>> &graph, int n, int source, vector<int> &visited, vector<int> &previous, vector<int> &passSeq) {
    visited[current] = 1; // visited
    for (int i = 0; i < n; i++) {
        if (graph[current][source] == 1 && previous[current] != source) { 
            //if the current kid can send the ball to source kid and current kid's sender is not source (cycle length != 2)
            while (current != source) {
                passSeq.push_back(current); //fill the pass sequence with the reverse order of senders
                current = previous[current];
            }
            passSeq.push_back(source);
            return true;
        }
        if (graph[current][i] && !visited[i]) { //if the current kid can send to i-th kid and i-th kid is not alread "visited"
            previous[i] = current; //i's sender is current kid
            if (nextChild(i, graph, n, source, visited, previous, passSeq)) { //call recursion, if cycle is found, return true
                return true;
            } //if no recursion, try with different i's
        }
    }
    return false;
}
//file creation
void dfs(vector<vector<int>> &graph, int n, int source) {
    vector<int> visited(n, 0);              //visited kids
    vector<int> previous(n, -1);            //sender of the kids
    vector<int> passSeq;                    //passing sequence
    fstream dfsOutFile ("dfs.txt", ios::out); //create file
    dfsOutFile << "DFS:\n";
    if (!nextChild(source, graph, n, source, visited, previous, passSeq)) { //if nextChild returns false
        dfsOutFile << "-1 "; //write -1 and return
        dfsOutFile.close();
        return;
    }
    dfsOutFile << passSeq.size() << " "; //else, write the reverse passSeq
    for (int i = passSeq.size()-1; i >= 0; i--) {
        dfsOutFile << passSeq[i] << "->";
    }
    dfsOutFile << source;
    dfsOutFile.close();
    return;
}

int main(int argc, char* argv[]){
    try {
        if (argc < 2) { //if no file in the command line
            throw(0);
        }
        auto start = high_resolution_clock::now(); //start timer
        int n, s, t;
        fstream infile(argv[1], ios::in); //opening the inputted file
        if (infile.is_open()) {
            infile >> n >> s >> t;
            int dist;
            vector<int> x(n), y(n), p(n); //x and y positions and strength of i-th kids 
            vector<vector<int>> graph(n, vector<int>(n, 0)); //adjecency matrix initialized to 0s
            for (int i = 0; i < n; i++) {
                infile >> x[i] >> y[i] >> p[i]; //reading the kids' data
            }
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < i; j++) { //only doing once for every pair of kids. if one can send the ball, the
                                            //automatically can send it back
                    dist = pow(x[i] - x[j], 2) + pow(y[i] - y[j], 2);
                    if (dist <= p[i] && dist <= p[j]) { //calculating if kids can throw the ball
                        graph[i][j] = 1; //if i can send, j can send back
                        graph[j][i] = 1;
                    }
                }
            }
            graphFile(graph, n); // creating graph.txt
            bfs(graph, n, s, t); //calculating and creating bfs.txt
            dfs(graph, n, s); //calculating and creating dfs.txt
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