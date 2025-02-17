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
        /* Global Reset */
        *, *::before, *::after {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        /* Base Styles */
        body {
            font-family: "Helvetica Neue", sans-serif;
            background-color: #121212;
            color: #e0e0e0;
            line-height: 1.5;
        }

        .container {
            max-width: 900px;
            width: 95%;
            margin: 40px auto;
            background-color: #1e1e1e;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.3);
            text-align: center;
        }

        h1 {
            font-size: 1.75rem;
            text-transform: uppercase;
            letter-spacing: 2px;
            border-bottom: 1px solid #444;
            padding-bottom: 10px;
            margin-bottom: 20px;
        }

        /* Color Picker & Mode Selector */
        .color-picker, .mode-selector {
            margin-bottom: 15px;
        }

        .color-picker input[type="color"] {
            width: 50px;
            height: 50px;
            border: 2px solid #444;
            border-radius: 50%;
            background: none;
            cursor: pointer;
        }

        /* Secondary Color Picker */
        #secondary-color-controls {
            margin-bottom: 15px;
            display: none;
            text-align: center;
        }

        #secondary-color-label {
            display: block;
            margin-bottom: 5px;
            font-size: 0.9rem;
        }

        /* Mode Selector */
        .mode-selector {
            position: relative;
            width: 200px;
            margin: 0 auto 15px;
        }

        .mode-selector select {
            width: 100%;
            padding: 10px;
            font-size: 1rem;
            border: 1px solid #444;
            border-radius: 4px;
            background-color: #2a2a2a;
            color: #e0e0e0;
            cursor: pointer;
            -webkit-appearance: none;
            -moz-appearance: none;
            appearance: none;
        }

        .mode-selector::after {
            content: '▼';
            position: absolute;
            right: 10px;
            top: 50%;
            transform: translateY(-50%);
            pointer-events: none;
            color: #e0e0e0;
            font-size: 0.8rem;
        }

        /* Controls & Tools */
        .controls, .tools {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            align-items: center;
            gap: 10px;
            margin: 15px 0;
        }

        .controls input[type="range"] {
            width: 200px;
        }

        .btn {
            background: transparent;
            border: 1px solid #444;
            border-radius: 4px;
            padding: 10px 20px;
            color: #e0e0e0;
            cursor: pointer;
            transition: background 0.2s ease, color 0.2s ease;
            margin: 5px;
        }

        .btn:hover {
            background: #e0e0e0;
            color: #121212;
        }

        /* Grid & Pixel Styles */
        .grid {
            display: grid;
            grid-template-columns: repeat(8, 1fr);
            gap: 4px;
            margin: 20px auto;
            max-width: 300px;
        }

        .pixel {
            width: 100%;
            padding-top: 100%; /* maintain square shape */
            position: relative;
            background-color: #000;
            border-radius: 4px;
            touch-action: none;
        }

        .pixel::after {
            content: "";
            display: block;
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
        }

        /* Debug Console */
        #debug {
            font-family: monospace;
            margin-top: 20px;
            padding: 10px;
            background-color: #1a1a1a;
            border-radius: 4px;
            text-align: left;
        }

        /* Responsive Adjustments */
        @media (max-width: 480px) {
            h1 { font-size: 1.5rem; }
            .btn { padding: 8px 16px; font-size: 0.9rem; }
            .color-picker input[type="color"] { width: 40px; height: 40px; }
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
                r: Math.round((g + m) * 255),
                g: Math.round((r + m) * 255),
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
                        let effectiveCol = (col + gradientOffset) % 8;
                        let factor = effectiveCol / 7;
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
            }, 200);
        });
        
        // Event: Primary color picker update.
        const colorInput = document.getElementById('color-picker');
        colorInput.addEventListener('input', (event) => {
            const rgb = hexToRgb(event.target.value);
            if (rgb) {
                currentColor = rgb;
                if (selectedMode !== "static") {
                    updatePreview(selectedMode);
                }
            }
        });
        
        // Event: Secondary color picker update.
        const secondaryInput = document.getElementById('secondary-color-picker');
        secondaryInput.addEventListener('input', (event) => {
            const rgb = hexToRgb(event.target.value);
            if (rgb) {
                secondaryColor = rgb;
                if (selectedMode === "checkerboard" || selectedMode === "gradient")
                    updatePreview(selectedMode);
            }
        });
        
        // Show/hide secondary controls based on mode.
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
        
        // Mode selector change event.
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
        
        // Build 8x8 grid for pixels.
        const matrix = document.getElementById('matrix');
        let lastColor = null;

        function updatePixelUI(pixelElement, r, g, b) {
            pixelElement.style.backgroundColor = `rgb(${r},${g},${b})`;
        }

        for (let i = 0; i < 64; i++) {
            const pixel = document.createElement('div');
            pixel.className = 'pixel';
            pixel.dataset.index = i;
            pixel.id = `pixel-${Math.floor(i/8)}-${i%8}`;
            matrix.appendChild(pixel);
        }
        
        updatePreview(selectedMode);
        
        // Pointer event handlers with drag support.
        let isDrawing = false;
        let lastPixelIndex = null;
        let currentAction = null;

        matrix.addEventListener('pointerdown', e => {
            if(selectedMode !== "static") {
                modeSelect.value = "static";
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
            sendPendingUpdates();
        });

        // Batch pixel updates.
        let pendingUpdates = [];
        let sendTimeout = null;

        function processPixel(pixel, action) {
            if (selectedMode !== "static") return;
            const newColor = action === 'paint' ? currentColor : {r:0,g:0,b:0};
            const currentColorStr = `rgb(${newColor.r},${newColor.g},${newColor.b})`;
            if (pixel.style.backgroundColor === currentColorStr) return;
            pixel.style.backgroundColor = currentColorStr;
            
            const index = parseInt(pixel.dataset.index);
            pendingUpdates.push({
                row: Math.floor(index / 8),
                col: index % 8,
                r: newColor.r,
                g: newColor.g,
                b: newColor.b
            });
            
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
                updates.forEach(update => {
                    const pixel = document.getElementById(`pixel-${update.row}-${update.col}`);
                    if (pixel) pixel.style.backgroundColor = lastColor;
                });
            });
        }
        
        // Bulk operations.
        document.getElementById('clear').addEventListener('click', () => {
            const pixels = document.querySelectorAll('.pixel');
            pixels.forEach(p => p.style.backgroundColor = '#000');
            fetch('/pixel', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({ 
                    fill: 'yes',
                    r: 0,
                    g: 0,
                    b: 0
                })
            }).catch(err => console.error('Clear failed:', err));
        });

        document.getElementById('fill').addEventListener('click', () => {
            const pixels = document.querySelectorAll('.pixel');
            pixels.forEach(p => p.style.backgroundColor = `rgb(${currentColor.r}, ${currentColor.g}, ${currentColor.b})`);
            fetch('/pixel', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({ 
                    fill: 'yes',
                    r: currentColor.r,
                    g: currentColor.g,
                    b: currentColor.b
                })
            }).catch(err => console.error('Fill failed:', err));
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

        // Secondary color API update.
        document.getElementById('secondary-color-picker').addEventListener('input', function(e) {
            secondaryColor = hexToRgb(e.target.value);
            updatePreview(selectedMode);
            fetch('/secondarycolor', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(secondaryColor)
            })
            .then(response => response.json())
            .then(data => console.log('Updated secondary color in firmware:', data))
            .catch(err => console.error('Error updating secondary color:', err));
        });

        // Real-time pixel sync.
        setInterval(() => {
            fetch('/pixels')
                .then(r => r.json())
                .then(pixels => {
                    pixels.forEach(pixel => {
                        const cell = document.getElementById(`pixel-${pixel.row}-${pixel.col}`);
                        if(cell)
                            cell.style.backgroundColor = `rgb(${pixel.r},${pixel.g},${pixel.b})`;
                    });
                });
        }, 1000);

        // Mode warning visibility.
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