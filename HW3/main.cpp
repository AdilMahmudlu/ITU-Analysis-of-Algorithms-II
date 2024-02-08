#include <iostream>
#include <string.h>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <algorithm>

using namespace std;

int timeToInt (string time) { // hh:mm to integers
    int h, m;
    if (time.size() == 4) {
        h = stoi(time.substr(0, 1));
        m = stoi(time.substr(2, 2));
    }
    else {
        h = stoi(time.substr(0, 2));
        m = stoi(time.substr(3, 2));
    }
    return 60*h+m;
}

int dateToInt(string date) { // dd.mm to integers
    int m, d;
    if (date.size() == 4) {
        d = stoi(date.substr(0, 1));
        m = stoi(date.substr(2, 2));
    }
    else {
        d = stoi(date.substr(0, 2));
        m = stoi(date.substr(3, 2));
    }
    return 31*m+d;
}

class Session { //sessions are a few hours long
    public:
        string salonName;
        int startMinutes;
        int endMinutes;
        string startTime;
        string endTime;
        long long capacity;
        Session() {};
        Session(string salonName, long long capacity, pair<string, string> times);
};

Session::Session(string salonName, long long capacity, pair<string, string> times) {
    this->salonName = salonName;
    this->capacity = capacity;
    this->startTime = times.first;
    this->endTime = times.second;
    this->startMinutes = timeToInt(startTime);
    this->endMinutes = timeToInt(endTime);
}

class Event { //events are a few days long
    public:
        string placeName;
        int startDay;
        int endDay;
        string startDate;
        string endDate;
        long long capacity;
        Event() {};
        Event(string placeName, long long capacity, pair<string, string> dates);
};

Event::Event(string placeName, long long capacity, pair<string, string> dates) {
    this->placeName = placeName;
    this->startDate = dates.first;
    this->endDate = dates.second;
    this->startDay = dateToInt(startDate);
    this->endDay = dateToInt(endDate);
    this->capacity = (endDay-startDay)*capacity;
}

class Salon {
    public:
        string salonName;
        vector<pair<string, string>> availableHours;
        long long capacity;
        Salon() {};
        Salon(string salonName, long long capacity);
};

Salon::Salon(string salonName, long long capacity) {
    this->salonName = salonName;
    this->capacity = capacity;
}

class Place {
    public:
        string name;
        vector<pair<string, string>> availableDays;
        unordered_map<string, Salon> salons;
        vector<string> salonNames;
        long long maxCapacity;
        vector<Session> selectedSessions;
        Place() {};
        Place(string name);
};

Place::Place(string name) {
    this->name = name;
}

class Asset {
    public:
        string name;
        long long price;
        double value;
        Asset() {};
        Asset(string name, long long price, double value);
};

Asset::Asset(string name, long long price, double value) {
    this->name = name;
    this->price = price;
    this->value = value;
}


bool compareSessions (Session &s1, Session &s2) {
    return (s1.endMinutes < s2.endMinutes) || ((s1.endMinutes == s2.endMinutes) && (s1.startMinutes < s2.endMinutes));
}

bool compareEvents (Event &e1, Event &e2) {
    return (e1.endDay < e2.endDay) || ((e1.endDay == e2.endDay) && (e1.startDay < e2.startDay));
}

//function to add salons and places from capacity file
void capacityToPlaces(unordered_map<string, Place> &places, vector<string> &placeNames, string placeName, string salonName, long long capacity) {
    if (places.find(placeName) == places.end()) { //if a place with this name does not exist, create one
        Place newPlace(placeName);
        places[placeName] = newPlace;
        placeNames.push_back(placeName);
    }
    //add salon to the place
    Salon newSalon(salonName, capacity);
    places[placeName].salons[salonName] = newSalon;
    places[placeName].salonNames.push_back(salonName);
}

//function to add salon schedules to the corresponding salons
void dailyScheduleToPlaces(unordered_map<string, Place> &places, string placeName, string salonName, string startTime, string endTime) {
    places[placeName].salons[salonName].availableHours.push_back({startTime, endTime});
}

//function to add place schedules to the corresponding places
void availabilityIntervalsToPlaces(unordered_map<string, Place> &places, string placeName, string startDate, string endDate) {
    places[placeName].availableDays.push_back({startDate, endDate});
}

//find the previous non-overlapping session with binary search
int findSessionBefore(vector<Session> &sessions, int i) {
    int low = 0, high = i-1;
    while (low <= high) {
        int mid = (low+high)/2;
        if (sessions[mid].endMinutes <= sessions[i].startMinutes) {
            if (sessions[mid+1].endMinutes <= sessions[i].startMinutes) {
                low = mid+1;
            }
            else {
                return mid;
            }
        }
        else {
            high = mid-1;
        }
    }
    return -1; //could not find
}

