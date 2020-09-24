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
  
  