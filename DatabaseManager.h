#ifndef SYNAPSE_DATABASE_MANAGER_H
#define SYNAPSE_DATABASE_MANAGER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "Models.h"
#include "DataStructures.h"

using namespace std;

// Extern declarations for global data structures (defined in main.cpp)
extern StudentDirectory directory;
extern SkillIndex skills;
extern LeaderboardHeap leaderboard;
extern EventHistory historyStack;
extern StudentNetwork networkGraph;

// ---------------------------------------------------------
// DATA PERSISTENCE (JSON)
// ---------------------------------------------------------

// Safe string conversion helpers
inline int safeStoi(string s, int def = 0) {
    if (s.empty()) return def;
    try { return stoi(s); } catch (...) { return def; }
}

inline float safeStof(string s, float def = 0.0f) {
    if (s.empty()) return def;
    try { return stof(s); } catch (...) { return def; }
}

inline string extractJson(string j, string k) {
    size_t keyPos = j.find("\"" + k + "\"");
    if (keyPos == string::npos) return "";
    
    size_t colonPos = j.find(":", keyPos);
    if (colonPos == string::npos) return "";
    
    size_t valStart = colonPos + 1;
    // Skip whitespace after colon
    valStart = j.find_first_not_of(" \t\n\r", valStart);
    if (valStart == string::npos) return "";

    if (j[valStart] == '\"') {
        // String value
        size_t s_start = valStart + 1;
        size_t s_end = j.find("\"", s_start);
        if (s_end == string::npos) return "";
        return j.substr(s_start, s_end - s_start);
    } else {
        // Number or boolean value
        size_t n_end = j.find_first_of(",}\n\r ", valStart);
        if (n_end == string::npos) n_end = j.length();
        return j.substr(valStart, n_end - valStart);
    }
}

struct Club {
    string name;
    string type;
    string desc;
};

extern vector<Club> globalClubs;

inline void saveClubs() {
    ofstream f("clubs.json");
    f << "[\n";
    for(size_t i = 0; i < globalClubs.size(); i++) {
        f << "  {\"name\":\"" << globalClubs[i].name << "\", \"type\":\"" << globalClubs[i].type << "\", \"desc\":\"" << globalClubs[i].desc << "\"}" << (i < globalClubs.size()-1 ? "," : "") << "\n";
    }
    f << "]\n";
    f.close();
}

inline void loadClubs() {
    globalClubs.clear();
    ifstream f("clubs.json");
    if(!f) {
        globalClubs.push_back({"Coding", "tech", "Algorithms and development hub."});
        globalClubs.push_back({"Robotics", "hardware", "IoT and mechanical design focus."});
        globalClubs.push_back({"Debate", "literature", "Model UN and debating."});
        globalClubs.push_back({"Design", "design", "UI/UX and creative design workshop."});
        saveClubs();
        return;
    }
    string content((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    f.close();
    size_t pos = 0;
    while((pos = content.find("{", pos)) != string::npos) {
        size_t endPos = content.find("}", pos);
        if(endPos == string::npos) break;
        string obj = content.substr(pos, endPos - pos + 1);
        string name = extractJson(obj, "name");
        string t = extractJson(obj, "type");
        string d = extractJson(obj, "desc");
        if(!name.empty()) {
            globalClubs.push_back({name, t, d});
        }
        pos = endPos;
    }
}

inline void saveDatabase() {
    auto all = directory.getAllStudents();
    ofstream dbFile("database.json");
    dbFile << "{\n  \"students\": [\n";
    for (size_t i = 0; i < all.size(); ++i) {
        Student* s = all[i];
        dbFile << "    {\n"
               << "      \"id\": \"" << s->getId() << "\",\n"
               << "      \"name\": \"" << s->getName() << "\",\n"
               << "      \"email\": \"" << s->getEmail() << "\",\n"
               << "      \"skill\": \"" << s->getSkill() << "\",\n"
               << "      \"year\": " << s->getYear() << ",\n"
               << "      \"sgpa\": " << s->getSgpa() << ",\n"
               << "      \"exp\": " << s->getExperience() << ",\n"
               << "      \"club\": \"" << s->getAppliedClub() << "\",\n"
               << "      \"status\": \"" << s->getMembershipStatus() << "\",\n"
               << "      \"message\": \"" << s->getApplicationMessage() << "\",\n"
               << "      \"domain\": \"" << s->getDomain() << "\",\n"
               << "      \"points\": " << s->getMeritPoints() << "\n"
               << "    }" << (i < all.size() - 1 ? "," : "") << "\n";
    }
    dbFile << "  ]\n}";
    dbFile.close();
}

inline void loadDatabase() {
    ifstream dbFile("database.json");
    if (!dbFile) {
        cout << "⚠️ No JSON database found. Attempting migration from students.txt..." << endl;
        ifstream oldFile("students.txt");
        if (oldFile) {
            string line;
            while (getline(oldFile, line)) {
                if (line.empty()) continue;
                stringstream ss(line); vector<string> f; string field;
                while(getline(ss, field, '|')) f.push_back(field);
                
                try {
                    Student* s = nullptr;
                    if (f.size() == 13) {
                        // ID|Name|Email|Pass|Skill|Yr|Sem|CG|SG|Exp|Club|Status|Pts
                        s = new Student(f[0], f[1], f[2], f[3], f[4], safeStoi(f[5]), safeStoi(f[6]), safeStof(f[7]), safeStof(f[8]), safeStoi(f[9]), f[10], f[11]);
                        s->setMeritPoints(safeStoi(f[12]));
                    } else if (f.size() >= 10) {
                        // ID|Name|Email|Pass|Skill|Yr|SG|Exp|Club|Status|Pts
                        s = new Student(f[0], f[1], f[2], f[0], f[3], safeStoi(f[4]), 1, safeStof(f[5]), safeStof(f[5]), safeStoi(f[6]), f[7], f[8]);
                        s->setMeritPoints(safeStoi(f[9]));
                    }
                    if (s) {
                        directory.insert(s); skills.indexStudent(s); leaderboard.insertOrUpdate(s);
                    }
                } catch (...) { continue; }
            }
            oldFile.close(); saveDatabase();
        }
        return;
    }

    string content((istreambuf_iterator<char>(dbFile)), istreambuf_iterator<char>());
    dbFile.close();

    size_t pos = 0;
    while ((pos = content.find("{", pos + 1)) != string::npos) {
        try {
            size_t objEnd = content.find("}", pos);
            if (objEnd == string::npos) break;
            string obj = content.substr(pos, objEnd - pos + 1);
            
            string id = extractJson(obj, "id");
            if (id.empty()) { pos = objEnd; continue; }
            
            string name = extractJson(obj, "name");
            string email = extractJson(obj, "email");
            string skill = extractJson(obj, "skill");
            
            int yr = safeStoi(extractJson(obj, "year"), 1);
            float sg = safeStof(extractJson(obj, "sgpa"), 0.0f);
            int ex = safeStoi(extractJson(obj, "exp"), 0);
            int pt = safeStoi(extractJson(obj, "points"), 0);
            
            string club = extractJson(obj, "club");
            string status = extractJson(obj, "status");
            string msg = extractJson(obj, "message"); // New
            string domain = extractJson(obj, "domain");

            Student* s = new Student(id, name, email, id, skill, yr, 1, sg, sg, ex, club, status, msg, domain);
            s->setMeritPoints(pt);
            directory.insert(s); skills.indexStudent(s); leaderboard.insertOrUpdate(s);
            pos = objEnd;
        } catch (...) { pos++; }
    }
    cout << "✅ Loaded database from JSON." << endl;
}

#endif