//find the previous non-overlapping event with binary search
int findEventBefore(vector<Event> &events, int i) {
    int low = 0, high = i-1;
    while (low <= high) {
        int mid = (low+high)/2;
        if (events[mid].endDay <= events[i].startDay) {
            if (events[mid+1].endDay <= events[i].startDay) {
                low = mid+1;
            }
            else {
                return mid;
            }
        }
        else {
            high = mid-1;
        }
    }
    return -1; //could not find
}

//interval scheduling for best_for_eachplace
long long timeIntervalScheduling(vector<Session> &sessions, vector<Session> &selectedSessions) {
    int s = sessions.size();
    sort(sessions.begin(), sessions.end(), compareSessions); //sort by end time
    vector<long long> maxCapacity(s, 0); //memoization
    vector<vector<Session>> sessionSequence(s, vector<Session>(0)); //keep sequences of sessions
    
    maxCapacity[0] = sessions[0].capacity;
    sessionSequence[0].push_back(sessions[0]);

    for (int i = 1; i < s; i++) {
        int p = findSessionBefore(sessions, i); //previous non-overlapping session
        
        int capacityWithCurrent = sessions[i].capacity + (p==-1 ? 0 : maxCapacity[p]); //if has p, add to its maxCap, not, dont add anything
        if (capacityWithCurrent > maxCapacity[i-1]) { //if current sequence is better
            if (p != -1) {
                sessionSequence[i] = sessionSequence[p]; //if no previous session
            }
            sessionSequence[i].push_back(sessions[i]); //add session to sequence
            maxCapacity[i] = capacityWithCurrent; //add capacity to maxCap
        }
        else { //else, best sequence is previous sequence
            sessionSequence[i] = sessionSequence[i-1];
            maxCapacity[i] = maxCapacity[i-1];
        }
    }
    selectedSessions = sessionSequence[s-1];
    return maxCapacity[s-1];
}


//interval scheduling for best_tour. Same with timeIntervalScheduling, so dont need explanation
int dateIntervalScheduling (vector<Event> &events, vector<Event> &selectedEvents) {
    int e = events.size();
    sort(events.begin(), events.end(), compareEvents);
    vector<long long> maxCapacity(e, 0);
    vector<vector<Event>> eventSequence(e, vector<Event>(0));
    
    maxCapacity[0] = events[0].capacity;
    eventSequence[0].push_back(events[0]);

    for (int i = 1; i < e; i++) {
        int p = findEventBefore(events, i);
        
        int capacityWithCurrent = events[i].capacity + (p==-1 ? 0 : maxCapacity[p]);
        if (capacityWithCurrent > maxCapacity[i-1]) {
            if (p != -1) {
                eventSequence[i] = eventSequence[p];
            }
            eventSequence[i].push_back(events[i]);
            maxCapacity[i] = capacityWithCurrent;
        }
        else {
            eventSequence[i] = eventSequence[i-1];
            maxCapacity[i] = maxCapacity[i-1];
        }
    }
    selectedEvents = eventSequence[e-1];
    return maxCapacity[e-1];
}

//knapsack for upgrade_list
double knapsack(int totalRevenue, vector<Asset> &assets, vector<Asset> &selectedAssets) {
    int a = assets.size();
    vector<vector<double>> K(totalRevenue+1, vector<double>(a+1, 0)); //table
    for (int x = 1; x <= totalRevenue; x++) { //as money increases
        for (int j = 1; j <= a; j++) { //as we go through items
            K[x][j] = K[x][j-1];
            if (assets[j-1].price <= x) { //if can buy current item
                K[x][j] = max(K[x][j], K[x-assets[j-1].price][j-1] + assets[j-1].value); //select max of buying new item or taking previous max
            }
        }
    }
    int x = totalRevenue, j = a; //find the selectedAssets
    while (j > 0 && x > 0) {
        if (K[x][j] != K[x][j-1]) { //if there's change in value from previous item size
            selectedAssets.push_back(assets[j-1]); //it means we bought the item
            x -= assets[j-1].price; //decrease money
        }
        j--;
    }
    return K[totalRevenue][a];
}


