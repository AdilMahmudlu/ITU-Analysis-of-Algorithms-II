#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <fstream>
#include <chrono>

using namespace std;
using namespace chrono;

// stores the centre of polygon (It is made
// global because it is used in compare function)
pair<long long, long long> center;

int quadrant(pair<long long, long long> a) { //returns quadrant of the coordinate
    return a.first >= 0 ? (a.second >= 0 ? 1 : 4) : (a.second >= 0 ? 2 : 3);
}

int orientation(pair<long long, long long> a, pair<long long, long long> b, pair<long long, long long> c) {
	//using cross product, standing on a looking at b, if c is on the left, return 1, on the right, return -1, 0 if on the line.  
	long long side = (b.first - a.first)*(c.second - a.second) - (c.first - a.first)*(b.second - a.second);

	if (side == 0) {
        return 0; //on the line
    }
	if (side > 0) { //standing on a looking at b, c is on the left side 
        return 1;
    }
	return -1; //standing on a looking at b, c is on the right side
}

bool compare(pair<long long, long long> a, pair<long long, long long> b) { //function used in sort()
	pair<long long, long long> aa = make_pair(a.first - center.first, a.second - center.second); //vector from center to point
	pair<long long, long long> bb = make_pair(b.first - center.first, b.second - center.second); 

	int aq = quadrant(aa); //find the quadrants of vectors
	int bq = quadrant(bb);

	if (aq != bq) { //if different quadrants, select based on quadrant order
        return (aq < bq);
    }
	return (aa.second*bb.first < bb.second*aa.first); //else, select counter-clockwise order
}

vector<pair<long long, long long>> merge(vector<pair<long long, long long>> a, vector<pair<long long, long long>> b) { //find upper and lower tangents
	int na = a.size(), nb = b.size(); // number of points in polygon a & b
	int ra = 0, lb = 0; //righmost index of a & leftmost index of b
	for (int i = 1; i < na; i++) { //find the rightmost point of a
		if (a[i].first > a[ra].first) {
            ra = i;
        }
    }
	for (int i = 1; i < nb; i++) { //find the leftmost point of b
		if (b[i].first < b[lb].first) {
	    	lb = i;
        }
    }

	int inda = ra, indb = lb; //indexes of points on a & b
	int done = 0;
	while (!done) { //finding upper tangent
		done = 1;
		while (orientation(b[indb], a[inda], a[(inda+1)%na]) <= 0) { 
			//a[(inda+1)%na] is the node after a[inda]. if on the right side, choose that node
			inda = (inda+1) % na; //next node
        }
		while (orientation(a[inda], b[indb], b[(nb+indb-1)%nb]) >= 0) {
			//b[(nb + indb-1)%nb] is the node before b[indb]. if on the left side, choose that node
			indb = (nb+indb-1) % nb; //previous node
			done = 0; //we changed indb, so we need to iterate the loop
		}
		//if we didn't change indb, then we found the upper tangent. while loop stops
	}

	int uppera = inda, upperb = indb; //save upper tangent point indexes
	inda = ra, indb = lb;
	done = 0;
	while (!done) { //finding the lower tangent, similar to upper tangent
		done = 1;
		while (orientation(a[inda], b[indb], b[(indb+1)%nb]) <= 0) {
			//b[(indb+1)%nb] is the node after b[indb]. if on the right side, choose that node
			indb = (indb+1) % nb;
        }
		while (orientation(b[indb], a[inda], a[(na+inda-1)%na]) >= 0) {
			//a[(na+inda-1)%na] is the node before a[inda]. if on the left side, choose that node
			inda = (na+inda-1) % na;
			done = 0;
		}
	}

	int lowera = inda, lowerb = indb; //save lower tangent point indexes
	vector<pair<long long, long long>> hull; //convex hull
	int ind = uppera; //a's point index on upper tangent
	hull.push_back(a[uppera]);
	while (ind != lowera) {
		ind = (ind+1)%na; //next node
		hull.push_back(a[ind]);
	}

	ind = lowerb; //b's point index on lower tangent
	hull.push_back(b[lowerb]);
	while (ind != upperb) {
		ind = (ind+1)%nb; //next node
		hull.push_back(b[ind]);
	}
	
	int leftmost = 0; //trying to find leftmost index
	for (int i = 1; i < (int)hull.size(); i++) {
		if (hull[i].first < hull[leftmost].first) {
	    	leftmost = i;
        }
    }
	rotate(hull.begin(), hull.begin()+leftmost, hull.end()); //rotate circularly to start from leftmost point
	return hull;
}

