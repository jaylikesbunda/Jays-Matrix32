#ifndef MATRIX_UI_H
#define MATRIX_UI_H

const char MATRIX_UI_HTML[] = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <!-- Mobile-friendly viewport -->
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Jay's Matrix32</title>
    <style>
        /* Global reset and modern styles */
        * { box-sizing: border-box; }
        body {
            font-family: "Helvetica Neue", sans-serif;
            margin: 0;
            padding: 0;
            /* Modern black and gray gradient */
            background: linear-gradient(135deg, #000000, #434343);
            color: #fff;
        }
        .container {
            max-width: 900px;
            width: 90%;
            margin: 40px auto;
            background: #252525;
            padding: 20px;
            border-radius: 12px;
            box-shadow: 0 4px 20px rgba(0,0,0,0.5);
            text-align: center;
        }
        h1 {
            margin-top: 0;
            text-align: center;
            font-weight: bold;
            font-size: 28px;
            text-transform: uppercase;
            letter-spacing: 2px;
            border-bottom: 1px solid #fff;
            padding-bottom: 10px;
            margin-bottom: 20px;
        }
        .color-picker, .mode-selector {
            margin-bottom: 15px;
        }
        .color-picker input[type="color"] {
            width: 60px;
            height: 60px;
            border: none;
            background: none;
            cursor: pointer;
            /* Use a subtle border for better visibility */
            border-radius: 50%;
            border: 2px solid #fff;
            padding: 0;
        }
        /* Secondary color picker (hidden by default) */
        #secondary-color-controls {
            margin-bottom: 15px;
            display: none;
            text-align: center;
        }
        #secondary-color-label {
            display: block;
            margin-bottom: 5px;
        }
        /* Mode selector – force centering and custom appearance */
        .mode-selector {
            position: relative;
            width: 200px;
            margin: 0 auto 15px;
        }
        .mode-selector select {
            width: 100%;
            padding: 8px 40px 8px 8px; /* extra padding on the right for the arrow */
            font-size: 16px;
            border: 2px solid #fff;
            border-radius: 4px;
            background: linear-gradient(135deg, #333, #111);
            background-color: #222; /* fallback color */
            color: #fff;
            cursor: pointer;
            -webkit-appearance: none;
            -moz-appearance: none;
            appearance: none;
        }
        /* Custom arrow for the dropdown */
        .mode-selector::after {
            content: '▼';
            position: absolute;
            right: 10px;
            top: 50%;
            transform: translateY(-50%);
            pointer-events: none;
            color: #fff;
            font-size: 14px;
        }
        .controls, .tools {
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 10px;
            margin: 15px 0;
            flex-wrap: wrap;
        }
        .controls input[type="range"] {
            width: 200px;
        }
        .btn {
            background: transparent;
            border: 2px solid #fff;
            border-radius: 4px;
            padding: 10px 20px;
            color: #fff;
            cursor: pointer;
            transition: background 0.2s ease, color 0.2s ease;
            margin: 5px;
        }
        .btn:hover {
            background: #fff;
            color: #000;
        }
        .grid {
            display: grid;
            grid-template-columns: repeat(8, 1fr);
            gap: 4px;
            margin: 20px auto;
            max-width: 300px;
        }
        .pixel {
            width: 100%;
            padding-top: 100%; /* square cells */
            position: relative;
            background: #000;
            border-radius: 4px;
            /* Prevent default touch actions (e.g. scrolling) */
            touch-action: none;
        }
        .pixel::after {
            content: "";
            display: block;
            position: absolute;
            top: 0; left: 0; right: 0; bottom: 0;
        }
        #debug {
            font-family: monospace;
            margin-top: 20px;
            padding: 10px;
            background: #333;
            border-radius: 4px;
            text-align: left;
        }
        /* Responsive adjustments for small screens */
        @media (max-width: 480px) {
            h1 { font-size: 24px; }
            .btn { padding: 8px 16px; font-size: 14px; }
            .color-picker input[type="color"] { width: 50px; height: 50px; }
            .grid { max-width: 250px; }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Jay's Matrix32</h1>
        <div class="color-picker">
            <input type="color" id="color-picker" value="#ff0000">
        </div>
        <div id="secondary-color-controls" class="color-picker">
            <label id="secondary-color-label" for="secondary-color-picker"></label>
            <input type="color" id="secondary-color-picker" value="#0000ff">
        </div>
        <div class="mode-selector">
            <select id="mode-select">
                <option value="static">Static</option>
                <option value="rainbow">Rainbow</option>
                <option value="checkerboard">Checkerboard</option>
                <option value="gradient">Gradient</option>
                <option value="random">Random</option>
            </select>
        </div>
        <div class="controls">
            <span>Brightness:</span>
            <!-- Default value 12 ≈5% brightness -->
            <input type="range" id="brightness" min="0" max="255" value="12">
            <span id="brightness-value">5%</span>
        </div>
        <div class="tools">
            <button class="btn" id="clear">Clear All</button>
            <button class="btn" id="fill">Fill All</button>
            <input type="file" id="image-upload" accept="image/*" hidden>
            <button class="btn" id="upload-btn">Upload Image</button>
        </div>
        <div class="grid" id="matrix"></div>
        <div id="debug"></div>
        <div id="mode-warning" style="color: red; display: none;">Switch to Static Mode to draw!</div>
    </div>

    <script>
        // API call helper.
        function logApiCall(endpoint, data) {
            console.log("Sending to", endpoint, data); // DEBUG LINE
            if (window.location.hostname !== '192.168.4.1') {
                document.getElementById('debug').innerHTML = `API Call to ${endpoint}:<br>${JSON.stringify(data, null, 2)}`;
                return Promise.resolve({ status: 'ok' });
            }
            return fetch(endpoint, {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(data)
            });
        }
        
        // Helper: Convert hex to rgb object.
        function hexToRgb(hex) {
            const shorthandRegex = /^#?([a-f\d])([a-f\d])([a-f\d])$/i;
            hex = hex.replace(shorthandRegex, (m, r, g, b) => r + r + g + g + b + b);
            const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
            return result ? {
                r: parseInt(result[1], 16),
                g: parseInt(result[2], 16),
                b: parseInt(result[3], 16)
            } : null;
        }
        
        // Helper: HSV to RGB conversion.
        function hsvToRgb(h, s, v) {
            let c = v * s;
            let x = c * (1 - Math.abs((h / 60) % 2 - 1));
            let m = v - c;
            let r = 0, g = 0, b = 0;
            if (h < 60) { r = c; g = x; b = 0; }
            else if (h < 120) { r = x; g = c; b = 0; }
            else if (h < 180) { r = 0; g = c; b = x; }
            else if (h < 240) { r = 0; g = x; b = c; }
            else if (h < 300) { r = x; g = 0; b = c; }
            else { r = c; g = 0; b = x; }
            return {
                r: Math.round((r + m) * 255),
                g: Math.round((g + m) * 255),
                b: Math.round((b + m) * 255)
            };
        }
        
        // Global variables for preview.
        let currentColor = { r: 255, g: 0, b: 0 },
            secondaryColor = { r: 0, g: 0, b: 255 },
            selectedMode = "static",
            animateInterval = null;
        
        // Function to update the grid preview based on selected mode.
        function updatePreview(mode) {
            // Clear any existing animation interval.
            if (animateInterval !== null) {
                clearInterval(animateInterval);
                animateInterval = null;
            }
            const cells = document.querySelectorAll('.pixel');
            
            if (mode === "rainbow") {
                let rainbowAngle = 0;
                animateInterval = setInterval(() => {
                    cells.forEach((cell, index) => {
                        let hue = (rainbowAngle + index * (360 / 64)) % 360;
                        const rgb = hsvToRgb(hue, 1, 1);
                        cell.style.backgroundColor = `rgb(${rgb.r}, ${rgb.g}, ${rgb.b})`;
                    });
                    rainbowAngle = (rainbowAngle + 5) % 360;
                }, 100);
            } else if (mode === "checkerboard") {
                cells.forEach((cell, index) => {
                    let row = Math.floor(index / 8), col = index % 8;
                    if ((row + col) % 2 === 0) {
                        cell.style.backgroundColor = `rgb(${currentColor.r}, ${currentColor.g}, ${currentColor.b})`;
                    } else {
                        cell.style.backgroundColor = `rgb(${secondaryColor.r}, ${secondaryColor.g}, ${secondaryColor.b})`;
                    }
                });
            } else if (mode === "gradient") {
                let gradientOffset = 0;
                animateInterval = setInterval(() => {
                    cells.forEach((cell, index) => {
                        let col = index % 8;
                        // Apply an offset to animate the gradient movement.
                        let effectiveCol = (col + gradientOffset) % 8;
                        let factor = effectiveCol / 7; // Interpolation factor between the two colors.
                        let r = Math.round(currentColor.r * (1 - factor) + secondaryColor.r * factor);
                        let g = Math.round(currentColor.g * (1 - factor) + secondaryColor.g * factor);
                        let b = Math.round(currentColor.b * (1 - factor) + secondaryColor.b * factor);
                        cell.style.backgroundColor = `rgb(${r}, ${g}, ${b})`;
                    });
                    gradientOffset = (gradientOffset + 1) % 8;
                }, 200);
            } else if (mode === "random") {
                cells.forEach(cell => {
                    let r = Math.floor(Math.random() * 256),
                        g = Math.floor(Math.random() * 256),
                        b = Math.floor(Math.random() * 256);
                    cell.style.backgroundColor = `rgb(${r}, ${g}, ${b})`;
                });
            }
        }
        
        // BRIGHTNESS DEBOUNCE
        let brightnessTimeout;
        const brightness = document.getElementById('brightness'),
              brightnessValue = document.getElementById('brightness-value');
        brightness.addEventListener('input', e => {
            clearTimeout(brightnessTimeout);
            brightnessTimeout = setTimeout(() => {
                const value = parseInt(e.target.value);
                fetch('/brightness', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({brightness: value})
                }).catch(err => console.error('Brightness error:', err));
            }, 200); // 200ms debounce
        });
        
        // Event: Color picker updates primary color.
        const colorInput = document.getElementById('color-picker');
        colorInput.addEventListener('input', (event) => {
            const rgb = hexToRgb(event.target.value);
            if (rgb) {
                currentColor = rgb;
                // Only update preview for non-static modes
                if (selectedMode !== "static") {
                    updatePreview(selectedMode);
                }
            }
        });
        
        // Event: Secondary color picker updates secondary color.
        const secondaryInput = document.getElementById('secondary-color-picker');
        secondaryInput.addEventListener('input', (event) => {
            const rgb = hexToRgb(event.target.value);
            if (rgb) {
                secondaryColor = rgb;
                if (selectedMode === "checkerboard" || selectedMode === "gradient")
                    updatePreview(selectedMode);
            }
        });
        
        // Show/hide secondary color control based on selected mode.
        function updateSecondaryColorControls(mode) {
            const secondaryControls = document.getElementById('secondary-color-controls');
            const secondaryLabel = document.getElementById('secondary-color-label');
            if (mode === "checkerboard" || mode === "gradient") {
                secondaryControls.style.display = "block";
                secondaryLabel.textContent = (mode === "gradient") ? "Gradient Secondary Color:" : "Checkerboard Secondary Color:";
            } else {
                secondaryControls.style.display = "none";
            }
        }
        
        // Event: Mode selector change.
        const modeSelect = document.getElementById('mode-select');
        modeSelect.addEventListener('change', e => {
            const mode = e.target.value;
            selectedMode = mode;
            updateSecondaryColorControls(mode);
            updatePreview(selectedMode);
            logApiCall('/mode', { mode }).then(() => {
                document.getElementById('debug').innerHTML = `Mode set to: ${mode}`;
            });
        });
        
        // Build 8x8 grid of clickable pixels.
        const matrix = document.getElementById('matrix');
        let lastColor = null;

        function updatePixelUI(pixelElement, r, g, b) {
            pixelElement.style.backgroundColor = `rgb(${r},${g},${b})`;
        }

        for (let i = 0; i < 64; i++) {
            const pixel = document.createElement('div');
            pixel.className = 'pixel';
            pixel.dataset.index = i;
            pixel.id = `pixel-${Math.floor(i/8)}-${i%8}`; // Coordinate-based ID
            matrix.appendChild(pixel);
        }
        
        // Initialize the preview.
        updatePreview(selectedMode);
        
        // MODIFIED pointer handlers with drag support
        let isDrawing = false;
        let lastPixelIndex = null;
        let currentAction = null; // 'paint' or 'erase'

        matrix.addEventListener('pointerdown', e => {
            if(selectedMode !== "static") {
                modeSelect.value = "static"; // AUTO-SWITCH MODE
                modeSelect.dispatchEvent(new Event('change'));
            }
            const pixel = e.target.closest('.pixel');
            if (!pixel) return;
            
            isDrawing = true;
            currentAction = (pixel.style.backgroundColor === 'rgb(0, 0, 0)') ? 'paint' : 'erase';
            processPixel(pixel, currentAction);
        });

        matrix.addEventListener('pointermove', e => {
            if (!isDrawing) return;
            e.preventDefault();
            
            const pixel = document.elementFromPoint(e.clientX, e.clientY);
            if (pixel && pixel.classList.contains('pixel')) {
                const index = pixel.dataset.index;
                if (index !== lastPixelIndex) {
                    processPixel(pixel, currentAction);
                    lastPixelIndex = index;
                }
            }
        });

        matrix.addEventListener('pointerup', () => {
            isDrawing = false;
            lastPixelIndex = null;
            sendPendingUpdates(); // Flush any remaining
        });

        // NEW: Batch pixel updates
        let pendingUpdates = [];
        let sendTimeout = null;

        function processPixel(pixel, action) {
            if (selectedMode !== "static") return; // Already handled
            
            const newColor = action === 'paint' ? currentColor : {r:0,g:0,b:0};
            const currentColorStr = `rgb(${newColor.r},${newColor.g},${newColor.b})`;
            
            // Skip if already in target state
            if (pixel.style.backgroundColor === currentColorStr) return;
            
            // Immediate UI update
            pixel.style.backgroundColor = currentColorStr;
            
            // Queue for API
            const index = parseInt(pixel.dataset.index);
            pendingUpdates.push({
                row: Math.floor(index / 8),
                col: index % 8,
                r: newColor.g,
                g: newColor.r,
                b: newColor.b
            });
            
            // FORCE send after 3px drawn (was 10)
            if(pendingUpdates.length >= 3) sendPendingUpdates();
        }

        function sendPendingUpdates() {
            if (pendingUpdates.length === 0) return;
            
            const updates = [...pendingUpdates];
            pendingUpdates = [];
            
            fetch('/pixel', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({updates})
            }).catch(err => {
                console.error('Batch update failed:', err);
                // Rollback UI for failed updates
                updates.forEach(update => {
                    const pixel = document.getElementById(`pixel-${update.row}-${update.col}`);
                    if (pixel) pixel.style.backgroundColor = lastColor;
                });
            });
        }
        
        // Bulk operations.
        document.getElementById('clear').addEventListener('click', () => {
            const pixels = document.querySelectorAll('.pixel');
            const updates = [];
            pixels.forEach((p, i) => {
                p.style.backgroundColor = '#000';
                updates.push({
                    row: Math.floor(i / 8),
                    col: i % 8,
                    r: 0, g: 0, b: 0
                });
            });
            logApiCall('/pixel', { updates });  // Use logApiCall with updates array
        });
        document.getElementById('fill').addEventListener('click', () => {
            const pixels = document.querySelectorAll('.pixel');
            const updates = [];
            pixels.forEach((p, i) => {
                p.style.backgroundColor = `rgb(${currentColor.r}, ${currentColor.g}, ${currentColor.b})`;
                updates.push({
                    row: Math.floor(i / 8),
                    col: i % 8,
                    r: currentColor.r,  // Send green as red (GRB order)
                    g: currentColor.g,  // Send red as green
                    b: currentColor.b   // Blue stays the same
                });
            });
            logApiCall('/pixel', { updates });  // Use logApiCall with updates array
        });
        
        // Image upload & downscale using Canvas.
        document.getElementById('upload-btn').addEventListener('click', () => {
            document.getElementById('image-upload').click();
        });
        document.getElementById('image-upload').addEventListener('change', function(e) {
            const file = e.target.files[0];
            if (!file) return;
            const reader = new FileReader();
            reader.onload = function(event) {
                const img = new Image();
                img.onload = function() {
                    const canvas = document.createElement('canvas');
                    canvas.width = 8;
                    canvas.height = 8;
                    const ctx = canvas.getContext('2d');
                    ctx.drawImage(img, 0, 0, 8, 8);
                    const imageData = ctx.getImageData(0, 0, 8, 8).data;
                    const pixels = document.querySelectorAll('.pixel');
                    for (let i = 0; i < 64; i++) {
                        const idx = i * 4;
                        const r = imageData[idx],
                              g = imageData[idx + 1],
                              b = imageData[idx + 2];
                        pixels[i].style.backgroundColor = `rgb(${r}, ${g}, ${b})`;
                        logApiCall('/pixel', {
                            row: Math.floor(i / 8),
                            col: i % 8,
                            r, g, b
                        });
                    }
                };
                img.src = event.target.result;
            };
            reader.readAsDataURL(file);
        });

        // NEW: Event listener on the secondary color picker.
        document.getElementById('secondary-color-picker').addEventListener('input', function(e) {
            secondaryColor = hexToRgb(e.target.value);
            updatePreview(selectedMode);
            // Send the secondary color to the firmware via the /secondarycolor endpoint.
            fetch('/secondarycolor', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(secondaryColor)
            })
            .then(response => response.json())
            .then(data => console.log('Updated secondary color in firmware:', data))
            .catch(err => console.error('Error updating secondary color:', err));
        });

        // NEW: Real-time pixel sync
        setInterval(() => { // Only sync non-static modes
                fetch('/pixels').then(r => r.json()).then(pixels => {
                    pixels.forEach(pixel => {
                        const cell = document.getElementById(`pixel-${pixel.row}-${pixel.col}`);
                        cell.style.backgroundColor = `rgb(${pixel.r},${pixel.g},${pixel.b})`;
                    });
                });
        }, 1000); // Update every 100ms

        // FIX 1: Make mode warning visible
        function showModeWarning() {
            const warning = document.getElementById('mode-warning');
            warning.textContent = "Switch to Static Mode to draw!";
            warning.style.display = 'block';
            setTimeout(() => warning.style.display = 'none', 3000);
        }
    </script>
</body>
</html> 
)=====";

#endif // MATRIX_UI_H 