int main(int argc, char* argv[]) {
    //a bunch of files opened for R/W
    string inputFolder = "./inputs/case_";
    string outputFolder = "./outputs";
    inputFolder += argv[1];
    string capacityFileName = inputFolder + "/capacity.txt";
    string dailyScheduleFileName = inputFolder + "/daily_schedule.txt";
    string availabilityIntervalsFileName = inputFolder + "/availability_intervals.txt";
    string assetsFileName = inputFolder + "/assets.txt";
    string bestForEachPlaceFileName = outputFolder + "/my_best_for_each_place.txt";
    string bestTourFileName = outputFolder + "/my_best_tour.txt";
    string upgradeListFileName = outputFolder + "/my_upgrade_list.txt";
    fstream capacityFile(capacityFileName, ios::in);
    fstream dailyScheduleFile(dailyScheduleFileName, ios::in);
    fstream availabilityIntervalsFile(availabilityIntervalsFileName, ios::in);
    fstream assetsFile(assetsFileName, ios::in);
    fstream bestForEachPlaceFile(bestForEachPlaceFileName, ios::out);
    fstream bestTourFile(bestTourFileName, ios::out);
    fstream upgradeListFile(upgradeListFileName, ios::out);
    //map of places with their names and vector of their names
    unordered_map<string, Place> places;
    vector<string> placeNames;

    if (capacityFile.is_open()) {
        string _, placeName, salonName;
        long long capacity;
        capacityFile >> _ >> _ >> _; //first line not needed
        while (capacityFile >> placeName >> salonName >> capacity) {
            capacityToPlaces(places, placeNames, placeName, salonName, capacity); //call to fill the places
        }
    }
    else {
        cout << capacityFileName << " could not be opened\n";
        exit(-1);
    }

    if (dailyScheduleFile.is_open()) {
        string _, placeName, salonName, startsAt, endsAt;
        dailyScheduleFile >> _ >> _ >> _ >> _; //first line not needed
        while (dailyScheduleFile >> placeName >> salonName >> startsAt >> endsAt) {
            dailyScheduleToPlaces(places, placeName, salonName, startsAt, endsAt); //call to fill the places
        }
    }
    else {
        cout << dailyScheduleFileName << " could not be opened\n";
        exit(-1);
    }

    if (availabilityIntervalsFile.is_open()) {
        string _, placeName, startDate, endDate;
        availabilityIntervalsFile >> _ >> _ >> _; //first line not needed
        while (availabilityIntervalsFile >> placeName >> startDate >> endDate) {
            availabilityIntervalsToPlaces(places, placeName, startDate, endDate); //call to fill the places
        }
    }
    //fill sessions vector to call interval scheduling. each session is a few hours long
    for (uint i = 0; i < places.size(); i++) {
        vector<Session> sessions;
        vector<Session> selectedSessions;
        Place currentPlace = places[placeNames[i]];
        for (uint j = 0; j < currentPlace.salons.size(); j++) {
            Salon currentSalon = currentPlace.salons[currentPlace.salonNames[j]];
            for (uint k = 0; k < currentSalon.availableHours.size(); k++) {
                Session newSession(currentSalon.salonName, currentSalon.capacity, currentSalon.availableHours[k]);
                sessions.push_back(newSession);
            }
            currentPlace.salons[currentPlace.salonNames[j]] = currentSalon;
        }
        currentPlace.maxCapacity = timeIntervalScheduling(sessions, selectedSessions);
        currentPlace.selectedSessions = selectedSessions;
        places[placeNames[i]] = currentPlace;
    }
    //print to the file
    for (uint i = 0; i < places.size(); i++) {
        Place currentPlace = places[placeNames[i]];
        bestForEachPlaceFile << currentPlace.name << " --> " << currentPlace.maxCapacity << endl;
        for (uint j = 0; j < currentPlace.selectedSessions.size(); j++) {
            Session currentSession = currentPlace.selectedSessions[j];
            bestForEachPlaceFile << currentPlace.name << "\t\t" << currentSession.salonName << "\t\t" 
            << currentSession.startTime << "\t\t" << currentSession.endTime << endl;
        }
        bestForEachPlaceFile << endl;
    }
    //fill events to call interval scheduling. each event is a few days long
    vector<Event> events;
    vector<Event> selectedEvents;
    for (uint i = 0; i < places.size(); i++) {
        Place currentPlace = places[placeNames[i]];
        for (uint j = 0; j < currentPlace.availableDays.size(); j++) {
            Event newEvent(currentPlace.name, currentPlace.maxCapacity, currentPlace.availableDays[j]);
            events.push_back(newEvent);
        }
    }
    //print to the file
    int totalRevenue = dateIntervalScheduling(events, selectedEvents);
    bestTourFile << "Total Revenue --> " << totalRevenue << endl;
    for (uint i = 0; i < selectedEvents.size(); i++) {
        Event currentEvent = selectedEvents[i];
        bestTourFile << currentEvent.placeName << "\t\t" << currentEvent.startDate << "\t\t"
        << currentEvent.endDate << endl;
    }

    vector<Asset> assets;
    vector<Asset> selectedAssets;
    double totalValue;
    if (assetsFile.is_open()) {
        string _, assetName;
        long long assetPrice;
        double assetValue;
        assetsFile >> _ >> _ >> _; //first line not needed
        while (assetsFile >> assetName >> assetPrice >> assetValue) {
            Asset newAsset(assetName, assetPrice, assetValue); //call to fill the assets
            assets.push_back(newAsset);
        }
        totalValue = knapsack(totalRevenue, assets, selectedAssets); //knapsack function for max value
    }
    else {
        cout << assetsFileName << " could not be opened\n";
        exit(-1);
    }
    //print to the file
    upgradeListFile << "Total Value --> " << totalValue << endl;
    for (uint i = 0; i < selectedAssets.size(); i++) {
        Asset currentAsset = selectedAssets[i];
        upgradeListFile << currentAsset.name << endl;
    }
    //close files
    bestForEachPlaceFile.close();
    bestTourFile.close();
    upgradeListFile.close();
    return 0;
}