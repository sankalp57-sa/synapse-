// script.js - Milestone 2 Logic

// 0. Global Critical Handlers
window.loginWithId = async function() {
    const id = document.getElementById('login-id').value;
    const error = document.getElementById('login-error');
    if (!id) return;

    try {
        const response = await fetch(`/api/student/${id.trim()}`);
        if (response.ok) {
            const user = await response.json();
            window.onLoginSuccess({ id: user.id, name: user.name, points: user.points });
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
    const overlay = document.getElementById('role-modal');

    if(errorMsg) errorMsg.style.display = 'none';

    if (role === 'student') {
        if (presBox) presBox.style.display = 'none';
        if (studentBox) studentBox.style.display = 'block';
    } else if (role === 'president') {
        if (studentBox) studentBox.style.display = 'none';
        if (presBox) presBox.style.display = 'block';
    } else if (role === 'faculty') {
            if (overlay) {
                localStorage.setItem('synapse_role', 'faculty');
                window.closeModal(overlay, () => {
                    window.location.href = 'faculty.html';
                });
            }
        } else {
            // Default (Portal Access clicked)
            if (overlay) overlay.style.display = 'flex';
        }
    };
    
    window.closeModal = function(modal, callback) {
        if (!modal) return;
        modal.style.opacity = '0';
        setTimeout(() => modal.style.transform = 'translateY(20px)', 100);
        setTimeout(() => {
            modal.style.display = 'none';
            if(callback) callback();
        }, 300);
    };

window.onLoginSuccess = function(user) {
    localStorage.setItem('synapse_user', JSON.stringify(user));
    window.showUser(user);
};

window.showUser = function(user) {
    // 1. Hero Overlay Show
    const profile = document.getElementById('user-display-status');
    if (profile) profile.style.display = 'block';
    
    document.getElementById('display-name').innerText = user.name;
    document.getElementById('display-id').innerText = user.id;
    const avatar = document.getElementById('display-avatar');
    if (avatar) avatar.src = `https://ui-avatars.com/api/?name=${user.name.replace(' ', '+')}&background=random`;

    // 2. Navbar Sync
    const uDisplay = document.getElementById('user-display');
    const uName = document.getElementById('user-name');
    const jBtn = document.getElementById('nav-join-btn');
    const lBtn = document.getElementById('nav-login-btn-init');
    const applySec = document.getElementById('apply');

    if (uDisplay) uDisplay.style.display = 'flex';
    if (uName) uName.innerText = user.name.split(' ')[0];
    if (jBtn) jBtn.style.display = 'inline-flex';
    if (lBtn) lBtn.style.display = 'none';
    if (applySec) applySec.style.display = 'block';
    
    if (document.getElementById('apply-id')) document.getElementById('apply-id').value = user.id;
    if (document.getElementById('name')) document.getElementById('name').value = user.name;

    // 3. Status Check from Backend
    fetch(`/api/student/${user.id}`).then(res => res.json()).then(data => {
        if(data.status === "success") {
            const sText = document.getElementById('display-status');
            const status = data.membershipStatus || "none";
            const club = data.appliedClub || "";
            
            if (sText) {
                if (status === "none") sText.innerText = "(NOT-MEMBER)";
                else if (status === "pending") sText.innerText = `(PENDING: ${club})`;
                else {
                    sText.innerText = `(MEMBER: ${club})`;
                    sText.style.color = "#00e676";
                }
            }
        }
    });

    // 4. Cleanup Modal
    window.closeModal(document.getElementById('role-modal'));
};

window.loginPresidentMain = async function() {
    const club = document.getElementById('login-club-main').value;
    const errorMsg = document.getElementById('login-error');

    try {
        const res = await fetch('/api/admin/login', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ role: "president", club })
        });
        if (res.ok) {
            localStorage.setItem('synapse_role', 'president');
            localStorage.setItem('synapse_club', club);
            window.location.href = "admin.html"; // Redirect to dashboard
        } else {
            if (errorMsg) {
                errorMsg.innerText = "❌ Login Failed. Ensure backend is running.";
                errorMsg.style.display = 'block';
            }
        }
    } catch (err) {
        console.error("Login error:", err);
    }
};

