window.addEventListener("load",loader);
function loader() {
    if(document.title == "Главное меню"){
        let data ={}
        let host_name = window.location.hostname;
        var gateway = "ws://" + host_name + "/ws";
        let websocket = new WebSocket(gateway);
        websocket.onmessage = function(event){
            for (i of event.data.split(";")){
                let val =  i.split(":");
                if(document.getElementById(val[0]))
                    document.getElementById(val[0]).value = parseFloat(val[1]).toFixed(2);
            }
        };
    }

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
    else if(document.getElementById("pechkasettings")){
        let request_settings = fetch("configpechka.json",{'Cache-Control': 'no-cache'});
        request_settings
        .then(response=>response.json())
        .then(data=>{
            for (key in data) {
                if(document.getElementById(key)) document.getElementById(key).value = data[key];
            }
        })
        .catch(error => console.error(error));
     }
    
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