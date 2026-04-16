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
        string id, name, email, pass, skill, club = "none", status = "none";
        int yr = 1, sem = 1, exp = 0, pts = 0;
        float cg = 0.0f, sg = 0.0f;

        getline(ss, id, '|');
        getline(ss, name, '|');
        getline(ss, email, '|');
        
        // This parser needs to be robust for the NEW format
        // ID|Name|Email|Password|Skill|Year|Sem|CGPA|SGPA|Exp|Club|Status|Points
        string field;
        vector<string> fields;
        while(getline(ss, field, '|')) fields.push_back(field);

        if (fields.size() >= 10) {
            // New Format
            pass = fields[0];
            skill = fields[1];
            yr = stoi(fields[2]);
            sem = stoi(fields[3]);
            cg = stof(fields[4]);
            sg = stof(fields[5]);
            exp = stoi(fields[6]);
            club = fields[7];
            status = fields[8];
            pts = stoi(fields[9]);
        } else {
            // Old Format (Migration)
            skill = fields[0];
            yr = stoi(fields[1]);
            cg = stof(fields[2]);
            exp = stoi(fields[3]);
            pass = id; // Default password for migration
            if (fields.size() > 4) {
                club = fields[4];
                status = fields[5];
                if (fields.size() > 6) pts = stoi(fields[6]);
            }
        }
        
        Student* s = new Student(id, name, email, pass, skill, yr, sem, cg, sg, exp, club, status);
        s->setMeritPoints(pts);
        
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
    sfile << s->getId() << "|" << s->getName() << "|" << s->getEmail() << "|" << s->getPassword() << "|" << s->getSkill() 
          << "|" << s->getYear() << "|" << s->getSemester() << "|" << s->getCgpa() << "|" << s->getSgpa() << "|" << s->getExperience() 
          << "|" << s->getAppliedClub() << "|" << s->getMembershipStatus() << "|" << s->getMeritPoints() << "\n";
    sfile.close();
}

