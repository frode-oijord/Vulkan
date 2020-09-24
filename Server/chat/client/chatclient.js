"use strict";

let SignalingConnection = () => {
  let hostname = window.location.hostname || "localhost";
  let port = hostname == 'localhost' ? ":8080" : "";
  let scheme = document.location.protocol === "https:" ? "wss" : "ws";
  let websocket = new WebSocket(scheme + "://" + hostname + port);

  let onmessage = {};

  let send = (json) => websocket.send(JSON.stringify(json));

  websocket.onopen = (evt) => {
    send({
      type: "username",
      username: document.getElementById("username").value
    });
  };

  websocket.onmessage = (evt) => {
    var message = JSON.parse(evt.data);
    onmessage[message.type](message);
  };

  websocket.onerror = console.log;

  return {
    onmessage, send
  };
};

let connect = () => {
  let videocalls = {};

  let constraints = {
    audio: true,
    video: {
      aspectRatio: {
        ideal: 1.333333
      }
    }
  };

  let localvideo = document.createElement("video");
  localvideo.muted = true;
  localvideo.autoplay = true;
  localvideo.controls = true;
  document.body.appendChild(localvideo);

  navigator.mediaDevices.getUserMedia(constraints).then(stream => {
    localvideo.srcObject = stream;
  }).catch(console.log);

  let signaler = SignalingConnection();

  let call = (event) => {
    // signaler.send({
    //   type: "create-call",
    //   username: event.target.textContent
    // });
    let username = event.target.textContent;
    if (!videocalls[username]) {
      videocalls[username] = VideoCall(username, constraints, signaler);
      videocalls[username].open();
    }
  };

  let appenduser = (username) => {
    let list = document.getElementById("userlist");
    let item = document.createElement("li");
    item.appendChild(document.createTextNode(username));
    item.addEventListener("click", call, false);
    list.appendChild(item);
  };

  signaler.onmessage["userlist"] = (message) => {
    message.users.map(appenduser);
  };

  signaler.onmessage["new-user"] = (message) => {
    appenduser(message.username);
  };

  signaler.onmessage["create-call"] = (message) => {
    if (videocalls[username]) {
      alert("videocall exists, wtf!");
    }
    videocalls[username] = VideoCall(username, constraints, signaler);
    videocalls[username].open();
  };

  signaler.onmessage["rtc-offer"] = (message) => {
    if (!videocalls[message.username]) {
      // alert("videocall not created, wtf?");
      videocalls[message.username] = VideoCall(message.username, constraints, signaler);
      videocalls[message.username].open();
    }
    videocalls[message.username].handleMessage(message);
  };

  signaler.onmessage["rtc-answer"] = (message) => videocalls[message.username].handleMessage(message);
  signaler.onmessage["ice-candidate"] = (message) => videocalls[message.username].handleMessage(message);
};

let VideoCall = (username, constraints, signaler, polite) => {
  let video = document.createElement("video");
  video.autoplay = true;
  video.controls = true;
  document.body.appendChild(video);

  let pc = new RTCPeerConnection({
    iceServers: [{ "urls": "stun:stun.l.google.com:19302" }]
  });

  let self = {};

  self.open = () => {
    navigator.mediaDevices.getUserMedia(constraints).then(stream => {
      stream.getTracks().forEach(track => pc.addTransceiver(track, { streams: [stream] }));
    }).catch(console.log);

    pc.ontrack = ({ track, streams }) => {
      track.onunmute = () => {
        video.srcObject = streams[0];
      };
    };

    pc.removeTrack = (event) => {
      if (video.srcObject.getTracks.length == 0) {
        self.close();
      }
    };

    pc.onicecandidate = (event) => {
      if (event.candidate) {
        signaler.send({
          type: "ice-candidate",
          username: username,
          candidate: event.candidate
        });
      }
    };

    pc.onnegotiationneeded = () => {
      pc.setLocalDescription().then(() => {
        signaler.send({
          type: "rtc-offer",
          username: username,
          sdp: pc.localDescription
        });
      }).catch(console.log);
    };

    pc.oniceconnectionstatechange = (event) => {
      switch (pc.iceConnectionState) {
        case "closed":
        case "failed":
        case "disconnected":
          self.close();
          break;
      }
    };
  };

  self.close = () => {
    pc.ontrack = null;
    pc.removeTrack = null;
    pc.onicecandidate = null;
    pc.onnegotiationneeded = null;
    pc.oniceconnectionstatechange = null;

    if (video.srcObject) {
      video.pause();
      video.srcObject.getTracks().forEach(track => track.stop());
      video.srcObject = null;
    }

    pc.close();
    document.body.removeChild(video);
  };

  self.handleMessage = async (message) => {
    if (message.type == "rtc-offer") {
      pc.setRemoteDescription(new RTCSessionDescription(message.sdp));

      pc.setLocalDescription().then(() => {
        signaler.send({
          type: "rtc-answer",
          username: username,
          sdp: pc.localDescription
        });
      });
    }
    else if (message.type == "rtc-answer") {
      pc.setRemoteDescription(new RTCSessionDescription(message.sdp)).catch(console.log);
    }
    else if (message.type == "ice-candidate") {
      pc.addIceCandidate(new RTCIceCandidate(message.candidate)).catch(console.log);
    }
  };

  return self;
};
