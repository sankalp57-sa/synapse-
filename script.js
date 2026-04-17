// Synapse - Modern Frontend Logic
document.addEventListener('DOMContentLoaded', () => {
    
    // 1. Session Initialization
    const initApp = () => {
        const user = JSON.parse(localStorage.getItem('synapse_user') || 'null');
        if (user) showUser(user);
        loadLeaderboard();
    };

    // 2. Role Selection and Login Logic
    window.selectRole = (role) => {
        const sBox = document.getElementById('student-login-box');
        const pBox = document.getElementById('president-login-box');
        const error = document.getElementById('login-error');
        
        error.style.display = 'none';
        sBox.style.display = role === 'student' ? 'block' : 'none';
        pBox.style.display = role === 'president' ? 'block' : 'none';
    };

    window.loginWithId = async () => {
        const id = document.getElementById('login-id').value;
        const error = document.getElementById('login-error');
        if (!id) return;

        try {
            const res = await fetch(`/api/student/${id.trim()}`);
            if (res.ok) {
                const user = await res.json();
                localStorage.setItem('synapse_user', JSON.stringify(user));
                showUser(user);
                // Hide modal
                const modal = document.getElementById('role-modal');
                modal.style.opacity = '0';
                setTimeout(() => modal.style.display = 'none', 300);
            } else {
                error.innerText = "Student ID not found.";
                error.style.display = 'block';
            }
        } catch (err) {
            error.innerText = "Connection error. Is the server running?";
            error.style.display = 'block';
        }
    };

    window.loginAsPresident = async () => {
        const pass = document.getElementById('president-pass').value;
        const error = document.getElementById('login-error');
        if (!pass) return;

        try {
            const res = await fetch('/api/admin/login', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ password: pass })
            });

            if (res.ok) {
                localStorage.setItem('synapse_role', 'president');
                window.location.href = 'admin.html';
            } else {
                error.innerText = "Invalid President Password.";
                error.style.display = 'block';
            }
        } catch (err) {
            error.innerText = "Connection error. Is the server running?";
            error.style.display = 'block';
        }
    };

    const showUser = (user) => {
        document.getElementById('user-display').style.display = 'flex';
        document.getElementById('user-name').innerText = user.name;
        document.getElementById('user-display-status').style.display = 'block';
        document.getElementById('display-name').innerText = user.name;
        document.getElementById('display-id').innerText = user.id;
        document.getElementById('nav-join-btn').style.display = 'inline-flex';
        document.getElementById('apply').style.display = 'block';
    };

    // 3. Leaderboard (Max Heap Display)
    async function loadLeaderboard() {
        const grid = document.getElementById('student-grid');
        try {
            const res = await fetch('/api/students');
            const students = await res.json();
            grid.innerHTML = '';
            students.sort((a,b) => b.points - a.points).forEach((s, i) => {
                grid.innerHTML += `
                    <div class="student-card">
                        <div class="rank">#${i+1}</div>
                        <div class="student-info">
                            <img src="https://ui-avatars.com/api/?name=${encodeURIComponent(s.name)}&background=random" class="avatar">
                            <h4>${s.name}</h4>
                        </div>
                        <div class="merit-score">${s.points} pts</div>
                    </div>
                `;
            });
        } catch (e) { console.error(e); }
    }

    // 4. Smooth Scrolling
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function (e) {
            e.preventDefault();
            document.querySelector(this.getAttribute('href')).scrollIntoView({ behavior: 'smooth' });
        });
    });

    window.logout = () => {
        localStorage.removeItem('synapse_user');
        location.reload();
    };

    initApp();
});
