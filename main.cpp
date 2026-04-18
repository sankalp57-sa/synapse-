#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <thread>
#include "httplib.h"
#include "Models.h"
#include "DataStructures.h"
#include "DatabaseManager.h"

using namespace std;

// Global Data Structures
StudentDirectory directory; // Hash Table
SkillIndex skills;         // BST
LeaderboardHeap leaderboard; // Max Heap
EventHistory historyStack;   // Stack
StudentNetwork networkGraph; // Graph
vector<Club> globalClubs;

int main() {
    try {
        loadDatabase();
        loadClubs();
        
        int port = 8080;
        try {
            const char* port_env = std::getenv("PORT");
            if (port_env) port = std::stoi(port_env);
        } catch (...) { port = 8080; }

        std::cout << "🚀 SYNAPSE CORE: Web Portal live on http://localhost:" << port << std::endl;
        std::cout << "📡 Connection Mode: Dedicated Server (Console Logic Removed)" << std::endl;

        #ifdef _WIN32
            system("start http://localhost:8080");
        #endif

        httplib::Server svr;

        auto serve = [](const string& file, const string& type) {
            return [file, type](const httplib::Request&, httplib::Response& res) {
                ifstream in(file);
                if (!in) { res.status = 404; return; }
                stringstream buffer; buffer << in.rdbuf();
                res.set_content(buffer.str(), type);
            };
        };

        svr.Get("/", serve("index.html", "text/html"));
        svr.Get("/index.html", serve("index.html", "text/html"));
        svr.Get("/style.css", serve("style.css", "text/css"));
        svr.Get("/script.js", serve("script.js", "application/javascript"));
        svr.Get("/admin.html", serve("admin.html", "text/html"));
        svr.Get("/admin", serve("admin.html", "text/html"));
        svr.Get("/faculty.html", serve("faculty.html", "text/html"));
        svr.Get("/faculty", serve("faculty.html", "text/html"));

        // API: Student
        svr.Get(R"(/api/student/(\w+))", [](const httplib::Request& req, httplib::Response& res) {
            Student* s = directory.getStudent(req.matches[1]);
            if (s) {
                stringstream json;
                json << "{\"status\":\"success\", \"id\":\"" << s->getId() << "\", "
                     << "\"name\":\"" << s->getName() << "\", \"points\":" << s->getMeritPoints() << ", "
                     << "\"membershipStatus\":\"" << s->getMembershipStatus() << "\", "
                     << "\"appliedClub\":\"" << s->getAppliedClub() << "\", "
                     << "\"skill\":\"" << s->getSkill() << "\"}";
                res.set_content(json.str(), "application/json");
            } else res.status = 404;
        });

        // API: Leaderboard
        svr.Get("/api/students", [](const httplib::Request&, httplib::Response& res) {
            auto all = directory.getAllStudents();
            stringstream json; json << "[";
            for (size_t i = 0; i < all.size(); ++i) {
                json << "{\"id\":\"" << all[i]->getId() << "\", \"name\":\"" << all[i]->getName() << "\", \"points\":" << all[i]->getMeritPoints() << ", \"skill\":\"" << all[i]->getSkill() << "\"}";
                if (i < all.size() - 1) json << ",";
            }
            json << "]";
            res.set_content(json.str(), "application/json");
        });

        // API: Login (Admin/President)
        svr.Post("/api/admin/login", [](const httplib::Request& req, httplib::Response& res) {
            string role = extractJson(req.body, "role");
            string pass = extractJson(req.body, "password");
            string club = extractJson(req.body, "club");

            if (role == "admin" && pass == "admin_root") {
                res.set_content("{\"status\":\"success\", \"role\":\"admin\"}", "application/json");
            } else if (role == "president") {
                res.set_content("{\"status\":\"success\", \"role\":\"president\", \"club\":\"" + club + "\"}", "application/json");
            } else {
                res.status = 401;
                res.set_content("{\"status\":\"error\"}", "application/json");
            }
        });

        // API: Broadcast Event
        svr.Post("/api/admin/event", [](const httplib::Request& req, httplib::Response& res) {
            string desc = extractJson(req.body, "description");
            string club = extractJson(req.body, "club");
            if (!desc.empty()) {
                historyStack.logEvent("📢 [" + club + "] " + desc);
                saveDatabase(); 
                res.set_content("{\"status\":\"success\"}", "application/json");
            } else res.status = 400;
        });

        // API: Register Student
        svr.Post("/api/admin/student", [](const httplib::Request& req, httplib::Response& res) {
            string id = extractJson(req.body, "id");
            string name = extractJson(req.body, "name");
            string email = extractJson(req.body, "email");
            string skill = extractJson(req.body, "skill");
            if (!id.empty() && !name.empty()) {
                Student* s = new Student(id, name, email, id, skill);
                directory.insert(s);
                skills.indexStudent(s);
                leaderboard.insertOrUpdate(s);
                saveDatabase();
                res.set_content("{\"status\":\"success\"}", "application/json");
            } else res.status = 400;
        });

        // API: Events
        svr.Get("/api/events", [&](const httplib::Request&, httplib::Response& res) {
            ostringstream oss; oss << "[";
            auto events = historyStack.getAllEvents();
            for (size_t i = 0; i < events.size(); ++i) {
                string e = events[i];
                if (e.find("{") == string::npos) oss << "{\"desc\":\"" << e << "\"}";
                else oss << e;
                if (i < events.size() - 1) oss << ",";
            }
            oss << "]";
            res.set_content(oss.str(), "application/json");
        });

        // API: Global Stats
        svr.Get("/api/stats", [](const httplib::Request&, httplib::Response& res) {
            auto all = directory.getAllStudents();
            int totalPoints = 0;
            int totalSkills = 0;
            for (auto s : all) {
                totalPoints += s->getMeritPoints();
                if (!s->getSkill().empty() && s->getSkill() != "none") totalSkills++;
            }
            string json = "{\"societies\": 4, \"points\": " + to_string(totalPoints) + ", \"skills\": " + to_string(totalSkills) + "}";
            res.set_content(json, "application/json");
        });

        // API: Societies
        svr.Get("/api/societies", [](const httplib::Request&, httplib::Response& res) {
            auto all = directory.getAllStudents();
            stringstream oss;
            oss << "[";
            for(size_t i = 0; i < globalClubs.size(); i++) {
                int count = 0;
                for (auto s : all) {
                    if (s->getMembershipStatus() == "approved" && s->getAppliedClub() == globalClubs[i].name) {
                        count++;
                    }
                }
                oss << "{\"name\":\"" << globalClubs[i].name << "\", \"type\":\"" << globalClubs[i].type << "\", \"members\":" << count << ", \"desc\":\"" << globalClubs[i].desc << "\"}";
                if(i < globalClubs.size() - 1) oss << ",";
            }
            oss << "]";
            res.set_content(oss.str(), "application/json");
        });

        // API: Faculty Club Management
        svr.Post("/api/faculty/club", [](const httplib::Request& req, httplib::Response& res) {
            string name = extractJson(req.body, "name");
            string type = extractJson(req.body, "type");
            string desc = extractJson(req.body, "desc");
            if (!name.empty()) {
                globalClubs.push_back({name, type, desc});
                saveClubs();
            }
            res.set_content("{\"status\":\"success\"}", "application/json");
        });

        svr.Delete(R"(/api/faculty/club/([\w\s]+))", [](const httplib::Request& req, httplib::Response& res) {
            string name = req.matches[1];
            for (auto it = globalClubs.begin(); it != globalClubs.end(); ++it) {
                if (it->name == name) {
                    globalClubs.erase(it);
                    saveClubs();
                    break;
                }
            }
            res.set_content("{\"status\":\"success\"}", "application/json");
        });

        // API: Faculty Event Broadcast
        svr.Post("/api/faculty/event", [&](const httplib::Request& req, httplib::Response& res) {
            string title = extractJson(req.body, "title");
            if(!title.empty()) {
                historyStack.logEvent(title);
            }
            res.set_content("{\"status\":\"success\"}", "application/json");
        });

        // API: Faculty Hyped Club
        svr.Get("/api/faculty/hype", [](const httplib::Request&, httplib::Response& res) {
            auto all = directory.getAllStudents();
            string bestClub = "None";
            int maxCount = 0;
            
            for(size_t i = 0; i < globalClubs.size(); i++) {
                int count = 0;
                for (auto s : all) {
                    if (s->getMembershipStatus() == "approved" && s->getAppliedClub() == globalClubs[i].name) {
                        count++;
                    }
                }
                if (count > maxCount || (count == maxCount && bestClub == "None")) {
                    maxCount = count;
                    bestClub = globalClubs[i].name;
                }
            }
            string json = "{\"name\":\"" + bestClub + "\", \"members\":" + to_string(maxCount) + "}";
            res.set_content(json, "application/json");
        });

        // API: Apply
        svr.Post("/api/apply", [](const httplib::Request& req, httplib::Response& res) {
            string id = extractJson(req.body, "id");
            string club = extractJson(req.body, "club");
            string msg = extractJson(req.body, "message");
            string name = extractJson(req.body, "name");
            string email = extractJson(req.body, "email");
            string skill = extractJson(req.body, "skill");
            int year = safeStoi(extractJson(req.body, "year"), 1);
            int semester = safeStoi(extractJson(req.body, "semester"), 1);
            float cgpa = safeStof(extractJson(req.body, "cgpa"), 0.0f);
            int exp = safeStoi(extractJson(req.body, "experience"), 0);
            string domain = extractJson(req.body, "domain");

            Student* s = directory.getStudent(id);
            if (s) {
                s->setAppliedClub(club);
                s->setMembershipStatus("pending");
                s->setApplicationMessage(msg);
                s->setDomain(domain);
                leaderboard.insertOrUpdate(s);
            } else {
                if (id.empty() || name.empty()) {
                    res.status = 400;
                    return;
                }
                s = new Student(id, name, email, id, skill, year, semester, cgpa, cgpa, exp, club, "pending", msg, domain);
                directory.insert(s);
                skills.indexStudent(s);
                leaderboard.insertOrUpdate(s);
            }
            saveDatabase();
            res.set_content("{\"status\":\"success\"}", "application/json");
        });

        // API: Applications for President
        svr.Get(R"(/api/admin/applications/(.+))", [](const httplib::Request& req, httplib::Response& res) {
            string club = req.matches[1];
            // Simple space decoding
            while (club.find("%20") != string::npos) club.replace(club.find("%20"), 3, " ");

            auto all = directory.getAllStudents();
            stringstream json; json << "[";
            bool first = true;
            for (auto s : all) {
                if (s->getAppliedClub() == club && s->getMembershipStatus() == "pending") {
                    if (!first) json << ",";
                    json << "{\"id\":\"" << s->getId() << "\", \"name\":\"" << s->getName() << "\", \"points\":" << s->getMeritPoints() << ", \"message\":\"" << s->getApplicationMessage() << "\", \"domain\":\"" << s->getDomain() << "\", \"skill\":\"" << s->getSkill() << "\"}";
                    first = false;
                }
            }
            json << "]";
            res.set_content(json.str(), "application/json");
        });

        // API: Active Members for President
        svr.Get(R"(/api/admin/members/(.+))", [](const httplib::Request& req, httplib::Response& res) {
            string club = req.matches[1];
            while (club.find("%20") != string::npos) club.replace(club.find("%20"), 3, " ");

            auto all = directory.getAllStudents();
            stringstream json; json << "[";
            bool first = true;
            for (auto s : all) {
                if (s->getAppliedClub() == club && s->getMembershipStatus() == "approved") {
                    if (!first) json << ",";
                    json << "{\"id\":\"" << s->getId() << "\", \"name\":\"" << s->getName() << "\", \"points\":" << s->getMeritPoints() << ", \"skill\":\"" << s->getSkill() << "\", \"domain\":\"" << s->getDomain() << "\"}";
                    first = false;
                }
            }
            json << "]";
            res.set_content(json.str(), "application/json");
        });

        // API: Update Status
        svr.Post("/api/admin/student/status", [](const httplib::Request& req, httplib::Response& res) {
            string id = extractJson(req.body, "id");
            string status = extractJson(req.body, "status");
            string club = extractJson(req.body, "club");
            
            Student* s = directory.getStudent(id);
            if (s) {
                s->setMembershipStatus(status);
                s->setAppliedClub(club);
                saveDatabase();
                res.set_content("{\"status\":\"success\"}", "application/json");
            } else res.status = 404;
        });

        if (!svr.listen("0.0.0.0", port)) {
            std::cerr << "❌ FAILED to bind to port " << port << std::endl;
        }

        return 0;
    } catch (const exception& e) {
        cerr << "\nFATAL ERROR: " << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "\nFATAL ERROR: Unknown crash occurred." << endl;
        return 1;
    }
}