vector<pair<long long, long long>> bruteForce(vector<pair<long long, long long>> a) { //if there are less than 6 points, we find the convex hull using brute force
	set<pair<long long, long long>> s; //points of the hull, using set to avoid duplicates
	int m = a.size();
	for (int i = 0; i < m; i++) { //take a point
		for (int j = i + 1; j < m; j++) { //take another point
			int pos = 0;
			int neg = 0;
			for (int k = 0; k < m; k++) {
				if (orientation(a[i], a[j], a[k]) <= 0) {
                    neg++;
                }
				if (orientation(a[i], a[j], a[k]) >= 0) {
					pos++;
                }
			}
			if (pos == m || neg == m) { //if all points are in one side of the line, then line is an edge
				s.insert(a[i]);
				s.insert(a[j]);
			}
		}
	}
	vector<pair<long long, long long>> hull; //initrblizing vector from set of unique points
	for (pair<long long, long long> e:s) {
        hull.push_back(e);
    }

	center = {0, 0}; //defined in global scope to be used in multiple functions
	int n = hull.size();
	for (int i = 0; i < n; i++) {
		center.first += hull[i].first; //summing up all the points to find center * n
		center.second += hull[i].second;
		hull[i].first *= n; //multiplying all the points by n
		hull[i].second *= n;
	}
	sort(hull.begin(), hull.end(), compare); //sorting points*n around the center*n in counter-clockwise
	for (int i = 0; i < n; i++) {
		hull[i] = make_pair(hull[i].first/n, hull[i].second/n); //reverting points*n back to points
    }
	int leftmost = 0; //trying to find leftmost index
	for (int i = 1; i < n; i++) {
		if (hull[i].first < hull[leftmost].first) {
	    	leftmost = i;
        }
    }
	rotate(hull.begin(), hull.begin()+leftmost, hull.end()); //rotate circularly to start from leftmost point 
	return hull;
}

vector<pair<long long, long long>> partition(vector<pair<long long, long long>> a) {
	int n = a.size();
	if (n <= 5) { //if there are less than 6 points, function uses brute force to find convex hull
		return bruteForce(a);
    }
	vector<pair<long long, long long>> left, right; //left and right half of the points
	for (int i = 0; i < n/2; i++) { //left half of the points
		left.push_back(a[i]);
    }
	for (int i = n/2; i < n; i++) { //right half of the points
		right.push_back(a[i]);
    }
	vector<pair<long long, long long>> leftSide = partition(left); //reindsion for left and right side of points to find convex hull
	vector<pair<long long, long long>> rightSide = partition(right);

	return merge(leftSide, rightSide); //merging left and right convex hulls
}

int main(int argc, char* argv[]) {
	try {
		if (argc < 2) { //if no file in the command line
            throw(0);
        }
        auto start = high_resolution_clock::now(); //start timer
		fstream infile(argv[1], ios::in); //opening the inputted file
		if (infile.is_open()) {
			long long c, b, x, y; // number of cities, number of bakeries, and coordinates x & y
			double _; //unused data
			vector<pair<long long, long long>> coords;
			map<pair<long long, long long>, int> indexes;
            infile >> c >> b >> _;
			for (int i = 0; i < b; i++) {
				infile >> _;
			}
            for (int i = 0; i < c; i++) {
				infile >> x >> y >> _;
				coords.push_back(make_pair(x, y));
				indexes.insert(make_pair(make_pair(x, y), i));
			}
			sort(coords.begin(), coords.end()); //sort by x
			vector<pair<long long, long long>> hull = partition(coords);
			fstream outfile ("convex.txt", ios::out);
			int h = hull.size();
			outfile << h << endl;
			for (int i = 0; i < h; i++) {
				outfile << indexes[hull[i]] << "->";
			}
			outfile << indexes[hull[0]] << endl;
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
