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
    string getEmail() const { return email; }
    
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
    int year;
    float cgpa;
    int experience;
    string appliedClub;
    string membershipStatus;

public:
    Student(string _id, string _name, string _email, string _skill, int _year = 1, float _cgpa = 0.0f, int _exp = 0, string _club = "none", string _status = "none") 
        : User(_id, _name, _email), primarySkill(_skill), year(_year), cgpa(_cgpa), experience(_exp), 
          appliedClub(_club), membershipStatus(_status), meritPoints(0) {
            calculateInitialMerit();
        }

    void calculateInitialMerit() {
        if (year == 1) {
            meritPoints = experience * 10;
        } else {
            meritPoints = (cgpa * 10) + (experience * 5);
        }
    }

    // Setters & Getters
    void addMeritPoints(int points) { meritPoints += points; }
    void setMeritPoints(int points) { meritPoints = points; }
    int getMeritPoints() const { return meritPoints; }
    string getSkill() const { return primarySkill; }
    int getYear() const { return year; }
    float getCgpa() const { return cgpa; }
    int getExperience() const { return experience; }
    string getAppliedClub() const { return appliedClub; }
    string getMembershipStatus() const { return membershipStatus; }

    void setAppliedClub(string club) { appliedClub = club; }
    void setMembershipStatus(string status) { membershipStatus = status; }

    // Polymorphism Override
    void displayProfile() const override {
        cout << "--- STUDENT PROFILE ---" << endl;
        cout << "ID: " << id << " | Name: " << name << endl;
        cout << "Year: " << year << " | CGPA: " << cgpa << " | Exp: " << experience << endl;
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
