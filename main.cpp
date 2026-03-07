#include "DataStructures.h"
#include "Models.h"
#include <iostream>
#include <string>


using namespace std;

// Initialize our core Data Structures
StudentDirectory directory;  // Hash Table
SkillIndex skills;           // BST
LeaderboardHeap leaderboard; // Max Heap
EventHistory historyStack;   // Stack

void displayMenu() {
  cout << "\n=============================================" << endl;
  cout << "     SYNAPSE SOCIETY MANAGEMENT SYSTEM       " << endl;
  cout << "=============================================" << endl;
  cout << "1. Create Student Profile (Hash + BST)" << endl;
  cout << "2. View Student Profile (Hash Table O(1))" << endl;
  cout << "3. Find Students by Skill (BST O(log N))" << endl;
  cout << "4. Log Society Event (Stack Push)" << endl;
  cout << "5. Undo Last Event (Stack Pop)" << endl;
  cout << "6. Add Merit Points & View Leaderboard (Max Heap)" << endl;
  cout << "7. Exit" << endl;
  cout << "=============================================" << endl;
  cout << "Select an option: ";
}

int main() {
  int choice;

  // Seed some initial data to demonstrate polymorphism
  Admin admin("A001", "Professor Smith", "smith@college.edu",
              "Computer Science Dept");
  cout << "\n[SYSTEM STARTED BY ADMIN]" << endl;
  User *currentUser = &admin; // Polymorphism: User pointer holding Admin object
  currentUser->displayProfile(); // Calls Admin's displayProfile()

  while (true) {
    displayMenu();
    if (!(cin >> choice)) {
      cout << "Invalid input. Exiting..." << endl;
      break;
    }

    switch (choice) {
    case 1: {
      string id, name, email, skill;
      cout << "Enter Student ID (e.g., S101): ";
      cin >> id;
      cout << "Enter Name: ";
      cin.ignore();
      getline(cin, name);
      cout << "Enter Email: ";
      cin >> email;
      cout << "Enter Primary Skill (e.g., C++, React, Design): ";
      cin >> skill;

      // OOP: Instantiating Student object
      Student *newStudent = new Student(id, name, email, skill);

      // DS: Insert into Hash Table and BST
      directory.insert(newStudent);
      skills.indexStudent(newStudent);

      cout << "\n[SUCCESS] Student " << name
           << " added to Directory and Skill Index." << endl;
      break;
    }
    case 2: {
      string searchId;
      cout << "Enter Student ID to lookup: ";
      cin >> searchId;

      // DS: O(1) Lookup via Hash Table
      Student *found = directory.getStudent(searchId);
      if (found) {
        cout << "\n[FOUND via HASH TABLE O(1)]" << endl;
        found->displayProfile(); // OOP: Polymorphism in action
      } else {
        cout << "\n[ERROR] Student ID not found in Hash Table." << endl;
      }
      break;
    }
    case 3: {
      string targetSkill;
      cout << "Enter skill to search for: ";
      cin >> targetSkill;

      // DS: O(log N) Lookup via BST
      skills.findStudentsBySkill(targetSkill);
      break;
    }
    case 4: {
      string eventDesc;
      cout << "Describe event: ";
      cin.ignore();
      getline(cin, eventDesc);

      // DS: Push to Stack
      historyStack.logEvent(eventDesc);
      break;
    }
    case 5: {
      // DS: Pop from Stack
      historyStack.undoLastEvent();
      historyStack.viewLatest();
      break;
    }
    case 6: {
      string sid;
      int points;
      cout << "Enter Student ID to award points: ";
      cin >> sid;
      Student *s = directory.getStudent(sid);

      if (s) {
        cout << "Enter points to award: ";
        cin >> points;
        // OOP: Encapsulation (using setter)
        s->addMeritPoints(points);

        // DS: Insert into Max Heap
        leaderboard.insertOrUpdate(s);
        cout << "\n[SUCCESS] Awarded " << points << " points to "
             << s->getName() << endl;

        // Display updated Heap
        leaderboard.displayLeaderboard(3);
      } else {
        cout << "\n[ERROR] Student ID not found." << endl;
      }
      break;
    }
    case 7:
      cout << "Shutting down system..." << endl;
      return 0;
    default:
      cout << "Invalid choice. Try again." << endl;
    }
  }

  return 0;
}