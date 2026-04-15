#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <thread>
#include <cstdlib>
#include "httplib.h"
#include "Models.h"
#include "DataStructures.h"

using namespace std;

// ... (rest of global vars) ...

// Initialize Core Data Structures
StudentDirectory directory;
SkillIndex skills;
LeaderboardHeap leaderboard;
EventHistory historyStack;
StudentNetwork networkGraph;

map<string, float> clubCriteria = {
    {"Coding", 7.5},
    {"Design", 7.0},
    {"Robotics", 8.0}
};

// ---------------------------------------------------------
// DATA PERSISTENCE: Load from Files
// ---------------------------------------------------------
void loadDatabase() {
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
        
        string yearStr, cgpaStr, expStr;
        int year = 1, experience = 0, points = 0;
        float cgpa = 0.0f;

        if (getline(ss, yearStr, '|')) year = stoi(yearStr);
        if (getline(ss, cgpaStr, '|')) cgpa = stof(cgpaStr);
        if (getline(ss, expStr, '|')) experience = stoi(expStr);
        ss >> points;
        
        Student* s = new Student(id, name, email, skill, year, cgpa, experience);
        s->setMeritPoints(points);
        
        directory.insert(s);
        skills.indexStudent(s);
        leaderboard.insertOrUpdate(s);
    }
    sfile.close();

    ifstream efile("events.txt");
    while (getline(efile, line)) {
        if (!line.empty()) historyStack.logEvent(line);
    }
    efile.close();
}

void saveStudent(Student* s) {
    ofstream sfile("students.txt", ios::app);
    sfile << s->getId() << "|" << s->getName() << "|" << s->getEmail() << "|" << s->getSkill() 
          << "|" << s->getYear() << "|" << s->getCgpa() << "|" << s->getExperience() 
          << "|" << s->getMeritPoints() << "\n";
    sfile.close();
}

void saveEvent(string desc) {
    ofstream efile("events.txt", ios::app);
    efile << desc << "\n";
    efile.close();
}

void displayMenu() {
    cout << "\n=============================================\n";
    cout << "      SYNAPSE - MICRO PROJECT (OOP & DSA)\n";
    cout << "=============================================\n";
    cout << "1. Add a New Student  (Hash Table)\n";
    cout << "2. Search by Skill    (Binary Search Tree)\n";
    cout << "3. View Leaderboard   (Max Heap)\n";
    cout << "4. Log an Event       (Stack - Push)\n";
    cout << "5. Undo Last Event    (Stack - Pop)\n";
    cout << "6. Add Connection     (Graph - Edge)\n";
    cout << "7. View Connections   (Graph Traversal)\n";
    cout << "0. Exit\n";
    cout << "=============================================\n";
    cout << "Enter your choice: ";
}

