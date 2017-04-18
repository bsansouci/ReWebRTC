module RTCIceCandidate = {
  type t;
  external create : unit => t = "RTCIceCandidate" [@@bs.new];
};

module RTCPeerConnectionIceEvent = {
  type t;
  external getCandidate : t => RTCIceCandidate.t = "candidate" [@@bs.get];
};

module RTCIceConnectionStateChangeEvent = {
  type t;
  external getIceConnectionState : t => string = "iceConnectionState" [@@bs.get];
};

module RTCErrorEvent = {
  type t;
  external getMessage : t => string = "message" [@@bs.get];
};

module RTCMessageEvent = {
  type t;
  external getData : t => string = "data" [@@bs.get];
  external getMessage : t => string = "message" [@@bs.get];
};

module RTCDataChannel = {
  type t;
  type optionsT;
  external setOnError : t => (RTCErrorEvent.t => unit) => unit = "onerror" [@@bs.set];
  external setOnOpen : t => (unit => unit) => unit = "onopen" [@@bs.set];
  external setOnClose : t => (unit => unit) => unit = "onclose" [@@bs.set];
  external setOnMessage : t => (RTCMessageEvent.t => unit) => unit = "onmessage" [@@bs.set];
  external send : t => string => unit = "send" [@@bs.send];
  external makeOptions : ordered::Js.boolean => maxRetransmitTime::float => optionsT =
    "" [@@bs.obj];
  let makeOptions ::ordered ::maxRetransmitTime =>
    makeOptions ordered::(Js.Boolean.to_js_boolean ordered) ::maxRetransmitTime;
};

module RTCDataChannelEvent = {
  type t;
  external getChannel : t => RTCDataChannel.t = "channel" [@@bs.get];
};

module RTCOffer = {
  type t;
  type optionsT;
  external makeOptions :
    offerToReceiveAudio::Js.boolean => offerToReceiveVideo::Js.boolean => optionsT =
    "" [@@bs.obj];
  let makeOptions ::offerToReceiveAudio ::offerToReceiveVideo =>
    makeOptions
      offerToReceiveAudio::(Js.Boolean.to_js_boolean offerToReceiveAudio)
      offerToReceiveVideo::(Js.Boolean.to_js_boolean offerToReceiveVideo);
};

module RTCSessionDescription = {
  type t;
};

module RTCPeerConnection = {
  type t;
  external create : unit => t = "RTCPeerConnection" [@@bs.new];
  external setOnIceCandidate : t => (RTCPeerConnectionIceEvent.t => unit) => unit =
    "onicecandidate" [@@bs.set];
  external setOnIceConnectionStateChange :
    t => (RTCIceConnectionStateChangeEvent.t => unit) => unit =
    "oniceconnectionstatechange" [@@bs.set];
  external setOnDataChannel : t => (RTCDataChannelEvent.t => unit) => unit =
    "ondatachannel" [@@bs.set];
  external addIceCandidate : t => RTCIceCandidate.t => Js_promise.t unit /*string*/ =
    "addIceCandidate" [@@bs.send];
  external createDataChannel :
    t => channelName::string => options::RTCDataChannel.optionsT => RTCDataChannel.t =
    "createDataChannel" [@@bs.send];
  external createOffer :
    t => options::RTCOffer.optionsT => Js_promise.t RTCSessionDescription.t /*string*/ =
    "createOffer" [@@bs.send];
  external createDefaultOffer : t => Js_promise.t RTCOffer.t /*string*/ =
    "createOffer" [@@bs.send];
  external createAnswer : t => Js_promise.t RTCSessionDescription.t /*string*/ =
    "createAnswer" [@@bs.send];
  external setLocalDescription : t => RTCSessionDescription.t => Js_promise.t unit /*string*/ =
    "setLocalDescription" [@@bs.send];
  external setRemoteDescription : t => RTCSessionDescription.t => Js_promise.t unit /*string*/ =
    "setRemoteDescription" [@@bs.send];
  external close : t => unit = "close" [@@bs.send];
};
