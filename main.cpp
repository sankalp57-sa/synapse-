#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector> // Added for --get-events to reverse order
#include "Models.h"
#include "DataStructures.h"

using namespace std;

// Initialize Core Data Structures
StudentDirectory directory;
SkillIndex skills;
LeaderboardHeap leaderboard;
EventHistory historyStack;

// Helper: Trim whitespace (if needed) and basic JSON escape
string escapeJSON(const string& s) {
    string result;
    for (char c : s) {
        if (c == '"') result += "\\\"";
        else result += c;
    }
    return result;
}

// ---------------------------------------------------------
// DATA PERSISTENCE: Load from Files
// ---------------------------------------------------------
void loadDatabase() {
    // Load Students
    ifstream sfile("students.txt");
    string line, id, name, email, skill;
    int points;
    
    while (getline(sfile, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        getline(ss, id, '|');
        getline(ss, name, '|');
        getline(ss, email, '|');
        getline(ss, skill, '|');
        ss >> points;
        
        Student* s = new Student(id, name, email, skill);
        s->addMeritPoints(points);
        
        directory.insert(s);
        skills.indexStudent(s);
        leaderboard.insertOrUpdate(s);
    }
    sfile.close();

    // Load Events
    ifstream efile("events.txt");
    while (getline(efile, line)) {
        if (!line.empty()) historyStack.logEvent(line);
    }
    efile.close();
}

void saveStudent(string id, string name, string email, string skill, int points) {
    ofstream sfile("students.txt", ios::app);
    sfile << id << "|" << name << "|" << email << "|" << skill << "|" << points << "\n";
    sfile.close();
}

void saveEvent(string desc) {
    ofstream efile("events.txt", ios::app);
    efile << desc << "\n";
    efile.close();
}

// ---------------------------------------------------------
// MAIN CLI API (Output MUST be valid JSON)
// ---------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "{\"error\": \"No arguments provided\"}" << endl;
        return 1;
    }

    // Always load data first to populate Hash Table, BST, etc.
    // Suppress cout from internal logic by redirecting rdbuf temporarily
    streambuf* orig_cout = cout.rdbuf();
    stringstream dummy;
    cout.rdbuf(dummy.rdbuf());
    
    loadDatabase();
    
    // Restore cout for JSON output
    cout.rdbuf(orig_cout);

    string command = argv[1];

    if (command == "--add-student" && argc >= 6) {
        string id = argv[2];
        string name = argv[3];
        string email = argv[4];
        string skill = argv[5];
        
        // Ensure not duplicate in our Directory (Hash Table)
        if (directory.getStudent(id) != nullptr) {
            cout << "{\"status\": \"error\", \"message\": \"Student ID already exists\"}" << endl;
            return 1;
        }

        saveStudent(id, name, email, skill, 0); // initial 0 points
        cout << "{\"status\": \"success\", \"message\": \"Student added\"}" << endl;
    }
    else if (command == "--get-students") {
        // Read directly from file for JSON simplicity, or iterate Hash Table
        // For simplicity, we just read the file and format as JSON array
        ifstream sfile("students.txt");
        string line;
        cout << "[";
        bool first = true;
        while (getline(sfile, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string id, name, email, skill, pStr;
            getline(ss, id, '|');
            getline(ss, name, '|');
            getline(ss, email, '|');
            getline(ss, skill, '|');
            getline(ss, pStr);
            
            if (!first) cout << ",";
            cout << "{\"id\": \"" << escapeJSON(id) << "\", \"name\": \"" << escapeJSON(name) 
                 << "\", \"skill\": \"" << escapeJSON(skill) << "\", \"points\": " << pStr << "}";
            first = false;
        }
        cout << "]" << endl;
    }
    else if (command == "--add-event" && argc >= 3) {
        string desc = argv[2];
        saveEvent(desc);
        cout << "{\"status\": \"success\", \"message\": \"Event added\"}" << endl;
    }
    else if (command == "--get-events") {
        ifstream efile("events.txt");
        string line;
        cout << "[";
        bool first = true;
        
        // Reverse order so newest is first (simulating Stack Pop order)
        vector<string> events;
        while (getline(efile, line)) {
            if (!line.empty()) events.push_back(line);
        }
        for (int i = events.size() - 1; i >= 0; i--) {
            if (!first) cout << ",";
            cout << "{\"desc\": \"" << escapeJSON(events[i]) << "\"}";
            first = false;
        }
        cout << "]" << endl;
    }
    else {
        cout << "{\"error\": \"Invalid command format\"}" << endl;
        return 1;
    }

    return 0;
}