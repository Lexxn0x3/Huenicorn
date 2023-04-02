log = console.log;


class SetupUI
{
  constructor()
  {
    this.setupSection = document.getElementById("setupSection");
    this.stepNodes = document.getElementsByClassName("step");
    this.errorMessages = document.getElementsByClassName("errorMessages");

    this.setConfigFilePath();

    this.start();
  }


  setConfigFilePath()
  {
    RequestUtils.get("/configFilePath", (jsonData) => {
      let data = JSON.parse(jsonData);
      let configFileLink = document.getElementById("configFilePath");
      configFileLink.href = `file://${data.configFilePath}`;
      configFileLink.innerHTML = data.configFilePath;
    })
  }


  start()
  {
    this.stepId = 0;
    this._loadStep(this.stepId);
  }


  incStep()
  {
    this._loadStep(this.stepId + 1);
  }


  abort()
  {
    this._exit();
  }


  // Bridge address step
  autoDetectBridge()
  {
    let autoDetectBridgePromise = RequestUtils.get("/autoDetectBridge");
    autoDetectBridgePromise.then((data) => {this.autoDetectBridgeCallback(data)});
    autoDetectBridgePromise.catch((error) => {log(error);});
  }


  autoDetectBridgeCallback(data)
  {
    if(data.succeeded){
      document.getElementById("bridgeAddress").value = data.bridgeAddress;
    }
    else{
      let errorMessageNode = document.getElementById("bridgeAddressErrorMessage");
      errorMessageNode.style.visibility = "visible";
      errorMessageNode.innerHTML = data.error;
    }
  }


  validateAddress()
  {
    let bridgeAddress = document.getElementById("bridgeAddress").value;

    if(bridgeAddress != ""){
      let validateBridgeAddressPromise = RequestUtils.put("/validateBridgeAddress", JSON.stringify({bridgeAddress : bridgeAddress}));
      validateBridgeAddressPromise.then((data) => {this.validateBridgeAddressCallback(data);});
      validateBridgeAddressPromise.catch((error) => {log(error);})
    }
    else{
      let errorMessageNode = document.getElementById("bridgeAddressErrorMessage");
      errorMessageNode.style.visibility = "visible";
      errorMessageNode.innerHTML = "Please provide a valid address";
    }
  }


  validateBridgeAddressCallback(data)
  {
    if(data.succeeded){
      this.incStep();
    }
    else{
      let errorMessageNode = document.getElementById("bridgeAddressErrorMessage");
      errorMessageNode.style.visibility = "visible";
      errorMessageNode.innerHTML = "Could not resolve Hue bridge";
    }
  }


  // Credentials validation step
  validateCredentials()
  {
    const username = document.getElementById("username").value;
    const clientkey = document.getElementById("clientkey").value;
    let errorMessageNode = document.getElementById("credentialsErrorMessage");

    if(username == "" || clientkey == ""){
      errorMessageNode.style.visibility = "visible";
      errorMessageNode.innerHTML = "Please provide valid credentials (or quickly generate new ones)";
      return;
    }

    const regex = new RegExp(/^[A-F0-9]{32}$/);
    if(!regex.test(clientkey)){
      errorMessageNode.style.visibility = "visible";
      errorMessageNode.innerHTML = "The clientkey doesn't match the format of 32 hexadecimal characters";
      return;
    }

    let validateCredentialsPromise = RequestUtils.put("/validateCredentials", JSON.stringify({username : username, clientkey : clientkey}));
    validateCredentialsPromise.then((data) => {this.validateCredentialsCallback(data);});
    validateCredentialsPromise.catch((error) => {log(error);});
  }


  validateCredentialsCallback(data)
  {

    if(data.succeeded){
      this._finish();
    }
    else{
      let errorMessageNode = document.getElementById("credentialsErrorMessage");
      errorMessageNode.style.visibility = "visible";
      errorMessageNode.innerHTML = "The provided username was denied";
    }
  }


  // Credentials generation step
  registerNewUser()
  {
    let registerNewUserPromise = RequestUtils.put("/registerNewUser", JSON.stringify(null));
    registerNewUserPromise.then((data) => {this.registerNewUserCallback(data);});
    registerNewUserPromise.catch((error) => {log(error);});
  }


  registerNewUserCallback(data)
  {
    if(data.succeeded){
      this._finish();
    }
    else{
      let errorMessageNode = document.getElementById("newUserErrorMessage");
      errorMessageNode.style.visibility = "visible";
      errorMessageNode.innerHTML = "Please press the central button on the bridge and try again";
    }
  }


  // Private methods
  _loadStep(stepId)
  {
    this.stepId = stepId;
    let i = 0;
    for(let stepNode of this.stepNodes){
      let display = (i++ == this.stepId) ? "block" : "none";
      stepNode.style.display = display;
    }

    for(let errorMessageNode of this.errorMessages){
      errorMessageNode.style.visibility = "hidden";
      errorMessageNode.innerHTML = "";
    }
  }

  _finish()
  {
    let finishSetupPromise = RequestUtils.post("/finishSetup", JSON.stringify(null));
    finishSetupPromise.then((data) => {log("Finished");});
    finishSetupPromise.catch((error) => {log(error);})

    for(let stepNode of this.stepNodes){
      stepNode.style.display = "none";
    }

    document.getElementById("finishSection").style.display = "block";

    this.pingFunctionInterval = setInterval(
      this._ping
      , 500);
  }


  _exit()
  {
    let abortPromise = RequestUtils.post("/abort", JSON.stringify(null));
    abortPromise.then((data) => {});
    abortPromise.catch((error) => {log(error);});

    for(let stepNode of this.stepNodes){
      stepNode.style.display = "none";
    }

    document.getElementById("abortSection").style.display = "block";
  }


  _ping()
  {
    let webUIStatusPromise = RequestUtils.get("/webUIStatus");
    webUIStatusPromise.then((data) => {
      document.getElementById("refreshSection").style.display = "block";
      clearInterval(setupUI.pingFunctionInterval);
    });
    webUIStatusPromise.catch((error) => {log(error);});
  }
}

let setupUI = new SetupUI();