document.addEventListener('DOMContentLoaded', () => {
    // 1. App State
    const savedUser = localStorage.getItem('synapse_user');

    window.initApp = function() {
        loadStudents();
        loadEvents();
        loadSocieties();
        loadStats();
        
        if (savedUser) {
            try { window.showUser(JSON.parse(savedUser)); } catch(e) {}
        } else {
            // Force portal entry on first visit
            window.selectRole('');
        }
    };

    // 2. Fetching & Display
    async function loadStudents() {
        const grid = document.getElementById('student-grid');
        try {
            const response = await fetch('/api/students');
            const students = await response.json();
            
            if (!students || students.length === 0) {
                grid.innerHTML = '<p style="text-align:center; padding:2rem;">No candidates ranked in the heap yet.</p>';
                return;
            }

            grid.innerHTML = ''; 
            students.sort((a, b) => b.points - a.points);
            
            students.forEach((student, index) => {
                const card = document.createElement('div');
                card.className = 'student-card';
                card.setAttribute('data-skill', (student.skill || "").toLowerCase());
                
                let bCls = index === 0 ? ' badge-gold' : index === 1 ? ' badge-silver' : index === 2 ? ' badge-bronze' : '';
                
                card.innerHTML = `
                    <div class="rank${bCls}">#${index + 1}</div>
                    <div class="student-info">
                        <img src="https://ui-avatars.com/api/?name=${encodeURIComponent(student.name)}&background=random&color=fff" class="avatar">
                        <div class="details"><h4>${student.name}</h4><p>${student.id}</p></div>
                    </div>
                    <div class="student-skills"><span class="skill-tag">${student.skill || "Developer"}</span></div>
                    <div class="merit-score"><div class="score-ring"><span>${student.points}</span></div><p>Points</p></div>
                `;
                grid.appendChild(card);
            });
            setupFilters();
        } catch (err) {
            console.error("Failed to load students:", err);
            grid.innerHTML = '<div style="color:#ff4d4d; text-align:center;">Error connecting to C++ Merit Engine.</div>';
        }
    }

    async function loadStats() {
        try {
            const res = await fetch('/api/stats');
            const data = await res.json();
            const statSoc = document.getElementById('stat-societies');
            const statPts = document.getElementById('stat-points');
            const statSkl = document.getElementById('stat-skills');
            
            if(statSoc) statSoc.innerText = data.societies + "+";
            if(statPts) statPts.innerText = data.points.toLocaleString() + "+";
            if(statSkl) statSkl.innerText = data.skills + "+";
        } catch(e) { console.error("Stats fail:", e); }
    }

    async function loadEvents() {
        try {
            const r = await fetch('/api/events');
            const evs = await r.json();
            const container = document.getElementById('live-events-container');
            if (evs && evs.length > 0) {
                container.innerHTML = evs.reverse().map(ev => `
                    <div class="live-event" style="padding:15px; margin-bottom:10px; background:rgba(255,255,255,0.05); border-left:4px solid var(--accent-purple); border-radius:8px;">
                        <i class="fa-solid fa-clock-rotate-left"></i> ${typeof ev === 'string' ? ev : ev.desc}
                    </div>
                `).join('');
            }
        } catch (e) { console.error("History failed:", e); }
    }

    async function loadSocieties() {
        const grid = document.getElementById('society-grid');
        const loginSelect = document.getElementById('login-club-main');
        const applySelect = document.getElementById('club');
        try {
            const res = await fetch('/api/societies');
            const data = await res.json();
            
            // Render Cards
            if (grid) {
                grid.innerHTML = data.map(s => `
                    <div class="society-card">
                        <div class="society-icon ${s.type}">
                            <i class="fa-solid ${s.type === 'tech' ? 'fa-code' : (s.type === 'hardware' ? 'fa-microchip' : (s.type === 'design' ? 'fa-pen-nib' : 'fa-microphone'))}"></i>
                        </div>
                        <h3>${s.name}</h3>
                        <p>${s.desc}</p>
                        <div class="society-footer">
                            <span class="members"><i class="fa-solid fa-users"></i> ${s.members} Members</span>
                            <a href="#apply" class="view-link" onclick="prefillClub('${s.name}')">Join <i class="fa-solid fa-arrow-right"></i></a>
                        </div>
                    </div>
                `).join('');
            }

            // Populate Dropdowns Dynamically
            let optionsHTML = data.map(s => `<option value="${s.name}">${s.name} Society</option>`).join('');
            if (loginSelect) loginSelect.innerHTML = optionsHTML;
            if (applySelect) applySelect.innerHTML = optionsHTML;
            
        } catch (e) { console.error("Societies failed:", e); }
    }

    window.prefillClub = function(clubName) {
        const clubSelect = document.getElementById('club');
        if (clubSelect) {
            clubSelect.value = clubName;
        }
    };

    function setupFilters() {
        const filterBtns = document.querySelectorAll('.filter-btn');
        filterBtns.forEach(btn => {
            btn.addEventListener('click', () => {
                const val = btn.getAttribute('data-filter');
                filterBtns.forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                
                document.querySelectorAll('.student-card').forEach(card => {
                    const skill = card.getAttribute('data-skill');
                    card.style.display = (val === 'all' || (skill && skill.includes(val))) ? 'flex' : 'none';
                });
            });
        });
    }


    // 3. Application Submission
    const applyForm = document.getElementById('applyForm');
    if (applyForm) {
        applyForm.addEventListener('submit', async (e) => {
            e.preventDefault();
            const id = document.getElementById('apply-id').value;
            const name = document.getElementById('name').value;
            const email = document.getElementById('email').value;
            const skill = document.getElementById('skill').value;
            const year = parseInt(document.getElementById('year').value) || 1;
            const semester = parseInt(document.getElementById('semester').value) || 1;
            const cgpa = parseFloat(document.getElementById('cgpa').value) || 0.0;
            const experience = parseInt(document.getElementById('experience').value) || 0;
            const club = document.getElementById('club').value;
            const domain = document.getElementById('domain').value;
            const message = document.getElementById('apply-message').value;
            const responseMsg = document.getElementById('responseMsg');

            try {
                const response = await fetch('/api/apply', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ id, name, email, skill, year, semester, cgpa, experience, club, domain, message })
                });
                
                if (response.ok) {
                    responseMsg.innerText = "✅ Application submitted! President review pending.";
                    responseMsg.style.color = "#00e676";
                    setTimeout(() => { window.location.reload(); }, 2000);
                } else {
                    responseMsg.innerText = "❌ Submission failed. Try again.";
                    responseMsg.style.color = "#ff4d4d";
                }
            } catch (err) {
                console.error(err);
                responseMsg.innerText = "❌ Connection error.";
            }
        });
    }

    // 4. Double Click for Fresh Application
    const studentInitBtn = document.querySelector('button[onclick*="student"]');
    if (studentInitBtn) {
        studentInitBtn.addEventListener('dblclick', () => {
            // Force open apply section
            const applySec = document.getElementById('apply');
            if (applySec) {
                applySec.style.display = 'block';
                applySec.scrollIntoView({ behavior: 'smooth' });
                // If not logged in, suggest logging in first
                if (!localStorage.getItem('synapse_user')) {
                    const idInput = document.getElementById('apply-id');
                    if (idInput) {
                        idInput.readOnly = false;
                        idInput.style.cursor = 'text';
                        idInput.style.background = 'rgba(0,0,0,0.2)';
                        idInput.placeholder = "Enter ID to Apply";
                    }
                }
            }
            // Close modal
            const roleModal = document.getElementById('role-modal');
            if (roleModal) roleModal.style.display = 'none';
        });
    }

    window.initApp();
});
