const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<html>
  <head>
    <title>Home Security System</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
    <link rel="icon" href="data:,">
    <style>
      html {font-family: Arial; display: inline-block; text-align: center;}
      p {font-size: 1.2rem;}
      body {margin: 0;}
      .topnav {overflow: hidden; background-color: #6C0BA9; color: white; font-size: 1.5rem;}
      .content {padding: 20px; }
      .card {background-color: white; box-shadow: 0px 0px 10px 1px rgba(140,140,140,.5); border: 1px solid #6C0BA9; border-radius: 15px;}
      .card.header {background-color: #6C0BA9; color: white; border-bottom-right-radius: 0px; border-bottom-left-radius: 0px; border-top-right-radius: 12px; border-top-left-radius: 12px;}
      .cards {max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));}
      .reading {font-size: 2.8rem;}
      .packet {color: #bebebe;}
      .camColor {color: #4d6a40;}
      .motionColor {color: #fd7e14;}
      .sirenColor {color: #1b78e2;}
      .sensorstateColor {color: #183153; font-size:15px;}
    </style>
  </head>
  
  <body>
    <div class="topnav">
      <h3>Home Security System</h3>
    </div>
    
    <br>
    
    <div class="content">
      <div class="cards">
         
        <div class="card">
          <div class="card header">
            <h2>Front Door</h2>
          </div>
          <br>
          <h4 class="camColor"><i class="fas fa-camera"></i> CAPTURE</h4>
          <img  id="imgsrc1" width="300px" heigth="450px" /> 
          <h4 class="motionColor"><i class="fas fa-american-sign-language-interpreting"></i> MOTION</h4>
          <p class="motionColor"><span class="reading"><span id="motion1"></span> </span></p>
          <h4 class="sirenColor"><i class="fas fa-bell"></i> SIREN</h4>
          <p class="sirenColor"><span class="reading"><span id="siren1"></span> </span></p>
          <p class="sensorstateColor"><span>Status of Sensor : </span><span id="status_read_1"></span></p>
          <p class="sensorstateColor"><span>Last time to receive data : </span><span id="LTRD_ESP32_1"></span></p>
        </div>
        
        <div class="card">
          <div class="card header">
            <h2>Real Door</h2>
          </div>
          <br>
           
          <h4 class="camColor"><i class="fas fa-camera"></i> CAPTURE</h4>
          <img  id="imgsrc2" width="300px" heigth="450px" />
          <h4 class="motionColor"><i class="fas fa-american-sign-language-interpreting"></i> MOTION</h4>
          <p class="motionColor"><span class="reading"><span id="motion2"></span> </span></p>
          <h4 class="sirenColor"><i class="fas fa-bell"></i> SIREN</h4>
          <p class="sirenColor"><span class="reading"><span id="siren2"></span> </span></p>
          <p class="sensorstateColor"><span>Status of Sensor : </span><span id="status_read_2"></span></p>
          <p class="sensorstateColor"><span>Last time to receive data : </span><span id="LTRD_ESP32_2"></span></p>
        </div>  
        
      </div>
    </div>
    
    <script>
      //------------------------------------------------------------
      document.getElementById("motion1").innerHTML = "NN"; 
      document.getElementById("imgsrc1").src = "https://www.guardanthealthamea.com/wp-content/uploads/2019/09/no-image.jpg"; 
      document.getElementById("siren1").innerHTML = "NN";
      document.getElementById("status_read_1").innerHTML = "NN";
      document.getElementById("LTRD_ESP32_1").innerHTML = "NN";
      document.getElementById("motion2").innerHTML = "NN"; 
      document.getElementById("imgsrc2").src = "https://www.guardanthealthamea.com/wp-content/uploads/2019/09/no-image.jpg";
      document.getElementById("siren2").innerHTML = "NN";
      document.getElementById("status_read_2").innerHTML = "NN";
      document.getElementById("LTRD_ESP32_2").innerHTML = "NN";
      //------------------------------------------------------------

      //------------------------------------------------------------
      if (!!window.EventSource) {
        var source = new EventSource('/events');
        
        source.addEventListener('open', function(e) {
          console.log("Events Connected");
        }, false);
        
        source.addEventListener('error', function(e) {
          if (e.target.readyState != EventSource.OPEN) {
            console.log("Events Disconnected");
          }
        }, false);
        
        source.addEventListener('message', function(e) {
          console.log("message", e.data);
        }, false);

        source.addEventListener('allDataJSON', function(e) {
          console.log("allDataJSON", e.data);

          var today = new Date();
          var time = leading_zeros(today.getHours()) + ":" + leading_zeros(today.getMinutes()) + ":" + leading_zeros(today.getSeconds());
          
          var obj = JSON.parse(e.data);
          
          if (obj.idESP32Sender == "#1") {
            document.getElementById("motion1").innerHTML = obj.motion;
            document.getElementById("siren1").innerHTML = obj.siren;
            document.getElementById("imgsrc1").src = obj.imgsrcc;
            document.getElementById("status_read_1").innerHTML = obj.sensorstatus;
            if (obj.StatusReadDHT11 == "FAILED") {
              document.getElementById("status_read_1").style.color = "red";
            } else {
              document.getElementById("status_read_1").style.color = "#183153";
            }
            document.getElementById("LTRD_ESP32_1").innerHTML = time;
          }
          if (obj.idESP32Sender == "#2") {
            document.getElementById("motion2").innerHTML = obj.motion;
            document.getElementById("siren2").innerHTML = obj.siren;
            document.getElementById("imgsrc1").src = obj.imgsrcc;
            document.getElementById("status_read_2").innerHTML = obj.sensorstatus;
            if (obj.StatusReadDHT11 == "FAILED") {
              document.getElementById("status_read_2").style.color = "red";
            } else {
              document.getElementById("status_read_2").style.color = "#183153";
            }
            document.getElementById("LTRD_ESP32_2").innerHTML = time;
          }
        }, false);
      }
      //------------------------------------------------------------

      function leading_zeros(x) { 
        return (x < 10 ? '0' : '') + x;
      }
    </script>
  </body>
</html>
)=====";
