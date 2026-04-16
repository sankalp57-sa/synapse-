// script.js

// 0. Global Critical Handlers (Defined first for immediate availability)
window.loginWithId = async function() {
    const id = document.getElementById('login-id').value;
    const error = document.getElementById('login-error');
    if (!id) return;

    try {
        const response = await fetch(`/api/student/${id.trim()}`);
        if (response.ok) {
            const user = await response.json();
            if (window.onLoginSuccess) window.onLoginSuccess({ id: user.id, name: user.name, points: user.points });
        } else {
            if (error) {
                error.style.display = 'block';
                error.innerText = "Student ID not found.";
            }
        }
    } catch (err) {
        if (error) {
            error.style.display = 'block';
            error.innerText = "Connection error. Is the backend server running?";
        }
    }
};

window.logout = function() {
    localStorage.removeItem('synapse_user');
    window.location.reload();
};

window.selectRole = function(role) {
    const studentBox = document.getElementById('student-login-box');
    const presBox = document.getElementById('president-login-box');
    const errorMsg = document.getElementById('login-error');
    if(errorMsg) errorMsg.style.display = 'none';

    if (role === 'student') {
        if (presBox) presBox.style.display = 'none';
        if (studentBox) studentBox.style.display = 'block';
    } else if (role === 'president') {
        if (studentBox) studentBox.style.display = 'none';
        if (presBox) presBox.style.display = 'block';
    } else {
        const overlay = document.getElementById('role-modal');
        if (overlay) {
            overlay.style.opacity = '0';
            setTimeout(() => overlay.style.display = 'none', 300);
        }
    }
};

document.addEventListener('DOMContentLoaded', () => {
    // 1. State
    const savedUser = localStorage.getItem('synapse_user');
    const savedPres = localStorage.getItem('synapse_president_club');

    window.initApp = function() {
        if (typeof window.loadStudents === 'function') window.loadStudents();
        if (typeof window.loadEvents === 'function') window.loadEvents();
        
        if (savedUser) {
            try { showUser(JSON.parse(savedUser)); } catch(e) {}
        }
        if (savedPres) {
            if (window.showPresidentDashboard) window.showPresidentDashboard(savedPres);
        }
    };

    // 2. Fetching & Display
    window.loadStudents = async function() {
        const grid = document.getElementById('student-grid');
        try {
            const response = await fetch('/api/students');
            const students = await response.json();
            if (!students || students.length === 0) {
                grid.innerHTML = '<div style="text-align:center; width:100%;">No students created yet.</div>';
                return;
            }
            grid.innerHTML = ''; 
            students.sort((a, b) => b.points - a.points);
            students.forEach((student, index) => {
                const card = document.createElement('div');
                card.className = 'student-card';
                card.setAttribute('data-skill', student.skill.toLowerCase());
                let bCls = index === 0 ? ' badge-gold' : index === 1 ? ' badge-silver' : index === 2 ? ' badge-bronze' : '';
                card.innerHTML = `
                    <div class="rank${bCls}">#${index + 1}</div>
                    <div class="student-info">
                        <img src="https://ui-avatars.com/api/?name=${encodeURIComponent(student.name)}&background=random&color=fff" class="avatar">
                        <div class="details"><h4>${student.name}</h4><p>${student.id}</p></div>
                    </div>
                    <div class="student-skills"><span class="skill-tag">${student.skill}</span></div>
                    <div class="merit-score"><div class="score-ring"><span>${student.points}</span></div><p>Points</p></div>
                `;
                grid.appendChild(card);
            });
            setupFilters();
        } catch (err) { }
    }

    window.loadEvents = async function() {
        try {
            const r = await fetch('/api/events');
            const evs = await r.json();
            const container = document.getElementById('live-events-container');
            if (evs && evs.length > 0) {
                container.innerHTML = '';
                evs.forEach(ev => {
                    container.innerHTML += `<div class="live-event" style="padding:15px; margin-bottom:10px; background:rgba(255,255,255,0.05); border-left:4px solid var(--text-secondary); border-radius:4px;"><i class="fa-solid fa-history"></i> ${ev.desc}</div>`;
                });
            }
        } catch (e) {}
    }

    function setupFilters() {
        const filterBtns = document.querySelectorAll('.filter-btn');
        const studentCards = document.querySelectorAll('.student-card');
        filterBtns.forEach(btn => {
            btn.addEventListener('click', () => {
                filterBtns.forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                const val = btn.getAttribute('data-filter');
                studentCards.forEach(card => {
                    const skill = card.getAttribute('data-skill');
                    card.style.display = (val === 'all' || (skill && skill.includes(val))) ? 'flex' : 'none';
                });
            });
        });
    }

    // Fixed 'this' usage by specifying target explicitly from anchor context
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function (e) {
            e.preventDefault();
            const href = anchor.getAttribute('href');
            if (href === '#') return;
            const target = document.querySelector(href);
            if (target) target.scrollIntoView({ behavior: 'smooth' });
        });
    });

    window.onLoginSuccess = function(user) {
        localStorage.setItem('synapse_user', JSON.stringify(user));
        showUser(user);
    };

    function showUser(user) {
        // 1. Hero Section Profile Card
        const profile = document.getElementById('user-display-status');
        if (profile) {
            profile.style.display = 'block';
            document.getElementById('display-name').innerText = user.name;
            document.getElementById('display-id').innerText = user.id;
            document.getElementById('display-avatar').src = `https://ui-avatars.com/api/?name=${user.name.replace(' ', '+')}&background=random`;
        }

        // 2. Navbar Sync
        const navUser = document.getElementById('user-display');
        const navUserName = document.getElementById('user-name');
        const navJoinBtn = document.getElementById('nav-join-btn');
        const navAdminBtn = document.getElementById('nav-admin-btn');
        const applySection = document.getElementById('apply');

        if (navUser) navUser.style.display = 'flex';
        if (navUserName) navUserName.innerText = user.name;
        if (navJoinBtn) navJoinBtn.style.display = 'inline-flex';
        if (navAdminBtn) navAdminBtn.style.display = 'none';
        if (applySection) applySection.style.display = 'block';

        // 3. Status Check
        fetch(`/api/student/${user.id}`).then(res => res.json()).then(data => {
            if(data.status === "success") {
                const statusText = document.getElementById('display-status');
                if (statusText) {
                    const status = data.membershipStatus || "none";
                    const club = data.appliedClub || "";
                    statusText.innerText = status === "none" ? "(NOT-MEMBER)" : 
                                         status === "pending" ? `(PENDING: ${club})` :
                                         `(MEMBER: ${club})`;
                    statusText.style.color = status === "accepted" ? "#00e676" : "#3b82f6";
                }
            }
        });

        // 4. Modal Cleanup
        const roleModal = document.getElementById('role-modal');
        if (roleModal) {
            roleModal.style.opacity = '0';
            setTimeout(() => roleModal.style.display = 'none', 300);
        }
    }

    window.logoutPresident = () => {
        localStorage.removeItem('synapse_president_club');
        location.reload();
    };

    // Close Modal when clicking outside
    const societyModal = document.getElementById('society-modal');
    if (societyModal) {
        societyModal.addEventListener('click', (e) => {
            if (e.target === societyModal) societyModal.classList.remove('active');
        });
    }

    // Init sessions
    const savedPresValue = localStorage.getItem('synapse_president_club');
    if (savedPresValue) {
        if (window.showPresidentDashboard) window.showPresidentDashboard(savedPresValue);
    }

    window.initApp();
});
