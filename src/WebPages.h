#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h>

// The Arduino UNO R4 core does not define FPSTR, which is common on other platforms
// for printing PROGMEM strings. We define it here for compatibility.
#ifndef FPSTR
#define FPSTR(p) (reinterpret_cast<const __FlashStringHelper *>(p))
#endif

// Using F() macro and PROGMEM to store strings in Flash memory, saving RAM.

// --- Login Page ---
const char LOGIN_PAGE_HEADER[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Solar Control Panel - Login</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { font-family: Arial, sans-serif; background: #e9f5ff; margin: 0; padding: 20px; }
        .login-container { max-width: 400px; margin: 50px auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { text-align: center; color: #007BFF; }
        .form-group { margin-bottom: 20px; }
        label { display: block; margin-bottom: 5px; font-weight: bold; }
        input[type="text"], input[type="password"] { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }
        .btn { width: 100%; padding: 12px; background: #007BFF; color: white; border: none; border-radius: 5px; font-size: 16px; cursor: pointer; }
        .btn:hover { background: #0056b3; }
        .btn.active { box-shadow: 0 0 12px #28a745; border: 2px solid #28a745; }
        .error { color: #dc3545; text-align: center; margin-top: 10px; }
    </style>
</head>
<body>
    <div class="login-container">
        <h1>[SECURE] Solar Control Panel</h1>
)rawliteral";

const char LOGIN_PAGE_ERROR[] PROGMEM = R"rawliteral(
        <div class="error">Invalid username or password. Please try again.</div>
)rawliteral";

const char LOGIN_PAGE_FORM[] PROGMEM = R"rawliteral(
        <form method="POST" action="/login">
            <div class="form-group">
                <label for="username">Username:</label>
                <input type="text" id="username" name="username" required>
            </div>
            <div class="form-group">
                <label for="password">Password:</label>
                <input type="password" id="password" name="password" required>
            </div>
            <button type="submit" class="btn">Login</button>
        </form>
    </div>
</body>
</html>
)rawliteral";


// --- Main Page ---
const char MAIN_PAGE_HEADER_START[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<title>Solar Control Panel</title>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
body{font-family:Arial,sans-serif;background:#f9f9f9;margin:0;}
.container{max-width:900px;margin:30px auto;background:#fff;border-radius:8px;box-shadow:0 2px 8px #ccc;padding:24px;display:flex;flex-wrap:wrap;gap:18px;}
.card{background:#f5f5f5;border-radius:6px;margin-bottom:18px;padding:18px;box-shadow:0 1px 4px #eee;flex:1 1 320px;min-width:280px;max-width:100%;}
@media(max-width:600px){.container{padding:8px;gap:8px;}.card{min-width:100%;padding:10px;}h1,h2,h4{font-size:1.1em;}}
.btn{padding:8px 18px;margin:4px;border:none;border-radius:4px;background:#007bff;color:#fff;cursor:pointer;font-size:1em;transition:background .2s;}
.btn.off{background:#dc3545;}
.btn.up{background:#28a745;}
.btn.down{background:#ffc107;color:#333;}
.btn.stop{background:#6c757d;}
.btn:active{box-shadow:0 2px 6px #aaa;}
.btn.active { box-shadow: 0 0 12px #28a745; border: 2px solid #28a745; }
.status{font-weight:bold;padding:2px 8px;border-radius:3px;}
.status.ok{color:#28a745;}
.status.err{color:#dc3545;}
.top-bar{display:flex;justify-content:space-between;align-items:center;background:#007bff;color:#fff;padding:10px 18px;border-radius:8px 8px 0 0;}
.top-bar-time,.top-bar-status{margin-right:18px;}
.rain{color:#17a2b8;}
.modal{display:none;position:fixed;z-index:10;left:0;top:0;width:100%;height:100%;overflow:auto;background:rgba(0,0,0,0.3);}
.modal-content{background:#fff;margin:10% auto;padding:24px;border-radius:8px;width:80%;max-width:500px;}
.close-button{float:right;font-size:1.5em;cursor:pointer;}
</style>
)rawliteral";

const char MAIN_PAGE_SCRIPT[] PROGMEM = R"rawliteral(
<script>
function showStatus(msg, isError) {
    var bar = document.getElementById('statusBar');
    if (!bar) {
        bar = document.createElement('div');
        bar.id = 'statusBar';
        bar.style.position = 'fixed';
        bar.style.top = '0';
        bar.style.left = '0';
        bar.style.width = '100%';
        bar.style.zIndex = '9999';
        bar.style.padding = '12px';
        bar.style.textAlign = 'center';
        bar.style.fontWeight = 'bold';
        bar.style.background = isError ? '#dc3545' : '#28a745';
        bar.style.color = '#fff';
        document.body.appendChild(bar);
    }
    bar.textContent = msg;
    bar.style.background = isError ? '#dc3545' : '#28a745';
    bar.style.display = 'block';
    setTimeout(function(){ bar.style.display='none'; }, 2000);
}

function sendCommand(u){
    document.body.style.cursor='wait';
    showStatus('Sending command...', false);
    fetch(u,{method:'POST',credentials:'same-origin'})
    .then(r=>{
        document.body.style.cursor='default';
        if(r.ok) {
            showStatus('Command successful!', false);
            setTimeout(function(){ window.location.reload(); }, 1200);
        } else {
            if(r.status==401) {
                showStatus('Authentication failed. Reloading...', true);
                setTimeout(function(){ window.location.reload(); }, 1200);
            } else {
                showStatus('Command failed. Try again.', true);
            }
        }
    })
    .catch(e=>{
        document.body.style.cursor='default';
        showStatus('Could not send command. Check connection.', true);
    });
}
document.addEventListener('DOMContentLoaded',function(){
    var m=document.getElementById('infoModal'),b=document.getElementById('infoBtn'),s=document.getElementsByClassName('close-button')[0];
    if(b)b.onclick=function(){m.style.display='block';};
    if(s)s.onclick=function(){m.style.display='none';};
    window.onclick=function(e){if(e.target==m)m.style.display='none';};
});
</script>
)rawliteral";

const char MAIN_PAGE_HEADER_END[] PROGMEM = R"rawliteral(
</head>
<body>
)rawliteral";

const char MAIN_PAGE_FOOTER[] PROGMEM = R"rawliteral(
    <div id="statusBar" style="display:none;"></div>
    <div style="text-align:center; margin-top:24px; color:#888;">
        <small>Secure Solar Automation System</small>
    </div>
    <!-- The System Info Modal is placed here, just before the closing body tag -->
    <div id='infoModal' class='modal'>
      <div class='modal-content'>
        <span class='close-button'>&times;</span>
        <!-- Modal content is generated by the server -->
      </div>
    </div>
</body>
</html>
)rawliteral";

const char CARD_START[] PROGMEM = "<div class='card'>";
const char CARD_END[]   PROGMEM = "</div>";
const char GRID_START[] PROGMEM = R"rawliteral(<div class="grid">)rawliteral";
const char GRID_END[] PROGMEM = R"rawliteral(</div>)rawliteral";


#endif // WEB_PAGES_H