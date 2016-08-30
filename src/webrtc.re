open Js.Unsafe;

let module RTCIceCandidate = {
  class t innerSelf => {
    val _innerSelf: any = innerSelf;
    method raw = _innerSelf;
  };
};

let module RTCPeerConnectionIceEvent = {
  class t innerSelf => {
    val _innerSelf = innerSelf;
    method candidate: RTCIceCandidate.t = (new RTCIceCandidate.t) (get _innerSelf "candidate");
  };
};

let module RTCIceConnectionStateChangeEvent = {
  class t innerSelf => {
    val _innerSelf = innerSelf;
    method iceConnectionState = Js.to_string (get _innerSelf "iceConnectionState");
  };
};

let module MessageEvent = {
  class t innerSelf => {
    val _innerSelf: any = innerSelf;
    method data: string = Js.to_string (get _innerSelf "data");
  };
};

let module RTCDataChannel = {
  class t innerSelf => {
    val _innerSelf = innerSelf;
    method setOnError (cb: string => unit) =>
      set _innerSelf "onerror" (Js.wrap_callback (fun e => cb (Js.string_of_error e)));
    method setOnOpen (cb: unit => unit) =>
      set _innerSelf "onopen" (Js.wrap_callback (fun () => cb ()));
    method setOnClose (cb: unit => unit) =>
      set _innerSelf "onclose" (Js.wrap_callback (fun () => cb ()));
    method setOnMessage (cb: MessageEvent.t => unit) =>
      set _innerSelf "onmessage" (Js.wrap_callback (fun e => cb ((new MessageEvent.t) e)));
    method send s :unit => meth_call _innerSelf "send" [|inject (Js.string s)|];
  };
  type optionsT = {ordered: bool, maxRetransmitTime: float};
  let createDataChannel connection channelName channelOptions => {
    let options = new_obj (variable "Object");
    set options "ordered" channelOptions.ordered;
    set options "maxRetransmitTime" channelOptions.maxRetransmitTime;
    let channel =
      meth_call
        connection#raw "createDataChannel" [|inject (Js.string channelName), inject options|];
    (new t) channel
  };
};

let module RTCSessionDescription = {
  class t innerSelf => {
    val _innerSelf: any = innerSelf;
    method raw = _innerSelf;
  };
};

let module RTCPeerConnection = {
  type offerT = {offerToReceiveAudio: float, offerToReceiveVideo: float};
  class t = {
    as self;
    val _innerSelf: any = new_obj (variable "RTCPeerConnection") [||];
    method raw = _innerSelf;
    method setOnIceCandidate (cb: RTCPeerConnectionIceEvent.t => unit) =>
      set
        _innerSelf
        "onicecandidate"
        (Js.wrap_callback (fun e => cb ((new RTCPeerConnectionIceEvent.t) e)));
    method setOnIceConnectionStateChange (cb: RTCIceConnectionStateChangeEvent.t => unit) =>
      set
        _innerSelf
        "oniceconnectionstatechange"
        (Js.wrap_callback (fun e => (new RTCIceConnectionStateChangeEvent.t) e));
    method setOnDataChannel (cb: RTCDataChannel.t => unit) =>
      set
        _innerSelf
        "ondatachannel"
        (Js.wrap_callback (fun e => cb ((new RTCDataChannel.t) (get e "channel"))));
    method addIceCandidate (iceCandidate: RTCIceCandidate.t) (cb: option string => unit) :unit => {
      let promise = meth_call _innerSelf "addIceCandidate" [|inject iceCandidate#raw|];
      meth_call
        promise
        "then"
        [|
          inject (Js.wrap_callback (fun () => cb None)),
          inject (Js.wrap_callback (fun err => cb (Some (Js.string_of_error err))))
        |]
    };
    method createDataChannel channelName channelOptions =>
      RTCDataChannel.createDataChannel self channelName channelOptions;
    method createOffer
           offerOptions
           (success: RTCSessionDescription.t => unit)
           (failure: string => unit)
           :unit => {
      let options = obj [|
        ("offerToReceiveAudio", inject offerOptions.offerToReceiveAudio),
        ("offerToReceiveVideo", inject offerOptions.offerToReceiveVideo)
      |];
      let promise = meth_call _innerSelf "createOffer" [|inject options|];
      meth_call
        promise
        "then"
        [|
          inject (Js.wrap_callback (fun e => success ((new RTCSessionDescription.t) e))),
          inject (Js.wrap_callback (fun e => failure (Js.string_of_error e)))
        |]
    };
    method createAnswer (success: RTCSessionDescription.t => unit) (failure: string => unit) :unit => {
      let promise = meth_call _innerSelf "createAnswer" [||];
      meth_call
        promise
        "then"
        [|
          inject (Js.wrap_callback (fun e => success ((new RTCSessionDescription.t) e))),
          inject (Js.wrap_callback (fun err => failure (Js.string_of_error err)))
        |]
    };
    method setLocalDescription (desc: RTCSessionDescription.t) (cb: option string => unit) :unit => {
      let promise = meth_call _innerSelf "setLocalDescription" [|inject desc#raw|];
      meth_call
        promise
        "then"
        [|
          inject (Js.wrap_callback (fun e => cb None)),
          inject (Js.wrap_callback (fun e => cb (Some (Js.string_of_error e))))
        |]
    };
    method setRemoteDescription (desc: RTCSessionDescription.t) (cb: option string => unit) :unit => {
      let promise = meth_call _innerSelf "setRemoteDescription" [|inject desc#raw|];
      meth_call
        promise
        "then"
        [|
          inject (Js.wrap_callback (fun e => cb None)),
          inject (Js.wrap_callback (fun e => cb (Some (Js.string_of_error e))))
        |]
    };
    method close () :unit => meth_call _innerSelf "close" [||];
  };
};

/* type t = SendingOffer | OfferSent; */
/* let module SymmetricConnection = {
     type t;

     let connect dataChannelName::dataChannelName => {
       meth_call (variable "window") "doThePeerConnectionThing" [|inject (Js.string dataChannelName)|];
     };
   }; */
