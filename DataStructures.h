#ifndef SYNAPSE_DATA_STRUCTURES_H
#define SYNAPSE_DATA_STRUCTURES_H

#include "Models.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// ==========================================
// DATA STRUCTURE 1: HASH TABLE (w/ Chaining)
// User lookup by ID -> O(1) Time Complexity
// ==========================================

struct HashNode {
  Student *student;
  HashNode *next;
  HashNode(Student *s) : student(s), next(nullptr) {}
};

class StudentDirectory {
private:
  static const int TABLE_SIZE = 100;
  HashNode *table[TABLE_SIZE];

  // Simple Hash Function
  int hashFunction(string id) {
    int sum = 0;
    for (char c : id) {
      sum += c;
    }
    return sum % TABLE_SIZE;
  }

public:
  StudentDirectory() {
    for (int i = 0; i < TABLE_SIZE; i++) {
      table[i] = nullptr;
    }
  }

  // Insert Student (Handles Collisions via Linked List)
  void insert(Student *s) {
    int index = hashFunction(s->getId());
    HashNode *newNode = new HashNode(s);

    // Insert at head of linked list
    if (table[index] == nullptr) {
      table[index] = newNode;
    } else {
      newNode->next = table[index];
      table[index] = newNode;
    }
  }

  // Lookup Student by ID O(1) average
  Student *getStudent(string id) {
    int index = hashFunction(id);
    HashNode *current = table[index];

    while (current != nullptr) {
      if (current->student->getId() == id) {
        return current->student;
      }
      current = current->next;
    }
    return nullptr; // Not found
  }

  // Retrieve all students for leaderboard/JSON
  vector<Student *> getAllStudents() {
    vector<Student *> all;
    for (int i = 0; i < TABLE_SIZE; i++) {
      HashNode *current = table[i];
      while (current != nullptr) {
        all.push_back(current->student);
        current = current->next;
      }
    }
    return all;
  }
};

// ==========================================
// DATA STRUCTURE 2: BINARY SEARCH TREE (BST)
// Organizes students by primary skill -> O(log N)
// ==========================================

struct TreeNode {
  string skill;
  Student *student;
  TreeNode *left;
  TreeNode *right;

  TreeNode(string s, Student *stud)
      : skill(s), student(stud), left(nullptr), right(nullptr) {}
};

class SkillIndex {
private:
  TreeNode *root;

  TreeNode *insertRec(TreeNode *node, string skill, Student *student) {
    if (node == nullptr) {
      return new TreeNode(skill, student);
    }
    if (skill < node->skill) {
      node->left = insertRec(node->left, skill, student);
    } else {
      node->right = insertRec(node->right, skill, student);
    }
    return node;
  }

  void searchSkillRec(TreeNode *node, string targetSkill, bool &found) {
    if (node == nullptr)
      return;

    searchSkillRec(node->left, targetSkill, found);

    if (node->skill == targetSkill) {
      cout << " - " << node->student->getName()
           << " (ID: " << node->student->getId() << ")" << endl;
      found = true;
    }

    searchSkillRec(node->right, targetSkill, found);
  }

public:
  SkillIndex() : root(nullptr) {}

  void indexStudent(Student *student) {
    root = insertRec(root, student->getSkill(), student);
  }

  void findStudentsBySkill(string skill) {
    bool found = false;
    cout << "\n--- Students with skill: " << skill << " ---" << endl;
    searchSkillRec(root, skill, found);
    if (!found)
      cout << "No students found with this skill." << endl;
    cout << "-----------------------------------" << endl;
  }
};

// ==========================================
// DATA STRUCTURE 3: MAX HEAP (Priority Queue)
// Auto-sorts Merit Leaderboard -> O(log N) insert
// ==========================================

class LeaderboardHeap {
private:
  vector<Student *> heap;

  int parent(int i) { return (i - 1) / 2; }
  int left(int i) { return (2 * i) + 1; }
  int right(int i) { return (2 * i) + 2; }

