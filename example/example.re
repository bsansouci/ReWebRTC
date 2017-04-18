open Webrtc;

open ReasonJs;

/* dom bindings */
external addClickListener : _ [@bs.as "click"] => (Dom.MouseEvent.t => unit) => unit =
  "addEventListener" [@@bs.send.pipe : Dom.Element.t];

external addKeyUpListener : _ [@bs.as "keyup"] => (Dom.KeyboardEvent.t => unit) => unit =
  "addEventListener" [@@bs.send.pipe : Dom.Document.t];

external setValue : Dom.HtmlElement.t_htmlElement => string => unit = "value" [@@bs.set];

external getValue : Dom.HtmlElement.t_htmlElement => string = "value" [@@bs.get];

/* helper functions */
let getElementOrFail (name: string) =>
  switch (Dom.Document.getElementById name Dom.document) {
  | None => failwith ("Couldn't find element [" ^ name ^ "]")
  | Some el => el
  };

let promiseFailed name _ => Js_promise.resolve @@ Js.log (name ^ " FAILED");

/* socketio setup */
module Common = {
  type t =
    | CancelEvent
    | AnswerEvent
    | CandidateEvent
    | OfferEvent;
  let stringify t =>
    switch t {
    | CancelEvent => "cancel-connection"
    | AnswerEvent => "answer"
    | CandidateEvent => "ice-candidate"
    | OfferEvent => "offer"
    };
  let all = [CancelEvent, AnswerEvent, CandidateEvent, OfferEvent];
};

module SocketIO = Client.Client Common;

/* dom elements */
let chatinput = {
  let chatinput = getElementOrFail "chatinput";
  switch (Dom.Element.asHtmlElement chatinput) {
  | None => failwith "error"
  | Some e => e
  }
};

let sendbutton = getElementOrFail "sendbutton";

let chatarea = getElementOrFail "chatarea";

let dataChannel = ref None;

let configureDataChannel dataChannel => {
  Js.log "configureDataChannel...";
  Js.log dataChannel;
  RTCDataChannel.setOnError dataChannel (fun _ => Js.log "there was an error somewhere");
  RTCDataChannel.setOnMessage
    dataChannel
    (
      fun msg => {
        let msgContent = RTCMessageEvent.getData msg;
        Js.log ("there was a message somehwere " ^ msgContent);
        let content = Dom.Element.innerHTML chatarea;
        Dom.Element.setInnerHTML
          chatarea
          (content ^ "<div><span style='color:red'>Other</span>: " ^ msgContent ^ "</div>")
      }
    );
  RTCDataChannel.setOnOpen dataChannel (fun () => Js.log "onopen");
  RTCDataChannel.setOnClose dataChannel (fun () => Js.log "onclose")
};

let setupConnection (connection: RTCPeerConnection.t) socket => {
  RTCPeerConnection.setOnIceCandidate
    connection
    (
      fun e => {
        Js.log "setOnIceCandidate";
        Js.log e;
        let candidate = RTCPeerConnectionIceEvent.getCandidate e;
        if (not (Js.Null.test (Js.Null.return candidate))) {
          SocketIO.emit socket CandidateEvent candidate
        }
      }
    );
  RTCPeerConnection.setOnIceConnectionStateChange
    connection (fun _ => Js.log "setOnIceConnectionStateChange");
  RTCPeerConnection.setOnDataChannel
    connection
    (
      fun e => {
        Js.log "setOnDataChannel";
        let dc = RTCDataChannelEvent.getChannel e;
        Js.log dc;
        configureDataChannel dc;
        RTCDataChannel.send dc "Connected!";
        dataChannel := Some dc
      }
    )
};

let createDataChannel (connection: RTCPeerConnection.t) channelName => {
  let options = RTCDataChannel.makeOptions ordered::true maxRetransmitTime::3000.0;
  let dataChannel = RTCPeerConnection.createDataChannel connection ::channelName ::options;
  configureDataChannel dataChannel;
  dataChannel
};

let makeConnection socket => {
  let connection = RTCPeerConnection.create ();
  setupConnection connection socket;
  SocketIO.on
    socket
    CancelEvent
    (
      fun () => {
        Js.log "closing...";
        RTCPeerConnection.close connection
      }
    );
  SocketIO.on
    socket
    AnswerEvent
    (
      fun desc => {
        Js.log "got answer";
        ignore (
          RTCPeerConnection.setRemoteDescription connection desc |>
          Js_promise.then_ (fun _ => Js_promise.resolve @@ Js.log "setRemoteDescription success") |>
          Js_promise.catch (promiseFailed "setRemoteDescription")
        )
      }
    );
  SocketIO.on
    socket
    CandidateEvent
    (
      fun candidate => {
        Js.log "got ice-candidate";
        ignore (
          RTCPeerConnection.addIceCandidate connection candidate |>
          Js_promise.then_ (fun _ => Js_promise.resolve @@ Js.log "addIceCandidate success") |>
          Js_promise.catch (promiseFailed "addIceCandidate")
        )
      }
    );
  dataChannel := Some (createDataChannel connection "somenamehere");
  connection
};

let socket = SocketIO.create ();

let connection = makeConnection socket;

let connectionOptions = RTCOffer.makeOptions offerToReceiveAudio::false offerToReceiveVideo::false;

RTCPeerConnection.createOffer connection options::connectionOptions |>
Js_promise.then_ (
  fun desc => {
    Js.log "createOffer success!";
    RTCPeerConnection.setLocalDescription connection desc |>
    Js_promise.then_ (
      fun () => {
        Js.log "setLocalDescription wow";
        Js_promise.resolve (SocketIO.emit socket OfferEvent desc)
      }
    ) |>
    Js_promise.catch (promiseFailed "setLocalDescription")
  }
) |>
Js_promise.catch (promiseFailed "createOffer");

SocketIO.on
  socket
  OfferEvent
  (
    fun desc => {
      Js.log "got offer";
      let connection = makeConnection socket;
      ignore (
        RTCPeerConnection.setRemoteDescription connection desc |>
        Js_promise.then_ (fun () => Js_promise.resolve @@ Js.log "setRemoteDescription success") |>
        Js_promise.catch (promiseFailed "addIceCandidate")
      );
      ignore (
        RTCPeerConnection.createAnswer connection |>
        Js_promise.then_ (
          fun desc => {
            Js.log "createAnswer went well";
            SocketIO.emit socket AnswerEvent desc;
            RTCPeerConnection.setLocalDescription connection desc
          }
        ) |>
        Js_promise.then_ (fun () => Js_promise.resolve (Js.log "setLocalDescription went well")) |>
        Js_promise.catch (promiseFailed "setRemoteDescription or createAnswer")
      )
    }
  );

let sendHandler _ => {
  let content = Dom.Element.innerHTML chatarea;
  let msgContent = getValue chatinput;
  Dom.Element.setInnerHTML
    chatarea (content ^ "<div><span style='color:red'>You</span>: " ^ msgContent ^ "</div>");
  switch !dataChannel {
  | None => Js.log "not connected yet"
  | Some chan => RTCDataChannel.send chan msgContent
  };
  setValue chatinput ""
};

addClickListener sendHandler sendbutton;

addKeyUpListener
  (
    fun e =>
      if (Dom.KeyboardEvent.key e == "Enter") {
        sendHandler ()
      }
  )
  Dom.document;