void rewriteAllStudents() {
    vector<Student*> all = directory.getAllStudents();
    ofstream sfile("students.txt");
    for(auto s : all) {
        sfile << s->getId() << "|" << s->getName() << "|" << s->getEmail() << "|" << s->getPassword() << "|" << s->getSkill() 
              << "|" << s->getYear() << "|" << s->getSemester() << "|" << s->getCgpa() << "|" << s->getSgpa() << "|" << s->getExperience() 
              << "|" << s->getAppliedClub() << "|" << s->getMembershipStatus() << "|" << s->getMeritPoints() << "\n";
    }
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
            json << "{"
                 << "\"status\":\"success\","
                 << "\"id\":\"" << s->getId() << "\","
                 << "\"name\":\"" << s->getName() << "\","
                 << "\"skill\":\"" << s->getSkill() << "\","
                 << "\"year\":" << s->getYear() << ","
                 << "\"sem\":" << s->getSemester() << ","
                 << "\"cgpa\":" << s->getCgpa() << ","
                 << "\"experience\":" << s->getExperience() << ","
                 << "\"appliedClub\":\"" << s->getAppliedClub() << "\","
                 << "\"membershipStatus\":\"" << s->getMembershipStatus() << "\","
                 << "\"points\":" << s->getMeritPoints()
                 << "}";
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

        Student* s = new Student(id, name, email, id, skill, 1, 1, 0.0, 0.0, 0); // Default values (Year 1, Sem 1)
        saveStudent(s);
        directory.insert(s);
        skills.indexStudent(s);
        leaderboard.insertOrUpdate(s);
        res.status = 200;
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    svr.Post("/api/apply", [&extractJsonString, &extractJsonNumber](const httplib::Request& req, httplib::Response& res) {
        string b = req.body;
        string id = extractJsonString(b, "id");
        string name = extractJsonString(b, "name");
        string email = extractJsonString(b, "email");
        string skill = extractJsonString(b, "skill");
        string club = extractJsonString(b, "club");
        int yr = (int)extractJsonNumber(b, "year");
        int sem = (int)extractJsonNumber(b, "semester");
        float cg = (float)extractJsonNumber(b, "cgpa");
        float sg = (float)extractJsonNumber(b, "sgpa");
        int ex = (int)extractJsonNumber(b, "experience");

        Student* s = directory.getStudent(id);
        if (!s) {
            // 1st Time Login User: Auto-Register
            s = new Student(id, name, email, id, skill, yr, sem, cg, sg, ex);
            directory.insert(s);
            skills.indexStudent(s);
            leaderboard.insertOrUpdate(s);
            saveStudent(s); // Save to file
        }

        // 1. Merit Decision logic: SGPA >= 9.0 -> Immediate Selection
        string status = (sg >= 9.0f) ? "Selected" : "Pending";
        bool autoApproved = (status == "Selected");

        s->setAppliedClub(club);
        s->setMembershipStatus(status);
        
        rewriteAllStudents();
        
        // 4. Activity Logs
        historyStack.logEvent("New Application: " + name + " for " + club + (autoApproved ? " (Elite Selection)" : ""));
        saveEvent("App: " + name + " for " + club + (autoApproved ? " (Elite Selection)" : ""));

        // 5. Response
        stringstream resJson;
        resJson << "{"
                << "\"status\":\"success\","
                << "\"studentId\":\"" << id << "\","
                << "\"autoApproved\":" << (autoApproved ? "true" : "false") << ","
                << "\"meritPoints\":" << s->getMeritPoints() << ","
                << "\"message\":\"" << (autoApproved ? "Elite Selection: Auto-Approved!" : "Application submitted! Wait for President review.") << "\""
                << "}";
        res.status = 200;
        res.set_content(resJson.str(), "application/json");
    });

    // --- PRESIDENT API ---

    svr.Post("/api/president/login", [&extractJsonString](const httplib::Request& req, httplib::Response& res) {
        string club = extractJsonString(req.body, "club");
        string pass = extractJsonString(req.body, "password");
        if (club == pass && !club.empty()) {
            res.set_content("{\"status\":\"success\"}", "application/json");
        } else {
            res.status = 401;
            res.set_content("{\"status\":\"error\", \"message\":\"Invalid credentials\"}", "application/json");
        }
    });

    svr.Get("/api/president/requests", [](const httplib::Request& req, httplib::Response& res) {
        string club = req.get_param_value("club");
        vector<Student*> all = directory.getAllStudents();
        stringstream json;
        json << "[";
        bool first = true;
        for (auto s : all) {
            if (s->getAppliedClub() == club && s->getMembershipStatus() == "pending") {
                if (!first) json << ",";
                json << "{\"id\":\"" << s->getId() << "\", \"name\":\"" << s->getName() 
                     << "\", \"skill\":\"" << s->getSkill() << "\", \"cgpa\":" << s->getCgpa() << "}";
                first = false;
            }
        }
        json << "]";
        res.set_content(json.str(), "application/json");
    });

    svr.Get("/api/president/members", [](const httplib::Request& req, httplib::Response& res) {
        string club = req.get_param_value("club");
        vector<Student*> all = directory.getAllStudents();
        stringstream json;
        json << "[";
        bool first = true;
        for (auto s : all) {
            if (s->getAppliedClub() == club && s->getMembershipStatus() == "accepted") {
                if (!first) json << ",";
                json << "{\"id\":\"" << s->getId() << "\", \"name\":\"" << s->getName() 
                     << "\", \"skill\":\"" << s->getSkill() << "\"}";
                first = false;
            }
        }
        json << "]";
        res.set_content(json.str(), "application/json");
    });

    svr.Post("/api/president/action", [&extractJsonString](const httplib::Request& req, httplib::Response& res) {
        string id = extractJsonString(req.body, "id");
        string action = extractJsonString(req.body, "action"); // "accepted" or "rejected"
        Student* s = directory.getStudent(id);
        if (s) {
            s->setMembershipStatus(action);
            
            // Rewrite database
            vector<Student*> allS = directory.getAllStudents();
            ofstream sfile("students.txt");
            for(auto st : allS) {
                sfile << st->getId() << "|" << st->getName() << "|" << st->getEmail() << "|" << st->getSkill() 
                      << "|" << st->getYear() << "|" << st->getCgpa() << "|" << st->getExperience() 
                      << "|" << st->getAppliedClub() << "|" << st->getMembershipStatus() << "|" << st->getMeritPoints() << "\n";
            }
            sfile.close();
            
            res.set_content("{\"status\":\"success\"}", "application/json");
        } else {
            res.status = 404;
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
    });

    svr.Post("/api/president/remove", [&extractJsonString](const httplib::Request& req, httplib::Response& res) {
        string id = extractJsonString(req.body, "id");
        Student* s = directory.getStudent(id);
        if (s) {
            s->setMembershipStatus("none");
            s->setAppliedClub("none");
            
            // Rewrite database
            vector<Student*> allS = directory.getAllStudents();
            ofstream sfile("students.txt");
            for(auto st : allS) {
                sfile << st->getId() << "|" << st->getName() << "|" << st->getEmail() << "|" << st->getSkill() 
                      << "|" << st->getYear() << "|" << st->getCgpa() << "|" << st->getExperience() 
                      << "|" << st->getAppliedClub() << "|" << st->getMembershipStatus() << "|" << st->getMeritPoints() << "\n";
            }
            sfile.close();
            
            res.set_content("{\"status\":\"success\"}", "application/json");
        } else {
            res.status = 404;
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
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
                        Student* s = new Student(id, name, email, id, skill, 1, 1, 0.0, 0.0, 0); // basic fallback
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