  void heapifyUp(int index) {
    while (index != 0 && heap[parent(index)]->getMeritPoints() <
                             heap[index]->getMeritPoints()) {
      swap(heap[index], heap[parent(index)]);
      index = parent(index);
    }
  }

public:
  void insertOrUpdate(Student *student) {
    heap.push_back(student);
    heapifyUp(heap.size() - 1);
  }

  void displayLeaderboard(int topN = 3) {
    cout << "\n🏆 --- SYNAPSE LEADERBOARD --- 🏆" << endl;
    if (heap.empty()) {
      cout << "No students on the leaderboard yet." << endl;
      return;
    }

    int count = min((int)heap.size(), topN);
    for (int i = 0; i < count; i++) {
      cout << i + 1 << ". " << heap[i]->getName() << " -> "
           << heap[i]->getMeritPoints() << " Points" << endl;
    }
    cout << "---------------------------------\n" << endl;
  }

  vector<Student*> getTopStudents(int n) {
      vector<Student*> top = heap;
      sort(top.begin(), top.end(), [](Student* a, Student* b) {
          return a->getMeritPoints() > b->getMeritPoints();
      });
      if (top.size() > n) top.resize(n);
      return top;
  }
};

// ==========================================
// DATA STRUCTURE 4: STACK (Linked List Based)
// LIFO history of Society Events -> O(1) push/pop
// ==========================================

struct EventNode {
  string eventDescription;
  EventNode *next;
  EventNode(string desc) : eventDescription(desc), next(nullptr) {}
};

class EventHistory {
private:
  EventNode *top;

public:
  EventHistory() : top(nullptr) {}

  void logEvent(string description) {
    EventNode *newNode = new EventNode(description);
    newNode->next = top;
    top = newNode;
    cout << "[LOGGED]: " << description << endl;
  }

  void undoLastEvent() {
    if (top == nullptr) {
      cout << "No history to undo." << endl;
      return;
    }
    EventNode *temp = top;
    top = top->next;
    cout << "[UNDO]: Removing event -> " << temp->eventDescription << endl;
    delete temp;
  }

  void viewLatest() {
    if (top == nullptr) {
      cout << "No recent events." << endl;
      return;
    }
    cout << "[LATEST EVENT]: " << top->eventDescription << endl;
  }

  vector<string> getAllEvents() {
    vector<string> all;
    EventNode *current = top;
    while (current != nullptr) {
      all.push_back(current->eventDescription);
      current = current->next;
    }
    return all;
  }
};

// ==========================================
// DATA STRUCTURE 5: GRAPH (Adjacency List)
// Student Networking -> O(V + E) BFS/DFS
// ==========================================

struct EdgeNode {
  Student* student;
  EdgeNode* next;
  EdgeNode(Student* s) : student(s), next(nullptr) {}
};

class StudentNetwork {
private:
  static const int MAX_VERTICES = 100;
  EdgeNode* adjList[MAX_VERTICES];

  int hashFunction(string id) {
    int sum = 0;
    for (char c : id) sum += c;
    return sum % MAX_VERTICES;
  }

public:
  StudentNetwork() {
    for (int i = 0; i < MAX_VERTICES; i++) {
        adjList[i] = nullptr;
    }
  }

  void addConnection(Student* u, Student* v) {
      if(!u || !v) return;
      int uIndex = hashFunction(u->getId());
      int vIndex = hashFunction(v->getId());

      EdgeNode* edge1 = new EdgeNode(v);
      edge1->next = adjList[uIndex];
      adjList[uIndex] = edge1;

      EdgeNode* edge2 = new EdgeNode(u);
      edge2->next = adjList[vIndex];
      adjList[vIndex] = edge2;
  }

  void displayNetwork(Student* start) {
      if(!start) return;
      int startIndex = hashFunction(start->getId());
      cout << "\n🌐 --- Student Network for " << start->getName() << " --- 🌐" << endl;
      
      EdgeNode* current = adjList[startIndex];
      if(!current) {
          cout << start->getName() << " has no connections yet." << endl;
      }
      while(current) {
          cout << " - Connected to: " << current->student->getName() << " (Skill: " << current->student->getSkill() << ")" << endl;
          current = current->next;
      }
      cout << "--------------------------------------\n" << endl;
  }
};

#endif // SYNAPSE_DATA_STRUCTURES_H
