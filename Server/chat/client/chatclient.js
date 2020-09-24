"use strict";

var connection = null;
var videocall = null;
var datachannel = null;

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


function openDataChannel(event) {
  if (datachannel) {
    alert("You can't start a channel you already have one open!");
  } else {
    var targetUsername = event.target.textContent;
    var myUsername = document.getElementById("name").value;

    if (targetUsername != myUsername) {
      datachannel = DataChannel(myUsername, targetUsername);
      datachannel.createRTCOffer();
    }
  }
}


function closeDataChannel() {
  if (datachannel) {
    datachannel.destroy();
    datachannel = null;
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

  datachannel.send({
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

      case "rtc-offer":
        if (!videocall) {
          var myUsername = document.getElementById("name").value;
          videocall = VideoCall(myUsername, msg.name);
        }
        videocall.handleRTCOffer(msg.sdp);
        break;

      case "rtc-answer":
        videocall.handleRTCAnswer(msg.sdp);
        break;

      case "ice-candidate":
        videocall.handleICECandidate(msg.candidate);
        break;

      default:
        console.log("Unknown message received:" + msg);
    }
  };
}


function DataChannel(myUsername, targetUsername) {
  var pc = new RTCPeerConnection({
    iceServers: [{ "urls": "stun:stun.l.google.com:19302" }]
  });

  var channel = pc.createDataChannel("chat", { negotiated: true, id: 0 });

  channel.onopen = () => {
    updateChatBox("Data channel is open", "");
  };

  channel.onmessage = function (event) {
    var message = JSON.parse(event.data);
    updateChatBox("got message on datachannel", message.text);
  }

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
        closeVideoCall();
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

  // pc.onnegotiationneeded = () => {
  //   updateChatBox("creating offer");
  //   pc.createOffer().then(offer => {
  //     return pc.setLocalDescription(offer);
  //   }).then(() => {
  //     sendToServer({
  //       name: myUsername,
  //       target: targetUsername,
  //       type: "rtc-offer",
  //       sdp: pc.localDescription
  //     });
  //   }).catch(console.log);
  // };

  return {

    destroy: () => {
      pc.onnicecandidate = null;
      pc.oniceconnectionstatechange = null;
      pc.onsignalingstatechange = null;
      pc.onicegatheringstatechange = null;
      pc.onnotificationneeded = null;

      pc.close();
    },

    send: (msg) => {
      channel.send(JSON.stringify(msg));
    },

    createRTCOffer: () => {
      pc.createOffer().then(offer => {
        return pc.setLocalDescription(offer);
      }).then(() => {
        sendToServer({
          name: myUsername,
          target: targetUsername,
          type: "rtc-offer",
          sdp: pc.localDescription
        });
      }).catch(console.log);
    },

    handleRTCOffer: (sdp) => {
      updateChatBox("handleRTCOffer");
      pc.setRemoteDescription(new RTCSessionDescription(sdp));

      if (pc.signalingState != "stable") {
        return pc.setLocalDescription({ type: "rollback" });
      }

      pc.createAnswer().then(answer => {
        pc.setLocalDescription(answer).then(() => {
          sendToServer({
            name: myUsername,
            target: targetUsername,
            type: "rtc-answer",
            sdp: pc.localDescription
          });
        });
      });
    },

    handleRTCAnswer: (sdp) => {
      updateChatBox("handleRTCAnswer");
      pc.setRemoteDescription(new RTCSessionDescription(sdp)).catch(console.log);
    },

    handleICECandidate: (candidate) => {
      updateChatBox("handleICECandidate");
      pc.addIceCandidate(new RTCIceCandidate(candidate)).catch(console.log);
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
        closeVideoCall();
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
    updateChatBox("creating offer");
    pc.createOffer().then(offer => {
      return pc.setLocalDescription(offer);
    }).then(() => {
      sendToServer({
        name: myUsername,
        target: targetUsername,
        type: "rtc-offer",
        sdp: pc.localDescription
      });
    }).catch(console.log);
  };

  pc.ontrack = (event) => {
    updateChatBox("ontrack event");
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
      document.getElementById("hangup-button").disabled = true;
    },

    createRTCOffer: () => {
      pc.createOffer().then(offer => {
        return pc.setLocalDescription(offer);
      }).then(() => {
        sendToServer({
          name: myUsername,
          target: targetUsername,
          type: "rtc-offer",
          sdp: pc.localDescription
        });
      }).catch(console.log);
    },

    handleRTCOffer: (sdp) => {
      updateChatBox("handleRTCOffer");
      pc.setRemoteDescription(new RTCSessionDescription(sdp));

      if (pc.signalingState != "stable") {
        return pc.setLocalDescription({ type: "rollback" });
      }

      pc.createAnswer().then(answer => {
        pc.setLocalDescription(answer).then(() => {
          sendToServer({
            name: myUsername,
            target: targetUsername,
            type: "rtc-answer",
            sdp: pc.localDescription
          });
        });
      });
    },

    handleRTCAnswer: (sdp) => {
      updateChatBox("handleRTCAnswer");
      pc.setRemoteDescription(new RTCSessionDescription(sdp)).catch(console.log);
    },

    handleICECandidate: (candidate) => {
      updateChatBox("handleICECandidate");
      pc.addIceCandidate(new RTCIceCandidate(candidate)).catch(console.log);
    }
  };
}