void startWebServer() {
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        ifstream in("index.html");
        if (!in) {
            res.status = 404;
            res.set_content("index.html not found", "text/plain");
            return;
        }
        stringstream buffer;
        buffer << in.rdbuf();
        res.set_content(buffer.str(), "text/html");
    });

    svr.Get("/admin.html", [](const httplib::Request& req, httplib::Response& res) {
        ifstream in("admin.html");
        if (!in) {
            res.status = 404;
            res.set_content("admin.html not found", "text/plain");
            return;
        }
        stringstream buffer;
        buffer << in.rdbuf();
        res.set_content(buffer.str(), "text/html");
    });

    svr.Get("/style.css", [](const httplib::Request& req, httplib::Response& res) {
        ifstream in("style.css");
        if (in) {
            stringstream buffer; buffer << in.rdbuf();
            res.set_content(buffer.str(), "text/css");
        }
    });

    svr.Get("/script.js", [](const httplib::Request& req, httplib::Response& res) {
        ifstream in("script.js");
        if (in) {
            stringstream buffer; buffer << in.rdbuf();
            res.set_content(buffer.str(), "application/javascript");
        }
    });

    svr.Get("/api/students", [](const httplib::Request& req, httplib::Response& res) {
        vector<Student*> all = directory.getAllStudents();
        stringstream json;
        json << "[";
        for (size_t i = 0; i < all.size(); ++i) {
            json << "{\"id\":\"" << all[i]->getId() 
                 << "\", \"name\":\"" << all[i]->getName() 
                 << "\", \"skill\":\"" << all[i]->getSkill() 
                 << "\", \"points\":" << all[i]->getMeritPoints() << "}";
            if (i < all.size() - 1) json << ",";
        }
        json << "]";
        res.set_content(json.str(), "application/json");
    });

    svr.Get("/api/events", [](const httplib::Request& req, httplib::Response& res) {
        vector<string> events = historyStack.getAllEvents();
        stringstream json;
        json << "[";
        for (size_t i = 0; i < events.size(); ++i) {
            string escaped = events[i];
            size_t pos = 0;
            while ((pos = escaped.find("\"", pos)) != string::npos) {
                escaped.replace(pos, 1, "\\\"");
                pos += 2;
            }
            json << "{\"desc\":\"" << escaped << "\"}";
            if (i < events.size() - 1) json << ",";
        }
        json << "]";
        res.set_content(json.str(), "application/json");
    });

    svr.Get(R"(/api/student/(\w+))", [](const httplib::Request& req, httplib::Response& res) {
        string id = req.matches[1];
        Student* s = directory.getStudent(id);
        if (s) {
            stringstream json;
            json << "{\"status\":\"success\", \"id\":\"" << s->getId() 
                 << "\", \"name\":\"" << s->getName() 
                 << "\", \"points\":" << s->getMeritPoints() << "}";
            res.set_content(json.str(), "application/json");
        } else {
            res.status = 404;
            res.set_content("{\"status\":\"error\", \"message\":\"Student not found\"}", "application/json");
        }
    });

    // Helper functions for parsing JSON
    auto extractJsonString = [](string json, string key) {
        size_t pos = json.find("\"" + key + "\":");
        if (pos == string::npos) return string("");
        size_t start = json.find("\"", pos + key.length() + 2) + 1;
        size_t end = json.find("\"", start);
        return json.substr(start, end - start);
    };
    
    auto extractJsonNumber = [](string json, string key) {
        size_t pos = json.find("\"" + key + "\":");
        if (pos == string::npos) return 0.0;
        size_t start = pos + key.length() + 3;
        size_t end = json.find_first_of(",}", start);
        return atof(json.substr(start, end - start).c_str());
    };

    svr.Post("/api/admin/event", [&extractJsonString](const httplib::Request& req, httplib::Response& res) {
        string desc = extractJsonString(req.body, "description");
        if (desc.empty()) { res.status = 400; return; }
        saveEvent(desc);
        historyStack.logEvent(desc);
        res.status = 200;
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    svr.Post("/api/admin/student", [&extractJsonString, &extractJsonNumber](const httplib::Request& req, httplib::Response& res) {
        string id = extractJsonString(req.body, "id");
        string name = extractJsonString(req.body, "name");
        string email = extractJsonString(req.body, "email");
        string skill = extractJsonString(req.body, "skill");

        if (directory.getStudent(id) != nullptr) {
            res.status = 400;
            res.set_content("{\"status\":\"error\", \"message\":\"Student ID already exists\"}", "application/json");
            return;
        }

        Student* s = new Student(id, name, email, skill, 1, 0.0, 0); // Default values
        saveStudent(s);
        directory.insert(s);
        skills.indexStudent(s);
        leaderboard.insertOrUpdate(s);
        res.status = 200;
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    svr.Post("/api/apply", [&extractJsonString, &extractJsonNumber](const httplib::Request& req, httplib::Response& res) {
        // Very basic manual JSON extraction to avoid external dependencies like nlohmann/json
        string body = req.body;

        string name = extractJsonString(body, "name");
        string email = extractJsonString(body, "email");
        string skill = extractJsonString(body, "skill");
        string club = extractJsonString(body, "club");
        int year = (int)extractJsonNumber(body, "year");
        float cgpa = (float)extractJsonNumber(body, "cgpa");
        int exp = (int)extractJsonNumber(body, "experience");

        if (year != 1) {
            float requiredCgpa = (clubCriteria.count(club) ? clubCriteria[club] : 7.0);
            if (cgpa < requiredCgpa) {
                res.status = 400;
                res.set_content("Does not meet minimum CGPA criteria (" + to_string(requiredCgpa) + ") for " + club, "text/plain");
                return;
            }
        }

        string id = "S" + to_string(rand() % 1000 + 200); 
        Student* s = new Student(id, name, email, skill, year, cgpa, exp);
        
        directory.insert(s);
        skills.indexStudent(s);
        leaderboard.insertOrUpdate(s);
        saveStudent(s);
        historyStack.logEvent("New Application: " + name + " to " + club);

        string responseJson = "{\"status\": \"success\", \"meritPoints\": " + to_string(s->getMeritPoints()) + ", \"studentId\": \"" + id + "\"}";
        res.status = 200;
        res.set_content(responseJson, "application/json");
    });

    int port = 8080;
    char* env_port = std::getenv("PORT");
    if (env_port) port = std::stoi(env_port);

    std::cout << "🚀 Synapse Backend starting on port " << port << "..." << std::endl;
    svr.listen("0.0.0.0", port);
}

int main() {
    // Suppress logs initially during load
    streambuf* orig_cout = cout.rdbuf();
    stringstream dummy;
    cout.rdbuf(dummy.rdbuf());
    
    loadDatabase();
    
    cout.rdbuf(orig_cout);

    char* env_headless = std::getenv("HEADLESS");
    bool isHeadless = (env_headless && std::string(env_headless) == "true");

    if (isHeadless) {
        std::cout << "🌐 Running in HEADLESS mode (Web Server Only)" << std::endl;
        startWebServer(); // This is a blocking call, so it keeps the process alive
    } else {
        std::thread webThread(startWebServer);
        webThread.detach();

        int choice;
        do {
            displayMenu();
            if (!(cin >> choice)) {
                if (cin.eof()) {
                    std::this_thread::sleep_for(std::chrono::hours(24 * 365));
                    continue;
                }
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            switch (choice) {
                case 1: {
                    string id, name, email, skill;
                    cout << "Enter Student ID: "; cin >> id;
                    cout << "Enter Name: "; cin.ignore(); getline(cin, name);
                    cout << "Enter Email: "; getline(cin, email);
                    cout << "Enter Primary Skill: "; getline(cin, skill);
                    
                    if (directory.getStudent(id) != nullptr) {
                        cout << "❌ Error: Student ID already exists!\n";
                    } else {
                        Student* s = new Student(id, name, email, skill, 1, 0.0, 0); // basic fallback
                        saveStudent(s);
                        directory.insert(s);
                        skills.indexStudent(s);
                        leaderboard.insertOrUpdate(s);
                        cout << "✅ Student saved successfully!\n";
                    }
                    break;
                }
                case 2: {
                    string skill;
                    cout << "Enter Skill to search: "; cin.ignore(); getline(cin, skill);
                    skills.findStudentsBySkill(skill);
                    break;
                }
                case 3:
                    leaderboard.displayLeaderboard(5);
                    break;
                case 4: {
                    string desc;
                    cout << "Enter Event Description: "; cin.ignore(); getline(cin, desc);
                    saveEvent(desc);
                    historyStack.logEvent(desc);
                    break;
                }
                case 5:
                    historyStack.undoLastEvent();
                    break;
                case 6: {
                    string id1, id2;
                    cout << "Enter First Student ID: "; cin >> id1;
                    cout << "Enter Second Student ID: "; cin >> id2;
                    Student* s1 = directory.getStudent(id1);
                    Student* s2 = directory.getStudent(id2);
                    if (s1 && s2) {
                        networkGraph.addConnection(s1, s2);
                        cout << "✅ " << s1->getName() << " and " << s2->getName() << " are now connected!\n";
                    } else {
                        cout << "❌ Invalid Student ID(s).\n";
                    }
                    break;
                }
                case 7: {
                    string id;
                    cout << "Enter Student ID to view connections: "; cin >> id;
                    Student* s = directory.getStudent(id);
                    if (s) {
                        networkGraph.displayNetwork(s);
                    } else {
                        cout << "❌ Student not found.\n";
                    }
                    break;
                }
                case 0:
                    cout << "Exiting Synapse. Goodbye!\n";
                    break;
                default:
                    cout << "Invalid choice!\n";
            }
        } while (choice != 0);
    }

    return 0;
}