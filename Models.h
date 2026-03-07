#ifndef SYNAPSE_MODELS_H
#define SYNAPSE_MODELS_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

// ==========================================
// 1. OOP PILLAR: ABSTRACTION & ENCAPSULATION
// ==========================================

// Base Abstract Class (User)
class User {
protected: // Encapsulation: Subclasses can access, but not public
    string id;
    string name;
    string email;

public:
    User(string _id, string _name, string _email) : id(_id), name(_name), email(_email) {}
    
    // Virtual Destructor
    virtual ~User() {}

    // Getters
    string getId() const { return id; }
    string getName() const { return name; }
    
    // OOP PILLAR: POLYMORPHISM
    // Pure virtual function making this an Abstract class
    virtual void displayProfile() const = 0; 
};


// ==========================================
// 2. OOP PILLAR: INHERITANCE
// ==========================================

// Derived Class 1: Student
class Student : public User {
private: // Encapsulation: Strict protection of sensitive data
    int meritPoints;
    string primarySkill;

public:
    Student(string _id, string _name, string _email, string _skill) 
        : User(_id, _name, _email), primarySkill(_skill), meritPoints(0) {}

    // Setters & Getters
    void addMeritPoints(int points) { meritPoints += points; }
    int getMeritPoints() const { return meritPoints; }
    string getSkill() const { return primarySkill; }

    // Polymorphism Override
    void displayProfile() const override {
        cout << "--- STUDENT PROFILE ---" << endl;
        cout << "ID: " << id << " | Name: " << name << endl;
        cout << "Skill: " << primarySkill << " | Merit Points: " << meritPoints << endl;
        cout << "-----------------------" << endl;
    }
};

// Derived Class 2: Admin / Society Head
class Admin : public User {
private:
    string societyName;

public:
    Admin(string _id, string _name, string _email, string _society)
        : User(_id, _name, _email), societyName(_society) {}

    // Polymorphism Override
    void displayProfile() const override {
        cout << "--- ADMIN PROFILE ---" << endl;
        cout << "Admin: " << name << " | Society: " << societyName << endl;
        cout << "ID: " << id << " | Contact: " << email << endl;
        cout << "---------------------" << endl;
    }
};

#endif // SYNAPSE_MODELS_H
