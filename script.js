// script.js
document.addEventListener('DOMContentLoaded', () => {
    // Initialize Socket.io connection for real-time events
    const socket = io();

    // 1. Navbar Scroll Effect
    const navbar = document.getElementById('navbar');
    window.addEventListener('scroll', () => {
        if (window.scrollY > 50) {
            navbar.classList.add('scrolled');
        } else {
            navbar.classList.remove('scrolled');
        }
    });

    // 2. Real-Time Event Listener
    socket.on('new_event', (data) => {
        const container = document.getElementById('live-events-container');
        
        // Remove "waiting" message if it exists
        if (container.querySelector('.fa-spinner')) {
            container.innerHTML = '';
        }
        
        // Create new event element
        const eventEl = document.createElement('div');
        eventEl.className = 'live-event';
        eventEl.style.padding = '15px';
        eventEl.style.marginBottom = '10px';
        eventEl.style.background = 'rgba(13, 138, 188, 0.1)';
        eventEl.style.borderLeft = '4px solid var(--primary-color)';
        eventEl.style.borderRadius = '4px';
        eventEl.style.animation = 'fadeIn 0.5s ease';
        
        const time = new Date().toLocaleTimeString();
        eventEl.innerHTML = `<strong><i class="fa-solid fa-bell"></i> New Event (${time})</strong><br>${data.desc}`;
        
        // Add to top
        container.prepend(eventEl);
    });

    // 3. Fetch Students from C++ Backend API
    async function loadStudents() {
        const grid = document.getElementById('student-grid');
        try {
            const response = await fetch('/api/students');
            const students = await response.json();
            
            if (!students || students.length === 0) {
                grid.innerHTML = '<div style="text-align:center; width:100%;">No students created yet. Register one!</div>';
                return;
            }

            grid.innerHTML = ''; // Clear loading
            
            students.sort((a, b) => b.points - a.points); // Sort by points (Max Heap does this in C++ too)
            
            students.forEach((student, index) => {
                const card = document.createElement('div');
                card.className = 'student-card';
                card.setAttribute('data-skill', student.skill.toLowerCase());
                
                // Assign badges based on rank
                let badgeClass = '';
                if (index === 0) badgeClass = ' badge-gold';
                else if (index === 1) badgeClass = ' badge-silver';
                else if (index === 2) badgeClass = ' badge-bronze';
                
                card.innerHTML = `
                    <div class="rank${badgeClass}">#${index + 1}</div>
                    <div class="student-info">
                        <img src="https://ui-avatars.com/api/?name=${encodeURIComponent(student.name)}&background=random&color=fff" alt="${student.name}" class="avatar">
                        <div class="details">
                            <h4>${student.name}</h4>
                            <p>${student.id}</p>
                        </div>
                    </div>
                    <div class="student-skills">
                        <span class="skill-tag">${student.skill}</span>
                    </div>
                    <div class="merit-score">
                        <div class="score-ring">
                            <span>${student.points}</span>
                        </div>
                        <p>Points</p>
                    </div>
                `;
                grid.appendChild(card);
            });
            
            // Re-attach filter logic
            setupFilters();
            
        } catch (err) {
            console.error(err);
            grid.innerHTML = '<div style="text-align:center; color:#ff4d4d;">Failed to load data from C++ Backend</div>';
        }
    }

    // 4. Fetch Initial Events
    async function loadEvents() {
        try {
            const response = await fetch('/api/events');
            const events = await response.json();
            
            const container = document.getElementById('live-events-container');
            if (events && events.length > 0) {
                container.innerHTML = '';
                events.forEach(ev => {
                    container.innerHTML += `
                        <div class="live-event" style="padding: 15px; margin-bottom: 10px; background: rgba(255,255,255,0.05); border-left: 4px solid var(--text-secondary); border-radius: 4px;">
                            <i class="fa-solid fa-history"></i> ${ev.desc}
                        </div>
                    `;
                });
            }
        } catch (err) {
            console.error(err);
        }
    }

    // 5. Student Filtering Logic
    function setupFilters() {
        const filterBtns = document.querySelectorAll('.filter-btn');
        const studentCards = document.querySelectorAll('.student-card');

        filterBtns.forEach(btn => {
            btn.addEventListener('click', () => {
                // Remove active class
                filterBtns.forEach(b => b.classList.remove('active'));
                btn.classList.add('active');

                const filterValue = btn.getAttribute('data-filter');

                studentCards.forEach(card => {
                    if (filterValue === 'all') {
                        card.style.display = 'flex';
                    } else if (card.getAttribute('data-skill') && card.getAttribute('data-skill').includes(filterValue)) {
                        card.style.display = 'flex';
                    } else {
                        card.style.display = 'none';
                    }
                });
            });
        });
    }

    // 6. Smooth Scrolling for anchor links
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function (e) {
            e.preventDefault();
            const target = document.querySelector(this.getAttribute('href'));
            if (target) {
                target.scrollIntoView({
                    behavior: 'smooth'
                });
            }
        });
    });

    // 7. Load backend data on startup
    loadStudents();
    loadEvents();
});

// Global Test Functions connecting to API
window.testAddStudent = async function() {
    const id = prompt("Enter Student ID (e.g., S101):");
    const name = prompt("Enter Full Name:");
    const email = prompt("Enter Email Address:");
    const skill = prompt("Enter Primary Skill (Web, AI, Design, C++):");
    
    if(id && name) {
        try {
            await fetch('/api/students', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({id, name, email, skill})
            });
            alert('Student Profile Registered in C++ Backend!');
            // Reload page to fetch new generated HTML
            window.location.reload();
        } catch(e) {
            alert('Failed to reach backend.');
        }
    }
}

window.testAddEvent = async function() {
    const desc = prompt("Enter new recruitment or society event (Broadcast to all students):");
    if(desc) {
        try {
            await fetch('/api/events', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({description: desc})
            });
            // The socket.io listener will automatically append the event to the UI without reloading
        } catch(e) {
            alert('Failed to reach backend.');
        }
    }
}
