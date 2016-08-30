let log bla =>
  ignore @@ Js.Unsafe.(meth_call (variable "console") "log" [|inject (Js.string bla)|]);

let unwrap s e =>
  switch e {
  | None => log s
  | Some err => log @@ ("Error occured '" ^ s ^ "': " ^ err)
  };

let configureDataChannel dataChannel => {
  dataChannel#setOnError (fun err => log "there was an error somewhere");
  dataChannel#setOnMessage (
    fun s => {
      log @@ ("there was a message somehwere " ^ s#data);
      let open Js.Unsafe;
      let chatarea =
        meth_call (variable "document") "getElementById" [|inject (Js.string "chatarea")|];
      set
        chatarea
        "innerHTML"
        (
          inject (
            Js.string (
              Js.to_string (get chatarea "innerHTML") ^
                "<div><span style='color:red'>Other</span>: " ^ s#data ^ "</div>"
            )
          )
        )
    }
  );
  dataChannel#setOnOpen (fun () => log "onopen");
  dataChannel#setOnClose (fun () => log "onclose")
};

let dataChannel = ref None;

let setupConnection (connection: Webrtc.RTCPeerConnection.t) socket => {
  connection#setOnIceCandidate (
    fun e => {
      log "setOnIceCandidate";
      socket#emit "ice-candidate" e#candidate#raw
    }
  );
  connection#setOnIceConnectionStateChange (fun e => log "setOnIceConnectionStateChange");
  connection#setOnDataChannel (
    fun dc => {
      log "setOnDataChannel";
      configureDataChannel dc;
      dc#send "Connected!";
      dataChannel := Some dc
    }
  )
};

let createDataChannel (connection: Webrtc.RTCPeerConnection.t) dataChannelName => {
  let dataChannel =
    connection#createDataChannel
      dataChannelName Webrtc.RTCDataChannel.{ordered: true, maxRetransmitTime: 3000.0};
  configureDataChannel dataChannel;
  dataChannel
};

let makeConnection socket => {
  let connection = new Webrtc.RTCPeerConnection.t;
  setupConnection connection socket;
  socket#on
    "cancel-connection"
    (
      fun e => {
        log "closing...";
        connection#close ()
      }
    );
  socket#on
    "answer"
    (
      fun desc => {
        log "got answer";
        connection#setRemoteDescription
          ((new Webrtc.RTCSessionDescription.t) desc)
          (unwrap "setRemoteDescription everything went well")
      }
    );
  socket#on
    "ice-candidate"
    (
      fun candidate => {
        log "got ice-candidate";
        connection#addIceCandidate
          ((new Webrtc.RTCIceCandidate.t) candidate) (unwrap "addIceCandidate went well")
      }
    );
  dataChannel := Some (createDataChannel connection "somenamehere");
  connection
};

let socket = new ReSocketIO.Socket.t;

let connection = makeConnection socket;

connection#createOffer
  Webrtc.RTCPeerConnection.{offerToReceiveAudio: 0.0, offerToReceiveVideo: 0.0}
  (
    fun desc => {
      log "createOffer success!";
      connection#setLocalDescription
        desc
        (
          fun maybeError => {
            log "setLocalDescription wow";
            socket#emit "offer" desc#raw
          }
        )
    }
  )
  (fun err => log @@ ("createOffer failure: " ^ err));

socket#on
  "offer"
  (
    fun desc => {
      log "got offer";
      let connection = makeConnection socket;
      connection#setRemoteDescription
        ((new Webrtc.RTCSessionDescription.t) desc)
        (unwrap "setRemoteDescription everything probably went well");
      connection#createAnswer
        (
          fun desc => {
            log "createAnswer went well";
            connection#setLocalDescription desc (unwrap "setLocalDescription went well");
            socket#emit "answer" desc#raw
          }
        )
        (fun err => log @@ ("blaaaa " ^ err))
    }
  );

{
  let open Js.Unsafe;
  let chatbox = meth_call (variable "document") "getElementById" [|inject (Js.string "chatbox")|];
  let sendButton =
    meth_call (variable "document") "getElementById" [|inject (Js.string "send-button")|];
  let chatarea = meth_call (variable "document") "getElementById" [|inject (Js.string "chatarea")|];
  ignore @@
    meth_call
      sendButton
      "addEventListener"
      [|
        inject (Js.string "click"),
        inject (
          Js.wrap_callback (
            fun () => {
              set
                chatarea
                "innerHTML"
                (
                  inject (
                    Js.string (
                      Js.to_string (get chatarea "innerHTML") ^
                        "<div><span>You</span>: " ^ Js.to_string (get chatbox "value") ^ "</div>"
                    )
                  )
                );
              switch !dataChannel {
              | None => log "not connected yet"
              | Some chan => chan#send (Js.to_string (get chatbox "value"))
              };
              set chatbox "value" (Js.string "")
            }
          )
        )
      |];
  ignore @@
    meth_call
      chatbox
      "addEventListener"
      [|
        inject (Js.string "keyup"),
        inject (
          Js.wrap_callback (
            fun e =>
              switch (int_of_float (Js.to_float (get e "which"))) {
              | 13 /* Enter key */ =>
                set
                  chatarea
                  "innerHTML"
                  (
                    inject (
                      Js.string (
                        Js.to_string (get chatarea "innerHTML") ^
                          "<div><span>You</span>: " ^ Js.to_string (get chatbox "value") ^ "</div>"
                      )
                    )
                  );
                switch !dataChannel {
                | None => log "not connected yet"
                | Some chan => chan#send (Js.to_string (get chatbox "value"))
                };
                set chatbox "value" (Js.string "")
              | _ => ()
              }
          )
        )
      |]
};
