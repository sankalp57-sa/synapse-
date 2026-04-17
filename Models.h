#ifndef SYNAPSE_MODELS_H
#define SYNAPSE_MODELS_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

// ==========================================
// OOP Pillar: ABSTRACTION & ENCAPSULATION
// ==========================================
class User {
protected:
    string id;
    string name;
    string email;

public:
    User(string _id, string _name, string _email) : id(_id), name(_name), email(_email) {}
    virtual ~User() {}

    string getId() const { return id; }
    string getName() const { return name; }
    string getEmail() const { return email; }
    
    void setName(string n) { name = n; }
    void setEmail(string e) { email = e; }
    
    // OOP Pillar: POLYMORPHISM (Pure Virtual)
    virtual void displayProfile() const = 0; 
};

// ==========================================
// OOP Pillar: INHERITANCE
// ==========================================
class Student : public User {
private:
    string password;
    int meritPoints;
    string primarySkill;
    int year, semester, experience;
    float cgpa, sgpa;
    string appliedClub, membershipStatus;

public:
    Student(string _id, string _name, string _email, string _pass, string _skill, 
            int _year = 1, int _sem = 1, float _cgpa = 0.0f, float _sgpa = 0.0f, 
            int _exp = 0, string _club = "none", string _status = "none") 
        : User(_id, _name, _email), password(_pass), primarySkill(_skill), 
          year(_year), semester(_sem), cgpa(_cgpa), sgpa(_sgpa), experience(_exp), 
          appliedClub(_club), membershipStatus(_status), meritPoints(0) {
            calculateInitialMerit();
        }

    void calculateInitialMerit() {
        // Simple logic for professor to see merit calculation
        meritPoints = (int)((cgpa * 10) + (experience * 5));
        if (year == 1) meritPoints += 20; // First year bonus
    }

    // Getters & Setters
    string getPassword() const { return password; }
    int getMeritPoints() const { return meritPoints; }
    void setMeritPoints(int p) { meritPoints = p; }
    string getSkill() const { return primarySkill; }
    int getYear() const { return year; }
    int getSemester() const { return semester; }
    float getCgpa() const { return cgpa; }
    float getSgpa() const { return sgpa; }
    int getExperience() const { return experience; }
    string getAppliedClub() const { return appliedClub; }
    string getMembershipStatus() const { return membershipStatus; }

    void setAppliedClub(string c) { appliedClub = c; }
    void setMembershipStatus(string s) { membershipStatus = s; }
    void setSkill(string s) { primarySkill = s; }
    void setYear(int y) { year = y; }
    void setSgpa(float s) { sgpa = s; }
    void setExperience(int e) { experience = e; }

    // OOP Pillar: POLYMORPHISM (Override)
    void displayProfile() const override {
        cout << "--- STUDENT [" << name << "] Profile ---" << endl;
        cout << "Skill: " << primarySkill << " | Merit: " << meritPoints << endl;
    }
};

class Admin : public User {
private:
    string societyName;
public:
    Admin(string _id, string _name, string _email, string _society)
        : User(_id, _name, _email), societyName(_society) {}

    void displayProfile() const override {
        cout << "--- ADMIN Society Head: " << name << " [" << societyName << "] ---" << endl;
    }
};

#endif
