"use strict";

var connection = null;
var videocall = null;

function sendToServer(msg) {
  var msgJSON = JSON.stringify(msg);
  connection.send(msgJSON);
}


function openVideoCall(event) {
  if (videocall) {
    alert("You can't start a call because you already have one open!");
  } else {
    var targetUsername = event.target.textContent;
    var myUsername = document.getElementById("name").value;

    if (targetUsername != myUsername) {
      videocall = VideoCall(myUsername, targetUsername);
    }
  }
}


function closeVideoCall() {
  if (videocall) {
    videocall.destroy();
    videocall = null;
  }
}

function handleUserlistMsg(msg) {
  var listElem = document.querySelector(".userlistbox");

  while (listElem.firstChild) {
    listElem.removeChild(listElem.firstChild);
  }

  msg.users.forEach(username => {
    var item = document.createElement("li");
    item.appendChild(document.createTextNode(username));
    item.addEventListener("click", openVideoCall, false);
    listElem.appendChild(item);
  });
}


function handleSendButton() {
  var input = document.getElementById("text");

  sendToServer({
    text: input.value,
    type: "message",
  });

  input.value = "";
}


function handleKey(evt) {
  if (evt.keyCode === 13 || evt.keyCode === 14) {
    if (!document.getElementById("send").disabled) {
      handleSendButton();
    }
  }
}


function updateChatBox(name, text) {
  var chatbox = document.querySelector(".chatbox");
  chatbox.innerHTML += "<b>" + name + "</b>: " + text + "<br>";
  chatbox.scrollTop = chatbox.scrollHeight - chatbox.clientHeight;
}


function updateChatBoxMsg(msg) {
  updateChatBox(msg.name, msg.text);
}


function connect() {
  var myHostname = window.location.hostname || "localhost";
  var scheme = (document.location.protocol === "https:") ? "wss" : "ws";
  let serverUrl = scheme + "://" + myHostname + ":8080";

  connection = new WebSocket(serverUrl);

  connection.onopen = (evt) => {
    document.getElementById("text").disabled = false;
    document.getElementById("send").disabled = false;

    connection.send(JSON.stringify({
      type: "username",
      name: document.getElementById("name").value
    }));
  };

  connection.onerror = console.log;

  connection.onmessage = (evt) => {
    var msg = JSON.parse(evt.data);
    console.log(msg);

    switch (msg.type) {
      case "message":
        updateChatBoxMsg(msg);
        break;

      case "userlist":
        handleUserlistMsg(msg);
        break;

      case "video-offer":
        if (!videocall) {
          var myUsername = document.getElementById("name").value;
          videocall = VideoCall(myUsername, msg.name);
        }
        videocall.handleVideoOffer(msg.sdp);
        break;

      case "video-answer":
        videocall.handleVideoAnswer(msg.sdp);
        break;

      case "ice-candidate":
        videocall.handleICECandidate(msg.candidate);
        break;

      case "hang-up":
        videocall.destroy();
        videocall = null;
        break;

      default:
        console.log("Unknown message received:" + msg);
    }
  };
}


function VideoCall(myUsername, targetUsername) {
  var hangupbutton = document.getElementById("hangup-button");
  hangupbutton.onclick = closeVideoCall;

  var pc = new RTCPeerConnection({
    iceServers: [{ "urls": "stun:stun.l.google.com:19302" }]
  });

  pc.onicecandidate = (event) => {
    if (event.candidate) {
      sendToServer({
        type: "ice-candidate",
        target: targetUsername,
        candidate: event.candidate
      });
    }
  };

  pc.oniceconnectionstatechange = (event) => {
    updateChatBox("ICE connection state: ", pc.iceConnectionState);
    switch (pc.iceConnectionState) {
      case "closed":
      case "failed":
      case "disconnected":
        destroy();
        break;
    }
  };

  pc.onicegatheringstatechange = (event) => {
    updateChatBox("ICE gathering state: ", pc.iceGatheringState);
  };

  pc.onsignalingstatechange = (event) => {
    updateChatBox("signaling state: ", pc.iceConnectionState);
    switch (pc.signalingState) {
      case "closed":
        destroy();
        break;
    }
  };

  pc.onnegotiationneeded = () => {
    pc.createOffer().then(offer => {
      return pc.setLocalDescription(offer);
    }).then(() => {
      sendToServer({
        name: myUsername,
        target: targetUsername,
        type: "video-offer",
        sdp: pc.localDescription
      });
    }).catch(console.log);
  };

  pc.ontrack = (event) => {
    console.log("ontrack event");
    document.getElementById("received_video").srcObject = event.streams[0];
    document.getElementById("hangup-button").disabled = false;
  };


  var mediaConstraints = {
    audio: true,
    video: {
      aspectRatio: {
        ideal: 1.333333
      }
    }
  };

  navigator.mediaDevices.getUserMedia(mediaConstraints).then(webcamStream => {
    document.getElementById("local_video").srcObject = webcamStream;
    webcamStream.getTracks().forEach(track => pc.addTransceiver(track, { streams: [webcamStream] }));
  }).catch(console.log);


  return {

    destroy: () => {
      var localVideo = document.getElementById("local_video");
      var remoteVideo = document.getElementById("received_video");

      pc.ontrack = null;
      pc.onnicecandidate = null;
      pc.oniceconnectionstatechange = null;
      pc.onsignalingstatechange = null;
      pc.onicegatheringstatechange = null;
      pc.onnotificationneeded = null;

      if (remoteVideo.srcObject) {
        remoteVideo.pause();
        remoteVideo.srcObject.getTracks().forEach(track => {
          track.stop();
        });
        remoteVideo.srcObject = null;
      }

      if (localVideo.srcObject) {
        localVideo.pause();
        localVideo.srcObject.getTracks().forEach(track => {
          track.stop();
        });
        localVideo.srcObject = null;
      }

      pc.close();
      pc = null;

      document.getElementById("hangup-button").disabled = true;
      targetUsername = null;
    },

    handleVideoOffer: (sdp) => {
      pc.setRemoteDescription(new RTCSessionDescription(sdp));

      if (pc.signalingState != "stable") {
        return pc.setLocalDescription({ type: "rollback" });
      }

      pc.createAnswer().then(answer => {
        pc.setLocalDescription(answer).then(() => {
          sendToServer({
            name: myUsername,
            target: targetUsername,
            type: "video-answer",
            sdp: pc.localDescription
          });
        });
      });
    },

    handleVideoAnswer: (sdp) => {
      pc.setRemoteDescription(new RTCSessionDescription(sdp)).catch(console.log);
    },

    handleICECandidate: (candidate) => {
      pc.addIceCandidate(new RTCIceCandidate(candidate)).catch(console.log);
    }
  };
}
