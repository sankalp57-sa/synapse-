#ifndef SYNAPSE_DATA_STRUCTURES_H
#define SYNAPSE_DATA_STRUCTURES_H

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <map>
#include "Models.h"

using namespace std;

// ---------------------------------------------------------
// 1. HASH TABLE (Student Directory)
//    - Used for O(1) average time complexity ID lookups.
// ---------------------------------------------------------
class StudentDirectory {
private:
    static const int TABLE_SIZE = 100;
    vector<Student*> table[TABLE_SIZE];

    int hashFunction(string id) {
        int sum = 0;
        for (char c : id) sum += c;
        return sum % TABLE_SIZE;
    }

public:
    void insert(Student* s) {
        int idx = hashFunction(s->getId());
        table[idx].push_back(s);
    }

    Student* getStudent(string id) {
        int idx = hashFunction(id);
        for (auto s : table[idx]) {
            if (s->getId() == id) return s;
        }
        return nullptr;
    }

    vector<Student*> getAllStudents() {
        vector<Student*> all;
        for (int i = 0; i < TABLE_SIZE; ++i) {
            for (auto s : table[i]) all.push_back(s);
        }
        return all;
    }
};

// ---------------------------------------------------------
// 2. BINARY SEARCH TREE (Skill Index)
//    - Used for searching students based on specific skills.
// ---------------------------------------------------------
struct BSTNode {
    string skill;
    vector<Student*> students;
    BSTNode *left, *right;
    BSTNode(string s) : skill(s), left(nullptr), right(nullptr) {}
};

class SkillIndex {
private:
    BSTNode* root = nullptr;

    BSTNode* insert(BSTNode* node, Student* s) {
        if (!node) {
            BSTNode* n = new BSTNode(s->getSkill());
            n->students.push_back(s);
            return n;
        }
        if (s->getSkill() == node->skill) {
            node->students.push_back(s);
        } else if (s->getSkill() < node->skill) {
            node->left = insert(node->left, s);
        } else {
            node->right = insert(node->right, s);
        }
        return node;
    }

public:
    void indexStudent(Student* s) { root = insert(root, s); }
    
    void findStudentsBySkill(string skill) {
        BSTNode* curr = root;
        while (curr) {
            if (curr->skill == skill) {
                cout << "Students with skill [" << skill << "]:\n";
                for (auto s : curr->students) cout << " - " << s->getName() << endl;
                return;
            }
            curr = (skill < curr->skill) ? curr->left : curr->right;
        }
        cout << "No students found with skill: " << skill << endl;
    }
};

// ---------------------------------------------------------
// 3. MAX HEAP (Leaderboard)
//    - Used to efficiently find the top-performing students.
// ---------------------------------------------------------
class LeaderboardHeap {
private:
    vector<Student*> heap;

    void bubbleUp(int idx) {
        while (idx > 0 && heap[(idx - 1) / 2]->getMeritPoints() < heap[idx]->getMeritPoints()) {
            swap(heap[idx], heap[(idx - 1) / 2]);
            idx = (idx - 1) / 2;
        }
    }

public:
    void insertOrUpdate(Student* s) {
        for(int i = 0; i < heap.size(); ++i) {
            if(heap[i]->getId() == s->getId()) {
                // If found, we just re-sort the whole heap for simplicity in this demo
                // A better way would be bubbleUp/Down from this index
                std::sort(heap.begin(), heap.end(), [](Student* a, Student* b) {
                    return a->getMeritPoints() > b->getMeritPoints();
                });
                return;
            }
        }
        heap.push_back(s);
        std::sort(heap.begin(), heap.end(), [](Student* a, Student* b) {
            return a->getMeritPoints() > b->getMeritPoints();
        });
    }

    void displayLeaderboard(int topN) {
        cout << "\n--- TOP " << topN << " MERIT LEADERBOARD ---\n";
        for (int i = 0; i < min((int)heap.size(), topN); ++i) {
            cout << i + 1 << ". " << heap[i]->getName() << " (" << heap[i]->getMeritPoints() << " pts)" << endl;
        }
    }
};

// ---------------------------------------------------------
// 4. STACK (Event History)
//    - Used for Undo operations (LIFO - Last In First Out).
// ---------------------------------------------------------
class EventHistory {
private:
    stack<string> events;

public:
    void logEvent(string desc) { events.push(desc); }
    
    void undoLastEvent() {
        if (!events.empty()) {
            cout << "Undoing: " << events.top() << endl;
            events.pop();
        }
    }

    vector<string> getAllEvents() {
        stack<string> temp = events;
        vector<string> res;
        while(!temp.empty()) {
            res.push_back(temp.top());
            temp.pop();
        }
        return res;
    }
};

// ---------------------------------------------------------
// 5. GRAPH (Student Connections)
//    - Used to model relationships between students.
// ---------------------------------------------------------
class StudentNetwork {
private:
    map<string, vector<string>> adj; 

public:
    void addConnection(Student* s1, Student* s2) {
        adj[s1->getId()].push_back(s2->getId());
        adj[s2->getId()].push_back(s1->getId());
    }

    void displayNetwork(Student* s) {
        cout << "Network connections for " << s->getName() << ":\n";
        for (const string& neighborId : adj[s->getId()]) cout << " <-> " << neighborId << endl;
    }
};

#endif
