const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const { execFile } = require('child_process');
const path = require('path');

const app = express();
const server = http.createServer(app);
const io = new Server(server);

// Middleware
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Serve static frontend files from current directory
app.use(express.static(__dirname));

// Ensure main.exe exists based on OS (Windows: main.exe, Linux/Mac: main)
const cppExecutable = process.platform === 'win32' ? 'main.exe' : './main';

// Helper to run C++ program
function runCppBackend(args) {
    return new Promise((resolve, reject) => {
        execFile(cppExecutable, args, { cwd: __dirname }, (error, stdout, stderr) => {
            if (error) {
                console.error(`Error executing C++: ${stderr || error.message}`);
                return reject({ error: 'Internal Server Error', details: stderr || error.message });
            }
            try {
                // Parse the JSON output from the C++ program
                const jsonResponse = JSON.parse(stdout.trim());
                resolve(jsonResponse);
            } catch (err) {
                console.error('Failed to parse C++ output as JSON:', stdout);
                reject({ error: 'Failed to parse backend response', raw: stdout });
            }
        });
    });
}

// ---------------------------------------------------------
// API ROUTES (Calling C++ Backend)
// ---------------------------------------------------------

// Get all students
app.get('/api/students', async (req, res) => {
    try {
        const data = await runCppBackend(['--get-students']);
        res.json(data);
    } catch (err) {
        res.status(500).json(err);
    }
});

// Add a new student
app.post('/api/students', async (req, res) => {
    const { id, name, email, skill } = req.body;
    if (!id || !name || !email || !skill) {
        return res.status(400).json({ error: 'Missing student fields' });
    }
    
    try {
        const result = await runCppBackend(['--add-student', id, name, email, skill]);
        res.json(result);
    } catch (err) {
        res.status(500).json(err);
    }
});

// Get all recent events
app.get('/api/events', async (req, res) => {
    try {
        const data = await runCppBackend(['--get-events']);
        res.json(data);
    } catch (err) {
        res.status(500).json(err);
    }
});

// Add a new society event (AND BROADCAST VIA WEBSOCKETS)
app.post('/api/events', async (req, res) => {
    const { description } = req.body;
    if (!description) {
        return res.status(400).json({ error: 'Missing event description' });
    }

    try {
        const result = await runCppBackend(['--add-event', description]);
        
        // ** REAL-TIME FEATURE **
        // Broadcast the new event to all connected students
        if (result.status === 'success') {
            io.emit('new_event', { desc: description });
        }
        
        res.json(result);
    } catch (err) {
        res.status(500).json(err);
    }
});

// ---------------------------------------------------------
// WEBSOCKETS (Real-Time Connections)
// ---------------------------------------------------------
io.on('connection', (socket) => {
    console.log(`Student connected: ${socket.id}`);
    
    socket.on('disconnect', () => {
        console.log(`Student disconnected: ${socket.id}`);
    });
});

// Start Server
const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
    console.log(`========================================`);
    console.log(`🚀 Synapse Web Server is running!`);
    console.log(`🌐 Open http://localhost:${PORT} in your browser`);
    console.log(`========================================`);
});
