#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#include <iostream>
#else
#include <iostream>
#include <cstdlib>
#endif
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <thread>
#include "httplib.h"
#include "Models.h"
#include "DataStructures.h"

using namespace std;

// Global Data Structures (Demonstrating various DSA types)
StudentDirectory directory; // Hash Table
SkillIndex skills;         // BST
LeaderboardHeap leaderboard; // Max Heap
EventHistory historyStack;   // Stack
StudentNetwork networkGraph; // Graph

// ---------------------------------------------------------
// DATA PERSISTENCE
// ---------------------------------------------------------
void loadDatabase() {
    ifstream sfile("students.txt");
    string line;
    int count = 0;
    cout << "Reading students.txt..." << endl;
    while (getline(sfile, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        vector<string> f;
        string field;
        while(getline(ss, field, '|')) f.push_back(field);

        try {
            Student* s = nullptr;
            if (f.size() == 13) {
                // Legacy Format: ID|Name|Email|Pass|Skill|Yr|Sem|CG|SG|Exp|Club|Status|Pts
                s = new Student(f[0], f[1], f[2], f[3], f[4], 
                               stoi(f[5]), stoi(f[6]), stof(f[7]), stof(f[8]), stoi(f[9]), 
                               f[10], f[11]);
                s->setMeritPoints(stoi(f[12]));
            } else if (f.size() >= 10) {
                // New Format: ID|Name|Email|Skill|Yr|SG|Exp|Club|Status|Pts
                s = new Student(f[0], f[1], f[2], f[0], f[3], 
                               stoi(f[4]), 1, stof(f[5]), stof(f[5]), stoi(f[6]), 
                               f[7], f[8]);
                s->setMeritPoints(stoi(f[9]));
            }

            if (s) {
                directory.insert(s);
                skills.indexStudent(s);
                leaderboard.insertOrUpdate(s);
                count++;
            }
        } catch (const exception& e) {
            cerr << "Warning: Skipping malformed line [" << line << "] - Error: " << e.what() << endl;
        }
    }
    sfile.close();
    cout << "✅ Loaded " << count << " students safely." << endl;

    ifstream efile("events.txt");
    while (getline(efile, line)) {
        if (!line.empty()) historyStack.logEvent(line);
    }
    efile.close();
}

void saveStudent(Student* s) {
    ofstream sfile("students.txt", ios::app);
    sfile << s->getId() << "|" << s->getName() << "|" << s->getEmail() << "|" << s->getSkill() 
          << "|" << s->getYear() << "|" << s->getSgpa() << "|" << s->getExperience() 
          << "|" << s->getAppliedClub() << "|" << s->getMembershipStatus() << "|" << s->getMeritPoints() << "\n";
}

void rewriteAllStudents() {
    auto all = directory.getAllStudents();
    ofstream sfile("students.txt");
    for(auto s : all) {
        sfile << s->getId() << "|" << s->getName() << "|" << s->getEmail() << "|" << s->getSkill() 
              << "|" << s->getYear() << "|" << s->getSgpa() << "|" << s->getExperience() 
              << "|" << s->getAppliedClub() << "|" << s->getMembershipStatus() << "|" << s->getMeritPoints() << "\n";
    }
}

// ---------------------------------------------------------
// WEB SERVER (API ENDPOINTS)
// ---------------------------------------------------------
void startWebServer() {
    httplib::Server svr;

    // Helper: Static File Serving
    auto serve = [](const string& file, const string& type) {
        return [file, type](const httplib::Request&, httplib::Response& res) {
            ifstream in(file);
            if (!in) { res.status = 404; return; }
            stringstream buffer; buffer << in.rdbuf();
            res.set_content(buffer.str(), type);
        };
    };

    svr.Get("/", serve("index.html", "text/html"));
    svr.Get("/style.css", serve("style.css", "text/css"));
    svr.Get("/script.js", serve("script.js", "application/javascript"));

    // Student Login/Profile API
    svr.Get(R"(/api/student/(\w+))", [](const httplib::Request& req, httplib::Response& res) {
        Student* s = directory.getStudent(req.matches[1]);
        if (s) {
            stringstream json;
            json << "{\"status\":\"success\", \"id\":\"" << s->getId() << "\", \"name\":\"" << s->getName() << "\", \"points\":" << s->getMeritPoints() << "}";
            res.set_content(json.str(), "application/json");
        } else res.status = 404;
    });

    // Leaderboard API
    svr.Get("/api/students", [](const httplib::Request&, httplib::Response& res) {
        auto all = directory.getAllStudents();
        stringstream json; json << "[";
        for (size_t i = 0; i < all.size(); ++i) {
            json << "{\"id\":\"" << all[i]->getId() << "\", \"name\":\"" << all[i]->getName() << "\", \"points\":" << all[i]->getMeritPoints() << "}";
            if (i < all.size() - 1) json << ",";
        }
        json << "]";
        res.set_content(json.str(), "application/json");
    });

    // Admin Login API
    svr.Post("/api/admin/login", [](const httplib::Request& req, httplib::Response& res) {
        if (req.body.find("\"password\":\"admin123\"") != string::npos) {
            res.set_content("{\"status\":\"success\"}", "application/json");
        } else {
            res.status = 401;
            res.set_content("{\"status\":\"error\", \"message\":\"Invalid password\"}", "application/json");
        }
    });

    // Admin Event Broadcast API
    svr.Post("/api/admin/event", [](const httplib::Request& req, httplib::Response& res) {
        auto extract = [](string j, string k) { 
            size_t p = j.find("\"" + k + "\":");
            if (p == string::npos) return string("");
            size_t s_start = j.find("\"", p + k.length() + 2);
            if (s_start == string::npos) return string("");
            s_start += 1;
            size_t s_end = j.find("\"", s_start);
            if (s_end == string::npos) return string("");
            return j.substr(s_start, s_end - s_start);
        };
        string desc = extract(req.body, "description");
        if (!desc.empty()) {
            historyStack.logEvent(desc);
            ofstream efile("events.txt", ios::app);
            efile << desc << "\n";
            res.set_content("{\"status\":\"success\"}", "application/json");
        } else res.status = 400;
    });

    // Admin Student Registration API
    svr.Post("/api/admin/student", [](const httplib::Request& req, httplib::Response& res) {
        auto extract = [](string j, string k) { 
            size_t p = j.find("\"" + k + "\":");
            if (p == string::npos) return string("");
            size_t s_start = j.find("\"", p + k.length() + 2);
            if (s_start == string::npos) return string("");
            s_start += 1;
            size_t s_end = j.find("\"", s_start);
            if (s_end == string::npos) return string("");
            return j.substr(s_start, s_end - s_start);
        };
        
        string id = extract(req.body, "id");
        string name = extract(req.body, "name");
        string email = extract(req.body, "email");
        string skill = extract(req.body, "skill");

        if (!id.empty() && !name.empty()) {
            // Create student with default values
            Student* s = new Student(id, name, email, id, skill); // ID as password by default
            directory.insert(s);
            skills.indexStudent(s);
            leaderboard.insertOrUpdate(s);
            saveStudent(s);
            res.set_content("{\"status\":\"success\"}", "application/json");
        } else res.status = 400;
    });

    // Club Application API
    svr.Post("/api/apply", [](const httplib::Request& req, httplib::Response& res) {
        auto extract = [](string j, string k) { 
            size_t p = j.find("\"" + k + "\":");
            if (p == string::npos) return string("");
            size_t s_start = j.find("\"", p + k.length() + 2);
            if (s_start == string::npos) return string("");
            s_start += 1;
            size_t s_end = j.find("\"", s_start);
            if (s_end == string::npos) return string("");
            return j.substr(s_start, s_end - s_start);
        };

        string id = extract(req.body, "id");
        string name = extract(req.body, "name");
        string email = extract(req.body, "email");
        string skill = extract(req.body, "skill");
        string yearStr = extract(req.body, "year");
        string sgpaStr = extract(req.body, "sgpa");
        string expStr = extract(req.body, "exp");
        string club = extract(req.body, "club");

        Student* s = directory.getStudent(id);
        if (s) {
            // Update Student Details from Form
            s->setName(name);
            s->setEmail(email);
            s->setSkill(skill);
            try {
                int yr = stoi(yearStr);
                float sg = stof(sgpaStr);
                int ex = stoi(expStr);
                
                s->setYear(yr);
                s->setSgpa(sg);
                s->setExperience(ex);
                
                // Merit Calculation Logic
                // Base: SGPA * 10 + Experience * 5
                // Bonus for First Year (Bypass criteria)
                int points = (int)(sg * 10) + (ex * 5);
                if (yr == 1) points += 20; // 1st Year boost
                
                s->setMeritPoints(points);
            } catch (...) {}

            s->setAppliedClub(club);
            s->setMembershipStatus("pending");
            
            leaderboard.insertOrUpdate(s); // Update leaderboard with new points
            rewriteAllStudents();
            res.set_content("{\"status\":\"success\"}", "application/json");
        } else res.status = 404;
    });

    const char* port_env = std::getenv("PORT");
    int port = port_env ? std::stoi(port_env) : 8080;

    std::cout << "🚀 Synapse Backend live on http://0.0.0.0:" << port << std::endl;
    svr.listen("0.0.0.0", port);
}

// ---------------------------------------------------------
// CLI MENU (For Professor Explanation)
// ---------------------------------------------------------
int main() {
    try {
        loadDatabase();
        thread webThread(startWebServer);
        webThread.detach();

        int choice;
        const char* headless = std::getenv("HEADLESS");
        if (headless && std::string(headless) == "true") {
            cout << "Running in HEADLESS mode (Web Server only)." << endl;
            while(true) std::this_thread::sleep_for(std::chrono::hours(24));
        }

        do {
            cout << "\n--- SYNAPSE MANAGEMENT SYSTEM ---\n";
            cout << "1. View Students (Hash Table)\n2. Search Skill (BST)\n3. Leaderboard (Heap)\n4. Undo Logic (Stack)\n0. Exit\nChoice: ";
            if (!(cin >> choice)) break;

            switch (choice) {
                case 1: 
                    for(auto s : directory.getAllStudents()) s->displayProfile(); 
                    break;
                case 2: {
                    string sk; cout << "Skill: "; cin >> sk;
                    skills.findStudentsBySkill(sk);
                    break;
                }
                case 3: leaderboard.displayLeaderboard(5); break;
                case 4: historyStack.undoLastEvent(); break;
            }
        } while (choice != 0);
    } catch (const exception& e) {
        cerr << "FATAL ERROR: " << e.what() << endl;
        return 1;
    }

    return 0;
}