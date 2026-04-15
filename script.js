// script.js
document.addEventListener('DOMContentLoaded', () => {
    // Initialize Socket.io connection (Disabled for C++ compatibility)
    // const socket = io();

    // 1. Initial State & Session Handling
    const savedUser = localStorage.getItem('synapse_user');
    if (savedUser) {
        showUser(JSON.parse(savedUser));
    }

    // Refresh everything on load
    window.loadStudents();
    window.loadEvents();

    // President Session check
    const savedPres = localStorage.getItem('synapse_president_club');
    if (savedPres) {
        showPresidentDashboard(savedPres);
    }

    // 2. Real-Time Event Listener (Disabled)
    /*
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
        
        container.prepend(eventEl);
    });
    */

    // 3. Fetch Students from C++ Backend API
    window.loadStudents = async function() {
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
    window.loadEvents = async function() {
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
    // 8. Mobile Menu Toggle
    const mobileMenu = document.getElementById('mobile-menu');
    const navLinks = document.querySelector('.nav-links');
    const navAuth = document.querySelector('.nav-auth');

    if (mobileMenu) {
        mobileMenu.addEventListener('click', () => {
            const isFlex = navLinks.style.display === 'flex';
            
            navLinks.style.display = isFlex ? 'none' : 'flex';
            navAuth.style.display = isFlex ? 'none' : 'flex';
            
            if (!isFlex) {
                navLinks.style.flexDirection = 'column';
                navLinks.style.position = 'absolute';
                navLinks.style.top = '100%';
                navLinks.style.left = '0';
                navLinks.style.width = '100%';
                navLinks.style.background = 'var(--bg-card)';
                navLinks.style.padding = '1rem';
                
                navAuth.style.flexDirection = 'column';
                navAuth.style.position = 'absolute';
                navAuth.style.top = 'calc(100% + 150px)';
                navAuth.style.left = '0';
                navAuth.style.width = '100%';
                navAuth.style.background = 'var(--bg-card)';
                navAuth.style.padding = '1rem';
            } else {
                navLinks.style = '';
                navAuth.style = '';
            }
        });
    }

    // 9. Society Details Modal Logic
    const societyData = {
        'Coding Club': {
            iconClass: 'tech',
            iconHTML: '<i class="fa-solid fa-code"></i>',
            members: '142 Members',
            president: 'Somraj Lodhi',
            faculty: 'Prof. Ankit Sharma',
            rank: '#1 in Engineering',
            focus: 'Web, Apps, Algorithms',
            events: ['Intro to React Workshop (Mar 20)', 'CodeStorm Hackathon (Apr 5)']
        },
        'Robotics Society': {
            iconClass: 'robotics',
            iconHTML: '<i class="fa-solid fa-robot"></i>',
            members: '85 Members',
            president: 'Aryan Gupta',
            faculty: 'Dr. Neha Verma',
            rank: '#3 in Engineering',
            focus: 'IoT, Drones, Automation',
            events: ['Line Follower Bot Race (Mar 22)', 'Arduino Basics (Mar 28)']
        },
        'Debate Society': {
            iconClass: 'debate',
            iconHTML: '<i class="fa-solid fa-microphone"></i>',
            members: '110 Members',
            president: 'Priya Patel',
            faculty: 'Dr. Ramesh Kumar',
            rank: '#1 in Arts',
            focus: 'Public Speaking, MUNs',
            events: ['National Mock Parliament (Mar 25)', 'Weekly debate rounds (Fridays)']
        },
        'Design & UI/UX': {
            iconClass: 'design',
            iconHTML: '<i class="fa-solid fa-pen-nib"></i>',
            members: '95 Members',
            president: 'Alex Chen',
            faculty: 'Prof. Manish Jain',
            rank: '#2 in Media',
            focus: 'Figma, User Research',
            events: ['UI Design Sprint (Mar 24)', 'Portfolio Review Day (Apr 2)']
        }
    };

    // Load custom clubs from local storage (to simulate a persistent database for presentation)
    const storedClubs = JSON.parse(localStorage.getItem('custom_clubs') || '[]');
    const societyGrid = document.querySelector('.society-grid');
    
    storedClubs.forEach(customClub => {
        // Add to data dictionary
        societyData[customClub.name] = customClub.data;
        
        // Generate and Inject the Card HTML into the grid
        const newCard = document.createElement('div');
        newCard.className = 'society-card';
        newCard.style.cursor = 'pointer';
        newCard.innerHTML = `
            <div class="society-icon ${customClub.data.iconClass}">
                ${customClub.data.iconHTML}
            </div>
            <h3>${customClub.name}</h3>
            <p>${customClub.data.focus}</p>
            <div class="society-footer">
                <span class="members"><i class="fa-solid fa-users"></i> ${customClub.data.members}</span>
                <a href="#" class="view-link" onclick="event.preventDefault()">View &rarr;</a>
            </div>
        `;
        // Insert right after the built-in cards
        if (societyGrid) {
            societyGrid.appendChild(newCard);
        }
    });

    const modal = document.getElementById('society-modal');
    const closeBtn = document.getElementById('modal-close-btn');

    // Attach click listener to all society cards
    document.querySelectorAll('.society-card').forEach(card => {
        card.style.cursor = 'pointer'; // Make the whole card clickable visually
        card.addEventListener('click', (e) => {
            e.preventDefault();
            
            // Get club name from the h3 inside the clicked card
            const clubName = card.querySelector('h3').innerText;
            const data = societyData[clubName];
            
            if(data) {
                // Populate modal
                document.getElementById('modal-club-name').innerText = clubName;
                document.getElementById('modal-members').innerHTML = `<i class="fa-solid fa-users"></i> ${data.members}`;
                document.getElementById('modal-president').innerText = data.president;
                document.getElementById('modal-faculty').innerText = data.faculty;
                document.getElementById('modal-rank').innerText = data.rank;
                document.getElementById('modal-focus').innerText = data.focus;
                
                const iconContainer = document.getElementById('modal-icon');
                iconContainer.className = `society-icon ${data.iconClass}`;
                iconContainer.innerHTML = data.iconHTML;
                
                const eventsList = document.getElementById('modal-events-list');
                eventsList.innerHTML = '';
                data.events.forEach(ev => {
                    eventsList.innerHTML += `<li><i class="fa-solid fa-calendar-check"></i> ${ev}</li>`;
                });
                
                // Show modal
                modal.classList.add('active');
            }
        });
    });

    // Close Modal when X is clicked
    closeBtn.addEventListener('click', () => {
        modal.classList.remove('active');
    });

    // 10. Role Display & Sessions
    window.onLoginSuccess = function(user) {
        localStorage.setItem('synapse_user', JSON.stringify(user));
        showUser(user);
    };

    function showUser(user) {
        const profile = document.getElementById('user-display-status');
        if (profile) {
            profile.style.display = 'block';
            document.getElementById('display-name').innerText = user.name;
            document.getElementById('display-id').innerText = user.id;
            document.getElementById('display-avatar').src = `https://ui-avatars.com/api/?name=${user.name.replace(' ', '+')}&background=random`;
            
            // Re-fetch student to get status
            fetch(`/api/student/${user.id}`).then(res => res.json()).then(data => {
                if(data.status === "success") {
                    const statusText = document.getElementById('display-status');
                    statusText.innerText = data.membershipStatus === "none" ? "(NOT-MEMBER)" : 
                                         data.membershipStatus === "pending" ? `(PENDING: ${data.appliedClub})` :
                                         `(MEMBER: ${data.appliedClub})`;
                    statusText.style.color = data.membershipStatus === "accepted" ? "#00e676" : "#3b82f6";
                }
            });
        }
        // Hide the initial role modal if it's still there
        const roleModal = document.getElementById('role-modal');
        if (roleModal) {
            roleModal.style.opacity = '0';
            setTimeout(() => roleModal.style.display = 'none', 300);
        }
    }

    window.loginWithId = async function() {
        const id = document.getElementById('login-id').value;
        const error = document.getElementById('login-error');
        if (!id) return;

        try {
            const response = await fetch(`/api/student/${id}`);
            if (response.ok) {
                const user = await response.json();
                window.onLoginSuccess({ id: user.id, name: user.name, points: user.points });
            } else {
                error.style.display = 'block';
                error.innerText = "Student ID not found.";
            }
        } catch (err) {
            error.style.display = 'block';
            error.innerText = "Connection error. Is the server running?";
        }
    };

    window.logout = function() {
        localStorage.removeItem('synapse_user');
        window.location.reload();
    };

    // Check for existing session
    const savedUser = localStorage.getItem('synapse_user');
    if (savedUser) {
        showUser(JSON.parse(savedUser));
    }

    // Close Modal when clicking outside the content box
    modal.addEventListener('click', (e) => {
        if (e.target === modal) {
            modal.classList.remove('active');
        }
    });

});
