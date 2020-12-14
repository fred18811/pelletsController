window.addEventListener("load",loader);
function loader() {

    let mass_setting_controller = document.querySelectorAll('input[type="number"');
    let timerId = null;
    let dataJson ={}
    let host_name = window.location.hostname;
    let gateway = "ws://" + host_name + "/ws";
    let websocket = new WebSocket(gateway);
 
//-------------------Принимаем данные от сервера------------------------------------------------------------
    if(document.title == "Главное меню"){
        websocket.onmessage = function(event){
            for (i of event.data.split(";")){
                let val =  i.split(":");
                if(document.getElementById(val[0]))
                    document.getElementById(val[0]).value = parseFloat(val[1]).toFixed(2);
            }
        };
    }
    else if(document.getElementById("controllersettings")){
        fetch('/getdata', {
            method: 'POST',
            body: "controlsetting=1"
        })
        .then(response => {  //Если проблемы дописать код!!!!!!!
            if (!response.ok) {
                return Promise.reject(new Error(
                    'controlsetting failed: ' + response.status + ' (' + response.statusText + ')'
                ));
            }
            return response.json();
        })
        .then(data =>{
            dataJson = data;
            for (key in data) {
                if(document.getElementById(key)) document.getElementById(key).value = data[key];
            }
        })
        .catch(error => console.error(error));
    }
//------------------------------------------------------------------------------------------------------------
//-------------------Отправляем измененные данные на сервер---------------------------------------------------
    for(i in mass_setting_controller){
        if(mass_setting_controller[i].type == "number"){
            mass_setting_controller[i].addEventListener("input",(e)=>{
                dataJson[e.target.id] = e.target.value;
                if(timerId){
                    clearTimeout(timerId);
                    timerId = null;
                }
                if(e.target.value){
                    let stateBool = true;
                    for (key in dataJson) {
                        if(!dataJson[key]){
                            stateBool = false;
                            break;
                        }
                    }
                    if(stateBool){
                        timerId = setTimeout(sentData, 3000, dataJson);
                    }
                }
            });
        }
    }
//-------------------------------------------------------------------------------------------------------------
    if(document.getElementById("ethsettings") || document.getElementById("main")){
        let request_settings = fetch("config.json",{'Cache-Control': 'no-cache'});
        request_settings
        .then(response=>response.json())
        .then(data=>{
            for (key in data) {
                    if(key == "dhsp"){
                        if(document.getElementById("dhsp")){
                            document.getElementById(key).value = data[key];
                            if(data[key] == "On"){
                                document.getElementById('ip').disabled = true ; 
                                document.getElementById('gw').disabled = true ; 
                                document.getElementById('subnet').disabled = true ; 
                            }
                            else{
                                document.getElementById('ip').disabled = false ; 
                                document.getElementById('gw').disabled = false ; 
                                document.getElementById('subnet').disabled = false ;    
                            }
                        }
                    }
                    else if(key == "mqtton"){
                        if(document.getElementById("mqtton")){
                            document.getElementById(key).value = data[key];
                            if(data[key] == "On"){
                                document.getElementById('ip_mqtt').disabled = true ; 
                                document.getElementById('port_mqtt').disabled = true ; 
                                document.getElementById('id_mqtt').disabled = true ; 
                            }
                            else{
                                document.getElementById('ip_mqtt').disabled = false ; 
                                document.getElementById('port_mqtt').disabled = false ; 
                                document.getElementById('id_mqtt').disabled = false ;    
                            }
                        } 
                    }
                    else if(key == "version_prosh"){
                        if(data[key] && document.getElementById(key)) document.getElementById(key).innerHTML = data[key];
                    }
                    else{
                        if(document.getElementById(key)) document.getElementById(key).value = data[key];
                    }
            }
        })
        .catch(error => console.error(error));
     }
/*
    else if(document.getElementById("pechkasettings")){
        let request_settings = fetch("configcontroller.json",{'Cache-Control': 'no-cache'});
        request_settings
        .then(response=>response.json())
        .then(data=>{
            for (key in data) {
                if(document.getElementById(key)) document.getElementById(key).value = data[key];
            }
        })
        .catch(error => console.error(error));
     }
*/    
    if (document.getElementById('dhsp')) { document.getElementById('dhsp').addEventListener('change', function (e) { checkChecked(e); })}
    if (document.getElementById('mqtton')) { document.getElementById('mqtton').addEventListener('change', function (e) { checkChecked(e);})}
}

function checkChecked(e) {
    if(e.target.id == "dhsp"){
        if(document.getElementById("dhsp").value == "On"){
            document.getElementById('ip').disabled = true ; 
            document.getElementById('gw').disabled = true ; 
            document.getElementById('subnet').disabled = true ; 
        }
        else{
            document.getElementById('ip').disabled = false ; 
            document.getElementById('gw').disabled = false ; 
            document.getElementById('subnet').disabled = false ;    
        }
    }
    else if(e.target.id == "mqtton"){
        if(document.getElementById("mqtton").value == "On"){
            document.getElementById('ip_mqtt').disabled = true ; 
            document.getElementById('port_mqtt').disabled = true ; 
            document.getElementById('id_mqtt').disabled = true ; 
        }
        else{
            document.getElementById('ip_mqtt').disabled = false ; 
            document.getElementById('port_mqtt').disabled = false ; 
            document.getElementById('id_mqtt').disabled = false ;    
        }
    }       
}

function sentData(jsonData){
    let strdata = "";
    if(document.getElementById("controllersettings")){
        strdata = "controlsetting=1&";
    }
    for(i in jsonData){
        strdata += i;
        strdata += "=";
        strdata += parseInt(jsonData[i]);
        strdata += "&";
    }

    fetch('/sentdata', {
        method: 'POST',
        body: strdata
    })
      .catch(error => console.error(error));
}